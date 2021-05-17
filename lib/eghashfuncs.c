#include <eghash.h>
#include <stdint.h>

#ifndef EXPORT
# ifdef __GNUC__
#  define EXPORT __attribute__((visibility("default")))
# else
#  define EXPORT
# endif
#endif

#define DJB_MAGIC 5381

static unsigned long
keep_positive(unsigned long v)
{
        if ((long)v < 0)
                v = -v;
        return v;
}

/**
 * eg_djb_hash - Dan Bernstein hash
 * @data: Byte array to hash
 * @size: size of @data, in bytes
 *
 * Return: Hash number.  If hash algorithm results in the highest bit
 * being set, then the two's complement will be returned to keep it
 * positive, whether signed or not.
 *
 * See <http://www.cse.yorku.ca/~oz/hash.html
 */
unsigned long EXPORT
eg_djb_hash(const void *data, size_t size)
{
        unsigned long hash = DJB_MAGIC;
        const char *s = data;
        unsigned long i;

        for (i = 0; i < size; i++)
                hash = ((hash << 5) + hash) + (unsigned long)s[i];

        return keep_positive(hash);
}

/**
 * eg_djb2_hash - Like eg_djb_hash, but using XOR rather than add
 * @data: Byte array to hash
 * @size: size of @data, in bytes
 *
 * Return: Hash number.  If hash algorithm results in the highest bit
 * being set, then the two's complement will be returned to keep it
 * positive, whether signed or not.
 *
 * See <http://www.cse.yorku.ca/~oz/hash.html
 */
unsigned long EXPORT
eg_djb2_hash(const void *data, size_t size)
{
        unsigned long hash = DJB_MAGIC;
        const char *s = data;
        unsigned long i;

        for (i = 0; i < size; i++)
                hash = ((hash << 5) + hash) ^ (unsigned long)s[i];

        return keep_positive(hash);
}

/**
 * eg_sdbm_hash - Hash that uses SDBM algorithm
 * @data: Byte array to hash
 * @size: size of @data, in bytes
 *
 * Return: Hash number.   If hash algorithm results in the highest bit
 * being set, then the two's complement will be returned to keep it
 * positive, whether signed or not.
 *
 * See <http://www.cse.yorku.ca/~oz/hash.html
 */
unsigned long EXPORT
eg_sdbm_hash(const void *data, size_t size)
{
        unsigned long hash = 0;
        const char *s = data;
        unsigned long i;

        /* below algo is machine shorthand for "c + hash * 65599" */
        for (i = 0; i < size; i++) {
                hash = (unsigned long)s[i]
                       + ((hash << 6) + (hash << 16) - hash);
        }

        return keep_positive(hash);
}

/* TODO: Find Paul Hsieh's license and use it */
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

/**
 * eg_psh_hash - Paul Hsieh's hash functions
 * @data: Byte array to hash
 * @size: size of @data, in bytes
 *
 * Return Hash number.  The MSB may or may not be set.
 *
 * See <www.azillionmonkeys.com/qed/hash.html>, but don't spend too much
 * time there.  His rants are only so useful, and he even defends BASIC.
 */
unsigned long EXPORT
eg_psh_hash(const void* data, size_t size)
{
        uint32_t hash = size, tmp;
        const char *sdata = data;
        int rem;

        if ((int)size <= 0 || sdata == NULL)
                return 0;

        rem = size & 3;
        size >>= 2;

        /* Main loop */
        for (;(int)size > 0; size--) {
                hash  += get16bits (sdata);
                tmp    = (get16bits (sdata + 2) << 11) ^ hash;
                hash   = (hash << 16) ^ tmp;
                sdata += 2 * sizeof (uint16_t);
                hash  += hash >> 11;
        }

        /* Handle end cases */
        switch (rem) {
        case 3: hash += get16bits (sdata);
                hash ^= hash << 16;
                hash ^= ((signed char)sdata[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (sdata);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*sdata;
                hash ^= hash << 10;
                hash += hash >> 1;
        }

        /* Force "avalanching" of final 127 bits */
        hash ^= hash << 3;
        hash += hash >> 5;
        hash ^= hash << 4;
        hash += hash >> 17;
        hash ^= hash << 25;
        hash += hash >> 6;

        return (unsigned long)hash;
}

/**
 * Git's version of a hash algorithm
 */
unsigned long EXPORT
eg_git_hash(const void *buf, size_t len)
{
        unsigned long hash = 0x811c9dc5;
        unsigned char *ucbuf = (unsigned char *) buf;
        while (len--) {
                unsigned int c = *ucbuf++;
                hash = (hash * 0x01000193) ^ c;
        }
        return hash;
}
