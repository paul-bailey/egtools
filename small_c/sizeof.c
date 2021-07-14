#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <setjmp.h>
#include <time.h>
#include <signal.h>

#define SIZEOF_PARAMS(type) \
        { .s = #type, .size = sizeof(type), }

static const struct sizeof_tbl_t {
        const char *s;
        size_t size;
} sizeof_tbl[] = {
        SIZEOF_PARAMS(uint8_t),
        SIZEOF_PARAMS(uint_fast8_t),
        SIZEOF_PARAMS(uint_least8_t),
        SIZEOF_PARAMS(uint16_t),
        SIZEOF_PARAMS(uint_fast16_t),
        SIZEOF_PARAMS(uint_least16_t),
        SIZEOF_PARAMS(uint32_t),
        SIZEOF_PARAMS(uint_fast32_t),
        SIZEOF_PARAMS(uint_least32_t),
        SIZEOF_PARAMS(char),
        SIZEOF_PARAMS(short),
        SIZEOF_PARAMS(int),
        SIZEOF_PARAMS(long),
        SIZEOF_PARAMS(long long),
        SIZEOF_PARAMS(jmp_buf),
        SIZEOF_PARAMS(size_t),
        SIZEOF_PARAMS(void *),
        SIZEOF_PARAMS(intptr_t),
        SIZEOF_PARAMS(uintptr_t),
        SIZEOF_PARAMS(off_t),
        SIZEOF_PARAMS(dev_t),
        SIZEOF_PARAMS(gid_t),
        SIZEOF_PARAMS(mode_t),
        SIZEOF_PARAMS(uid_t),
        SIZEOF_PARAMS(off_t),
        SIZEOF_PARAMS(pid_t),
#if HAVE_LOFF_T
        SIZEOF_PARAMS(loff_t),
#endif
        SIZEOF_PARAMS(ssize_t),
        SIZEOF_PARAMS(size_t),
        SIZEOF_PARAMS(daddr_t),
        SIZEOF_PARAMS(caddr_t),
        SIZEOF_PARAMS(time_t),
        SIZEOF_PARAMS(sig_atomic_t),
        // SIZEOF_PARAMS(useconds_t),
        SIZEOF_PARAMS(suseconds_t),
        SIZEOF_PARAMS(register_t),
        SIZEOF_PARAMS(FILE),
        { .s = NULL },
};

int main(int argc, char **argv)
{
        const struct sizeof_tbl_t *t;
        if (argc != 2) {
                fprintf(stderr,
                        "%s: Expected: TYPE (use quotes if TYPE has a space)\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        } else if (argv[1][0] == '-') {
                const char *s = argv[1];
                while (*s == '-')
                        ++s;
                if (!strcmp(s, "help") || !strcmp(s, "h") || !strcmp(s, "?")) {
                        printf("Expected: sizeof TYPE, where TYPE is one of:\n");
                        for (t = sizeof_tbl; t->s != NULL; ++t) {
                                printf("\t\"%s\"\n", t->s);
                        }
                        printf("(Use quotes for multi-word types)\n");
                        exit(EXIT_SUCCESS);
                }
                /* else, fall through, maybe it's a type? */
        }

        for (t = sizeof_tbl; t->s != NULL; ++t) {
                if (!strcmp(t->s, argv[1])) {
                        printf("%d\n", (int)t->size);
                        return EXIT_SUCCESS;
                }
        }
        fprintf(stderr, "%s: Unknown type \"%s\"\n", argv[0], argv[1]);
        return EXIT_FAILURE;
}
