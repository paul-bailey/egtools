/*
 * TODO: This assumes file will be read back on machine of same
 * endianness.
 */
#include <eghash.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>

#ifndef EXPORT
# ifdef __GNUC__
#  define EXPORT __attribute__((visibility("default")))
# else
#  define EXPORT
# endif
#endif

#define DEFAULT_CALC eg_psh_hash

#define HTABLE_MAGIC (0x85763294) /* sure, why not */

#define MAX_BUCKETS(t_) (((t_)->t_size) * 3 / 4)
#define MAX_COLLISIONS  8

/**
 * struct HashTable - private definition of the HashTable type declared
 *                         in eghash.h
 * @t_size: Array size (in indexes) of @t_entries
 * @t_flags: Currently unused metadata
 * @t_calc: Pointer to algorithm to calculate a key's hash.  This will
 *      need to be restored by user when reading in from file
 * @t_nentries: Current number of entries entered into the table
 * @t_collisions: Size of the largest collision list.  '1' implies no
 *      collisions; '0' implies no entries (ie @t_nentries == 0)
 * @t_entries: Pointer to the bucket array.  Rather than being an array
 *      of buckets, it is an array of pointers to buckets, which makes
 *      resizing quicker (but requires a malloc() for every insertion).
 */
struct HashTable {
        size_t t_size;
        unsigned int t_flags;
        hashfunc_t t_calc;
        unsigned int t_nentries;
        unsigned int t_collisions;
        struct bucket_t **t_entries;
};

/**
 * struct h_header_t - File header for hash table
 * @magic: Magic number to sanity-check that the following hash table
 *      meets our format
 * @size: Size of the hash table to follow the header, in bytes
 */
struct h_header_t {
        uint32_t magic;
        uint32_t size;
        uint32_t RESERVED[2];
};

/**
 * struct bucket_t - Descriptor of a hash table entry
 * @e_key: Look up key
 * @e_data: Data to set/query
 * @e_child: Pointer to the next bucket in the collision list
 * @e_fchild: Integer alias of @e_child, for use when unswizzling
 * @e_parent: Pointer to the previous bucket in the collision list, or NULL
 *         if the entry is the first item on the list
 * @e_flags: Currently-unused extra metadata about the entry
 */
struct bucket_t {
        struct eghash_t e_key;
        struct eghash_t e_data;
        unsigned long e_hash;
        union {
                struct bucket_t *e_child;
                unsigned long e_fchild;
        };
        struct bucket_t *e_parent;
        unsigned int e_flags;
};

/*
 * Return true if a hash table has too many entries or too many
 * collisions
 */
static bool
limits_exceeded(HashTable *table, int collisions)
{
        return table->t_nentries >= MAX_BUCKETS(table)
                || collisions >= MAX_COLLISIONS;
}

static size_t
next_table_size(HashTable *table)
{
        /* TODO: Return a more intelligent next size */
        return table->t_size * 2 + 1;
}

/* Called after table was either allocated or declared on stack. */
static int
eghash_create_(HashTable *table, size_t size, hashfunc_t calc)
{
        struct bucket_t **bucket;
        size_t i;

        /* IE sizeof ptr array */
        bucket = malloc(sizeof(*bucket) * size);
        if (!bucket)
                return -1;
        table->t_nentries = 0;
        table->t_entries = bucket;
        table->t_size = size;
        table->t_calc = (calc == NULL) ? DEFAULT_CALC : calc;
        table->t_collisions = 0;
        for (i = 0; i < size; i++)
                table->t_entries[i] = NULL;
        return 0;
}

static bool
hash_matches(struct bucket_t *entry, const struct eghash_t *key, unsigned long hash)
{
        if (hash != entry->e_hash)
                return false;
        if (entry->e_key.size != key->size)
                return false;
        if (memcmp(key->data, entry->e_key.data, key->size) != 0)
                return false;
        return true;
}

