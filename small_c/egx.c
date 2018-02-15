#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int verbose = 0;

static uint32_t djb_hash(const char *xs)
{
        uint32_t hash = 0;
        int c;

        while ((c = *xs++) != '\0')
                hash = hash * 33 + c;
        return hash;
}

static int
count_bits(uint8_t hash)
{
        int i;
        int count = 0;
        for (i = 0; i < 8; i++) {
                if (!!(hash & 1))
                        ++count;
                hash >>= 1;
        }
        return count;
}

static uint8_t hashcollapse(uint32_t hash)
{
        return ((hash >> 24) | (hash >> 16)) ^ ((hash >> 8) | hash);
}

static uint8_t
calchash(const char *xs)
{
        uint32_t hash32;
        uint8_t hash8;
        int count;
        int spinlock_count = 0;

        hash32 = djb_hash(xs);
        do {
                hash8 = hashcollapse(hash32);
                count = count_bits(hash8);
                hash32++;
                if (++spinlock_count > 1000000) {
                        fprintf(stderr, "Cannot calculate insane hash!\n");
                        exit(EXIT_FAILURE);
                }
        } while (count > 6 || count < 3);
        return hash8;
}

static void
egx(const char *xs)
{
        int c;
        uint8_t hash = calchash(xs);
        if (verbose) {
                fprintf(stderr, "hash = 0x%02X\n", (unsigned int)hash);
                exit(EXIT_SUCCESS);
        }

        while ((c = getchar()) != EOF) {
                c ^= hash;
                putchar(c);
        }
}

static void
help(FILE *fp)
{
        fprintf(fp, "Expected: egx -f XFILE, or egx XS\n");
}

int
main(int argc, char **argv)
{
        int opt;
        char *xfile = NULL;
        char *xs;

        while ((opt = getopt(argc, argv, "vf:?")) != -1) {
                switch (opt) {
                case '?':
                        help(stdout);
                        return 0;
                case 'f':
                        xfile = optarg;
                        break;
                case 'v':
                        verbose = 1;
                        break;
                default:
                        help(stderr);
                        return 1;
                }
        }

        if (xfile != NULL) {
                FILE *fp = fopen(xfile, "r");
                size_t count = 0;
                char *s;
                if (fp == NULL)
                        goto exfile;
                xs = NULL;
                if (getline(&xs, &count, fp) == -1)
                        goto exfile;
                /* strip newline from end of s */
                for (s = xs; *s != '\0'; s++) {
                        if (*s == '\n') {
                                *s = '\0';
                                break;
                        }
                }
        } else if (optind >= argc) {
                help(stderr);
                return 1;
        } else {
                /* Get xs from command line */
                xs = strdup(argv[optind]);
                if (!xs) {
                        perror("OOM!");
                        return 1;
                }
        }

        egx(xs);
        free(xs);
        return 0;

exfile:
        perror("Cannot parse xfile");
        return 1;
}
