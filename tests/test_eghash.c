#include <eghash.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <egdebug.h>
#include <fcntl.h>

#define METHOD eg_git_hash
#define TABSIZ 1

static void
diag(HashTable *tbl)
{
        struct eghash_diag_t diag;
        eghash_diag(tbl, &diag);
        printf("hits: %lu\n", diag.hits);
        printf("misses: %lu\n", diag.misses);
        printf("size: %lu\n", diag.size);
        printf("collisions: %lu\n", diag.collisions);
        printf("maxcollisions: %lu\n", diag.maxcollisions);
}

/*
 * This only tests effectiveness of table, it doesn't
 * really check it for bugs.
 */
int
main(void)
{
        enum { WORDSIZE = 256 };
        char *line = NULL;
        size_t nalloc = 0;
        HashTable *tbl;
        struct eghash_t hashpair[2];
        char words[2][WORDSIZE];

        tbl = eghash_create(TABSIZ, METHOD);
        assert(tbl != NULL);
        for (;;) {
                int i, res;
                for (i = 0; i < 2; i++) {
                        memset(words[i], 0, WORDSIZE);
                        if (getline(&line, &nalloc, stdin) == -1)
                                goto out;
                        strncpy(words[i], line, WORDSIZE-1);
                        hashpair[i].data = (void *)words[0];
                        hashpair[i].size = strlen(words[0]) + 1;
                }
                res = eghash_put(tbl, &hashpair[0], &hashpair[1]);
                assert(res == 0);
        }
out:
        diag(tbl);
        return 0;
}