static int
insert_at_index_r(struct bucket_t *entry, struct bucket_t *parent, int count)
{
        ++count;
        if (hash_matches(parent, &entry->e_key, entry->e_hash)) {
                return -1;
        } else if (parent->e_child == NULL) {
                parent->e_child = entry;
                entry->e_child = NULL;
                entry->e_parent = parent;
                return count;
        }

        return insert_at_index_r(entry, parent->e_child, count);
}

/*
 * Returns of number of entries at inserted index, or -1 if there's
 * already a match for @entry's key
 */
static int
insert_at_index(HashTable *table, struct bucket_t *entry)
{
        struct bucket_t **ppentry;
        ppentry = &table->t_entries[entry->e_hash % table->t_size];
        if (*ppentry == NULL) {
                entry->e_child = NULL;
                entry->e_parent = NULL;
                *ppentry = entry;
                return 1;
        } else {
                return insert_at_index_r(entry, *ppentry, 1);
        }
}

static int
eghash_resize(HashTable *old)
{
        HashTable new;
        unsigned int max;
        size_t i;

        if (eghash_create_(&new, next_table_size(old), old->t_calc) < 0)
                return -1;

        /*
         * Algorithm to change the pointers rather than allocating and
         * copying entries to new addresses.
         */
        max = 0;
        for (i = 0; i < old->t_size; i++) {
                struct bucket_t *p, *q;
                p = old->t_entries[i];
                while (p != NULL) {
                        unsigned int cols;
                        /*
                         * Do this first, because the insert call will
                         * clobber p's pointers.
                         */
                        q = p->e_child;

                        cols = insert_at_index(&new, p);
                        if (cols > max)
                                max = cols;
                        ++new.t_nentries;
                        p = q;
                }
        }
        new.t_collisions = max;

        /* Old bucket-pointer array no longer being used. */
        free(old->t_entries);
        /* Keep old table, but with new info. */
        memcpy(old, &new, sizeof(*old));
        return 0;
}

#define HDRECURSIVE 001
#define HDKEEPDATA  002

static void
delete_entry(HashTable *table, struct bucket_t *entry, unsigned int flags)
{
        struct bucket_t *child = entry->e_child;

        /* unlink */
        if (entry->e_parent != NULL) {
                entry->e_parent->e_child = entry->e_child;
        } else {
                /*
                 * We're at the top of the bucket list (sic), so make
                 * sure we aren't referenced from there.
                 */
                /* TODO: assert(table->t_entries[idx] == e) first */
                table->t_entries[entry->e_hash % table->t_size] = child;
        }

        if (child != NULL)
                child->e_parent = entry->e_parent;

        /* Maybe free data */
        free(entry->e_key.data);
        free(entry->e_data.data);

        free(entry);

        if (child != NULL && !!(flags & HDRECURSIVE))
                delete_entry(table, child, flags);
}

/* some syntactic sugar */
static inline unsigned long
calc_hash(HashTable *table, const struct eghash_t *key)
{
        return table->t_calc(key->data, key->size);
}

static struct bucket_t *
hashtbl_seek(HashTable *table, const struct eghash_t *key)
{
        unsigned long hash;
        struct bucket_t *entry;

        if (key->size == 0 || key->data == NULL)
                return NULL;

        hash = calc_hash(table, key);
        entry = table->t_entries[hash % table->t_size];
        while (entry != NULL && !hash_matches(entry, key, hash)) {
                entry = entry->e_child;
                if (entry == NULL)
                        return NULL;
        }
        return entry;
}


/* **********************************************************************
 *              Serialization (syncing and opening) helpers
 ***********************************************************************/

#define HALIGN ((unsigned long)1 << 4)
#define HALIGN_MASK (HALIGN-1)
#define HROUNDUP(x_) (((x_) + HALIGN_MASK) & ~HALIGN_MASK)

static inline size_t BUCKET_SIZE_UNROUNDED(HashTable *table)
{
        return sizeof(table->t_entries) * table->t_size;
}

/* Size of table bucket pointer array, in bytes rather than indices,
 * possibly with trailing extra space at end to round up
 */
static inline size_t BUCKET_SIZE(HashTable *table)
{
        return HROUNDUP(BUCKET_SIZE_UNROUNDED(table));
}


