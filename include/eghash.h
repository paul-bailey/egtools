#ifndef EGHASH_H
#define EGHASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

struct eghash_t {
        union {
                /*
                 * REVISIT: This assumes that union between pointer and
                 * uintptr_t is safe, IE that sizeof(uintptr_t) == sizeof(pointer)
                 * and never sizeof(uintptr_t) > sizeof(pointer), or that
                 * endianness is not a problem in such a case.  This data
                 * may be written to, and read back from, a file on disk.
                 */
                void *data;
                uintptr_t _fdata;
        };
        size_t size;
};

typedef struct HashTable HashTable;
typedef unsigned long (*hashfunc_t)(const void *data, size_t size);

/* The main hash functions */
extern HashTable *eghash_create(size_t size, hashfunc_t calc);
extern void eghash_destroy(HashTable *table);
extern int eghash_put(HashTable *table, const struct eghash_t *key,
                      const struct eghash_t *data);
extern int eghash_get(HashTable *table, const struct eghash_t *key,
                      struct eghash_t *data);
extern int eghash_remove(HashTable *table, const struct eghash_t *key);
extern int eghash_change(HashTable *table, const struct eghash_t *key,
                         const struct eghash_t *new_data);

typedef struct eghash_iter_t {
        unsigned long _i;
        void *_p;
} eghash_iter_t;

extern int eghash_iterate(HashTable *table, eghash_iter_t *state,
                          struct eghash_t *key, struct eghash_t *data);

/* Serialization */
extern int eghash_sync(HashTable *table, int fd);
extern HashTable *eghash_open(int fd, hashfunc_t calc);

/* convenient hash algos if you don't want to write your own */
extern unsigned long eg_djb_hash(const void *data, size_t size);
extern unsigned long eg_djb2_hash(const void *data, size_t size);
extern unsigned long eg_sdbm_hash(const void *data, size_t size);
extern unsigned long eg_psh_hash(const void *data, size_t size);

/**
 * struct eghash_diag_t - Diagnostic info for testing hashing etc.
 * @hits: Number of hash table indices with data
 * @misses: Number of hash table indices with no data
 * @size: Size, in bytes, of the table metadata, its entries, and their
 *  keys and data, if written to file
 * @collisions: Total number of collisions
 * @maxcollisions: Size of the largest collision list.  '1' implies no
 * collision.
 */
struct eghash_diag_t {
        unsigned long hits;
        unsigned long misses;
        unsigned long size;
        unsigned long collisions;
        unsigned long maxcollisions;
};

extern void eghash_diag(HashTable *table, struct eghash_diag_t *diag);


#ifdef __cplusplus
}
#endif

#endif /* EGHASH_H */