/* **********************************************************************
 *              Sync'ing out
 ***********************************************************************/

static size_t
calculate_entry_size(struct bucket_t *entry)
{
        size_t res;
        res = HROUNDUP(sizeof(*entry))
                + HROUNDUP(entry->e_key.size)
                + HROUNDUP(entry->e_data.size);
        if (entry->e_child != NULL)
                res += calculate_entry_size(entry->e_child);
        return res;
}

static size_t
estimate_fsize(HashTable *table)
{
        size_t res;
        size_t i;
        res = sizeof(*table) + BUCKET_SIZE(table);
        for (i = 0; i < table->t_size; i++) {
                if (table->t_entries[i] == NULL)
                        continue;
                res += calculate_entry_size(table->t_entries[i]);
        }
        return res;
}

/* Returns updated @off */
static size_t
unswizzle_entry(char *buf, uintptr_t off, struct bucket_t *src)
{
        struct bucket_t *dst;

        if (src == NULL)
                return off;

        dst = (struct bucket_t *)&buf[off];

        /* Copy the bucket */
        memcpy(dst, src, sizeof(*dst));
        off += HROUNDUP(sizeof(*dst));

        /* We're no longer uniquely allocated */

        /* Copy and unswizzle the bucket's data... */
        memcpy(&buf[off], src->e_data.data, src->e_data.size);
        dst->e_data._fdata = off;
        off += HROUNDUP(src->e_data.size);

        /* ...and key */
        memcpy(&buf[off], src->e_key.data, src->e_key.size);
        dst->e_key._fdata = off;
        off += HROUNDUP(src->e_key.size);

        dst->e_fchild = src->e_child != NULL ? off : 0L;

        /* Repeat for collisions */
        return unswizzle_entry(buf, off, src->e_child);
}

/*
 * copy a hash table and its data into an array of contiguous
 * memory and convert all its references from pointers to original
 * data into array indices of newly copied data.
 */
static void
table_copy(HashTable *table, char *buf, size_t bufsize)
{
        uintptr_t off;
        size_t i;
        struct bucket_t **buf_entries;

        /* Copy the basic table in */
        memcpy(buf, table, sizeof(*table));

        /*
         * Unrounded, so we don't copy from possibly unallocated trailing
         * memory after table->t_entries.
         */
        memcpy(&buf[sizeof(*table)], table->t_entries,
               BUCKET_SIZE_UNROUNDED(table));

        /* Point into array rather than old place in memory */
        ((HashTable *)buf)->t_entries = (struct bucket_t **)sizeof(*table);

        /*
         * Make sure collisions are copied in, and then that their
         * pointers are indexes into this array rather than pointers
         * to their old places in memory.
         */
        off = (uintptr_t)(sizeof(*table) + BUCKET_SIZE(table));
        /* start of entries */
        buf_entries = (struct bucket_t **)(&buf[sizeof(*table)]);
        for (i = 0; i < table->t_size; i++) {
                struct bucket_t *src = table->t_entries[i];
                if (src == NULL)
                        buf_entries[i] = (struct bucket_t *)0L;
                else
                        buf_entries[i] = (struct bucket_t *)off;
                off = unswizzle_entry(buf, off, src);
        }

        /* TODO: assert(off == bufsize); */
}


/* **********************************************************************
 *              Reading in
 ***********************************************************************/

/* XXX: Should be checking after FILETYPE_TO_PTR()? */
#define INVALID_KEY_SIZE(k_, lim_) \
        ((k_)->_fdata >= (lim_) \
         || (k_)->size > (lim_) \
         || ((k_)->_fdata + (k_)->size) > (lim_))

#define INVALID_ENTRY_PTRS(e_, lim_) \
        (INVALID_KEY_SIZE(&(e_)->e_key, lim_) \
         || INVALID_KEY_SIZE(&(e_)->e_data, lim_))

/*
 * Technically a copy, not a swizzle, since swizzling in-place prevents
 * the ability to grow the hash table later.
 */
static struct bucket_t *
entry_swizzle(char *buf, uintptr_t off,
              size_t bufsize, struct bucket_t *parent)
{
        void *data, *key;
        struct bucket_t *src;
        struct bucket_t *dst;

        dst = malloc(sizeof(*dst));
        if (!dst)
                return NULL;
        src = (struct bucket_t *)&buf[off];
        if (INVALID_ENTRY_PTRS(src, bufsize))
                goto e_freedst;
        memcpy(dst, src, sizeof(*dst));
        dst->e_parent = parent;
        data = malloc(dst->e_data.size);
        if (!data)
                goto e_freedst;
        key = malloc(dst->e_key.size);
        if (!key)
                goto e_freedata;
        memcpy(data, &buf[dst->e_data._fdata], dst->e_data.size);
        memcpy(key, &buf[dst->e_key._fdata], dst->e_key.size);
        dst->e_data.data = data;
        dst->e_key.data = key;
        if (dst->e_fchild == 0) {
                dst->e_child = NULL;
        } else {
                dst->e_child = entry_swizzle(buf, dst->e_fchild, bufsize, dst);
                if (dst->e_child == NULL)
                        goto e_freekey;
        }
        return dst;

e_freekey:
        free(key);
e_freedata:
        free(data);
e_freedst:
        free(dst);
        return NULL;
}

/* Helper to eghash_deserialize.  Handle one index of .t_entries */
static int
deserialize_idx(char *buf, unsigned long bufsize,
                HashTable *table, size_t idx)
{
        uintptr_t off = (uintptr_t)table->t_entries[idx];
        if (off > bufsize) {
                /* bad pointer */
                table->t_entries[idx] = NULL;
                return -1;
        } else if (off == 0) {
                table->t_entries[idx] = NULL;
        } else {
                struct bucket_t *entry;

                entry = entry_swizzle(buf, off, bufsize, NULL);
                table->t_entries[idx] = entry; /* keep in order */
                if (entry == NULL)
                        return -1;

                /*
                 * Just once, make sure the user has the correct method
                 * to calculate hash, or this table is useless.
                 */
                if (table->t_nentries == 0) {
                        if (calc_hash(table, &entry->e_key)
                            != entry->e_hash) {
                                return -1;
                        }
                }

                /* Update this; XXX: time-consuming */
                for (; entry != NULL; entry = entry->e_child)
                        ++table->t_nentries;
        }
        return 0;
}

/*
 * Create a swizzled hash table, copying data from buf.
 * buf can be freed after this operation.
 */
static HashTable *
eghash_deserialize(char *buf, unsigned long bufsize, hashfunc_t calc)
{
        int i;
        size_t bucket_size;
        unsigned long nentrysav;
        HashTable *table = malloc(sizeof(*table));
        if (!table)
                return NULL;
        memcpy(table, buf, sizeof(*table));

        table->t_calc = calc != NULL ? calc : DEFAULT_CALC;

        /* save for our final sanity check */
        nentrysav = table->t_nentries;
        table->t_nentries = 0;

        bucket_size = BUCKET_SIZE(table);

        /* more sanity checking */
        if ((bucket_size + sizeof(*table)) > bufsize)
                goto esanity;
        if (table->t_entries != (struct bucket_t **)sizeof(*table))
                goto esanity;

        table->t_entries = malloc(bucket_size);
        if (!table->t_entries)
                goto esanity;
        memcpy(table->t_entries, &buf[sizeof(*table)], bucket_size);
        for (i = 0; i < table->t_size; i++) {
                if (deserialize_idx(buf, bufsize, table, i) < 0)
                        goto e_unwind;
        }

        /* one last sanity check */
        if (table->t_nentries != nentrysav) {
                --i;
                goto e_unwind;
        }

        return table;

        /* start of error handling */
e_unwind:
        while ((int)i > 0) {
                if (table->t_entries[i] != NULL) {
                        delete_entry(table, table->t_entries[i], HDRECURSIVE);
                }
                --i;
        }
        free(table->t_entries);

esanity:
        free(table);
        return NULL;
}


/* **********************************************************************
 *              API Functions
 ***********************************************************************/

/**
 * eghash_sync - Write a hash table back out to a file
 * @table: Hash table to write
 * @fd: Descriptor of file to write DB to.  eghash_sync() assumes that
 * the descriptor already points to the correct place in the file.
 *
 * Return: zero if successful, -1 if not.
 *
 * Warning: The hash calculation method will be lost.  The user is
 * expected to know which calculation method was used when reopening
 * the file.
 *
 * Also, if any of the key/value pairs have pointers in their private
 * data, that data will become meaningless.
 */
int EXPORT
eghash_sync(HashTable *table, int fd)
{
        size_t size;
        unsigned long *buf;
        struct h_header_t hdr;
        int ret = 0;

        size = estimate_fsize(table);

        memset(&hdr, 0, sizeof(hdr));
        hdr.magic = HTABLE_MAGIC;
        hdr.size = size;

        /*
         * FIXME: Wow, this could be a lot of mmeory!
         * Surely there's a better way to serialize all this data
         * without having to copy every single byte into a new
         * array!
         */
        buf = malloc(size);
        if (buf == NULL)
                return -1;

        /* XXX: should never fail? */
        table_copy(table, (char *)buf, size);

        if (write(fd, &hdr, sizeof(hdr)) != sizeof(hdr))
                ret = -1;

        if (ret == 0) {
                if (write(fd, buf, size) != size)
                        ret = -1;
        }

        free(buf);
        return ret;
}

/**
 * eghash_open - Open a hash table from disk and read it into memory
 * @fd: Open file to read.  eghash_open() assumes that the file
 * descriptor's position indicator is already at the expected position.
 * @calc: Method used to calculate the hash for a given key
 *
 * Return: A handle to the hash table, or NULL if there was an error.
 * An error will be returned if the table at @fd cannot be interpreted,
 * memory cannot be allocated, or @calc is not the same method used to
 * create the hash table in the first place.
 */
HashTable EXPORT *
eghash_open(int fd, unsigned long (*calc)(const void *, size_t))
{
        unsigned long *buf;
        struct h_header_t hdr;
        HashTable *ret;

        if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr))
                return NULL;

        if (hdr.magic != HTABLE_MAGIC)
                return NULL;

        buf = malloc(hdr.size);
        if (buf == NULL)
                return NULL;

        if (read(fd, buf, hdr.size) != hdr.size) {
                free(buf);
                return NULL;
        }

        ret = eghash_deserialize((char *)buf, hdr.size, calc);

        free(buf);
        return ret;
}

/**
 * eghash_diag - Find out how well these functions work.
 * @table: Hash table to check out
 * @diag: Pointer to struct to fill with information about @table,
 *      such as how much of it's being filled, how many collisions there
 *      are, etc.
 *
 * This could be used at development time for trial-and-error tests of
 * different hash algorithms.
 */
void
eghash_diag(HashTable *table, struct eghash_diag_t *diag)
{
        size_t i;
        unsigned long misses, cols;
        cols = 0;
        misses = 0;
        for (i = 0; i < table->t_size; i++) {
                struct bucket_t *entry = table->t_entries[i];
                if (entry == NULL) {
                        ++misses;
                } else {
                        while (entry->e_child != NULL) {
                                ++cols;
                                entry = entry->e_child;
                        }
                }
        }
        diag->hits = table->t_size - misses;
        diag->misses = misses;
        diag->size = estimate_fsize(table);
        diag->collisions = cols;
        diag->maxcollisions = table->t_collisions;
}

/**
 * eghash_put - Insert data into a hash table
 * @table: Table to insert data into
 * @key: Lookup key that will be used
 * @data: Data to inserted.
 *
 * @key and @data WILL BE COPIED, including the data they point to!.  Do
 * not use this API if you intend to maintain pointers into the data
 * being inserted.  It's not that kind of a database.
 *
 * Neither @key nor @data may be NULL, nor may they have NULL .data
 * fields, nor may their .size fields be zero.
 *
 * Return: zero if @key and @data were inserted into @table, -1 if there
 * was an error.  errno will be set if there was an error due to no
 * memory (in case a collision requires new memory to be allocated), or
 * if @key or @data are invalid arguments.
 */
int EXPORT
eghash_put(HashTable *table, const struct eghash_t *key, const struct eghash_t *data)
{
        struct bucket_t *entry;
        unsigned long hash;
        void *ddata, *kdata;
        int colcount;

        if (key == NULL || data == NULL
            || key->data == NULL || key->size == 0
            || data->data == NULL || data->size == 0) {
                errno = EINVAL;
                return -1;
        }
        /*
         * TODO: Flag parameter to determine if @data's pointers
         * are pass-by-reference or not.
         */
        kdata = malloc(key->size);
        if (!kdata)
                goto ekdata;
        ddata = malloc(data->size);
        if (!ddata)
                goto eddata;

        memcpy(kdata, key->data, key->size);
        memcpy(ddata, data->data, data->size);

        hash = calc_hash(table, key);

        entry = malloc(sizeof(*entry));
        if (entry == NULL)
                goto eentry;

        /* Initialize entry */
        memset(entry, 0, sizeof(*entry));

        /*
         * TODO: By adding to front of list, this doesn't check
         * if there are too many entries per bucket.
         */
        entry->e_key.size = key->size;
        entry->e_key.data = kdata;
        entry->e_data.size = data->size;
        entry->e_data.data = ddata;
        entry->e_hash = hash;

        colcount = insert_at_index(table, entry);
        if (colcount < 0)
                goto ematch;

        if (colcount > table->t_collisions)
                table->t_collisions = colcount;

        table->t_nentries++;
        if (limits_exceeded(table, colcount))
                return eghash_resize(table);
        return 0;

ematch:
        free(entry);
eentry:
        free(ddata);
eddata:
        free(kdata);
ekdata:
        return -1;
}

/**
 * eghash_get - Get data from a hash table
 * @table: Table to get data from
 * @key: Lookup key to find data
 * @data: Pointer to struct to fill data with
 *
 * Return: zero if there was a match for @key and @data was filled;
 * -1 if @key did not have a match.
 *
 * WARNING: @data->data will point to a *copy* of the data originally
 * inserted.  This should be treated as a *constant*, and copied again
 * if you intend to manipulate it.  The only manipulation of data in
 * the hash table should be through hash table functions.
 */
int EXPORT
eghash_get(HashTable *table, const struct eghash_t *key, struct eghash_t *data)
{
        struct bucket_t *entry;
        entry = hashtbl_seek(table, key);
        if (entry == NULL)
                return -1;
        memcpy(data, &entry->e_data.data, sizeof(*data));
        return 0;
}

/**
 * eghash_remove - Delete a hash entry
 * @table: Table holding the entry
 * @key: Lookup key to find the entry
 *
 * Return: zero if the key was deleted, -1 if the key was not found.
 */
int EXPORT
eghash_remove(HashTable *table, const struct eghash_t *key)
{
        struct bucket_t *entry;
        entry = hashtbl_seek(table, key);
        if (!entry)
                return -1;

        delete_entry(table, entry, 0);
        table->t_nentries--;
        return 0;
}

/**
 * eghash_change - Remove old data from hash table and replace it with
 *    new data
 * @table: Hash table to modify
 * @key: Key to the data, old and new
 * @new_data: New data to insert
 *
 * Return: zero if successfully changed, -1 if @key not found, or @new_data
 * could not be entered.
 *
 * For fastest results, the .size field of @new_data should be equal to
 * or less than the size of the data already entered.  Otherwise a malloc()
 * call will occur.
 */
int
eghash_change(HashTable *table, const struct eghash_t *key,
              const struct eghash_t *new_data)
{
        struct bucket_t *entry;
        if ((entry = hashtbl_seek(table, key)) == NULL)
                return -1;
        if (new_data->size > entry->e_data.size) {
                /* Need to allocate larger buffer */
                void *newbuf = malloc(new_data->size);
                if (!newbuf)
                        return -1;
                memcpy(newbuf, new_data->data, new_data->size);
                free(entry->e_data.data);
                entry->e_data.size = new_data->size;
                entry->e_data.data = newbuf;
        } else {
                /* Old buffer is sufficient. Overwrite it */
                memcpy(entry->e_data.data, new_data->data, new_data->size);
                /*
                 * REVISIT: We lose size of still-allocated memory, since
                 * free() doesn't need it.  Do we need it?
                 */
                entry->e_data.size = new_data->size;
        }
        return 0;
}

/**
 * eghash_iterate - Get the unordered next entry in the hash table
 * @table: Hash table handle
 * @state: Handle to the iterator state.  memset this to zero before the
 *         first call.
 * @key: Pointer to a struct to fill with entry key
 * @data: Pointer to a struct to fill with entry data
 *
 * Return: zero if @key and @data were filled with valid info from the
 * next entry; -1 if no more entries exists, or if there was an argument
 * error.
 *
 * This is used for getting data from the table in an arbitrary manner to
 * find matches in data attributes, tally sums, and do other such
 * structured query operations. As such, it works best on hash tables
 * whose data always contains the same structure or type.
 *
 * WARNING: eghash_iterate() will fill the .data pointers in @key and
 * @data with addresses of what should be treated as READ-ONLY VALUES.
 */
int
eghash_iterate(HashTable *table, eghash_iter_t *state,
               struct eghash_t *key, struct eghash_t *data)
{
        struct bucket_t *entry;
        size_t i;

        if (state == NULL)
                return -1;

        entry = state->_p;
        i = state->_i;

        if (i > table->t_size)
                return -1;

        if (entry != NULL && entry->e_child != NULL) {
                entry = entry->e_child;
                state->_p = entry;
                /* keep state->_i where it is */
        } else {
                if (entry != NULL)
                        ++i;
                for (; i < table->t_size; i++) {
                        if (table->t_entries[i] != NULL) {
                                entry = table->t_entries[i];
                                state->_p = entry;
                                state->_i = i;
                                goto out;
                        }
                }
                state->_p = NULL;
                state->_i = 0;
                return -1;
        }

out:
        if (entry != NULL) {
                memcpy(key, &entry->e_key, sizeof(*key));
                memcpy(data, &entry->e_data, sizeof(*data));
        }
        return 0;
}

/**
 * eghash_create - Create a new hash table
 * @size: Number of indices to make the table.  This value may not be
 *   zero.  Conventional wisdom holds that it should be a prime number,
 *   slightly larger than the number of expected insertions.  The table
 *   may grow if too many collisions or insertions general occur.  The
 *   table will not shrink, however.
 * @calc: User-defined function to calculate hash number.  If %NULL, the
 *    hash algorithms performed on this table returned will default to
 *    the library default.
 *
 * Return: Handle to a new hash table that uses @calc algorithm.
 *
 * BUGS: There is currently no mechanism for growing the hash table.
 * If you make it too small, then there will be lots of collisions,
 * resulting in lots of linked-list searching.  Pick a better value
 * for @size.
 *
 * Also, this library currently caches the entire database and has a
 * rather brute-force serialization method, so don't expect the most
 * optimal synching.
 */
HashTable EXPORT *
eghash_create(size_t size, unsigned long (*calc)(const void *, size_t))
{
        HashTable *table;

        if (size == 0)
                return NULL;

        table = malloc(sizeof(*table));
        if (!table)
                return NULL;

        if (eghash_create_(table, size, calc) < 0) {
                free(table);
                return NULL;
        }
        return table;
}

/**
 * eghash_destroy - Destroy a hash table in memory.
 * @table: Hash table to free
 *
 * Do not make any more calls using @table after destroying it with
 * eghash_destroy().  Also do not dereference any data retrieved from
 * a call to eghash_get() on the same hash table.
 */
void
eghash_destroy(HashTable *table)
{
        size_t i;
        for (i = 0; i < table->t_size; i++) {
                if (table->t_entries[i] != NULL)
                        delete_entry(table, table->t_entries[i], HDRECURSIVE);
        }
        free(table->t_entries);
        free(table);
}
