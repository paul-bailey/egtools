#ifdef TEST_HASHTABLE__
#include "hashtable.h"

#include <assert.h>
#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <time.h>

static int
init_schedule(void)
{
        enum {
                /* TODO: FIFO, RR, or DEADLINE? */
                TRIGGERD_POLICY = SCHED_FIFO,
        };
        struct sched_param param;
        int priority;

        if ((priority = sched_get_priority_max(TRIGGERD_POLICY)) < 0)
                return -1;

        param.sched_priority = priority;
        if (sched_setscheduler(0, TRIGGERD_POLICY, &param) == -1)
                return -1;

        return 0;
}

static struct test_data_t test = {
        .num_lookups = 0,
        .max_lookup_time = 0.0,
        .sum_lookup_time = 0.0,
};

static void
mycleanup(char *s, void *data)
{
        /* Make this true just once to test if we're getting called */
        if (false)
                assert(false);

        assert(s == NULL);
        /* data declared on stack, so nothing to clean up */
        assert(data != NULL);
}

/* Unsophisticated test to check some basic things */
static void
simple_sanity_test(void)
{
        struct hashtable_t *tbl;
        int one = 1, two = 2, three = 3, dummy = 'd', res;
        void *data;
        bool b;

        tbl = hashtable_create(HTBL_COPY_KEY, NULL);
        assert(tbl);
        res = hashtable_put(tbl, "one", &one, 0, 0);
        assert(res == 0);
        res = hashtable_put(tbl, "one", &dummy, 0, 0);
        assert(res == -1);
        res = hashtable_put(tbl, "two", &two, 0, 0);
        assert(res == 0);
        res = hashtable_put(tbl, "three", &three, 0, 0);
        assert(res == 0);
        data = hashtable_get(tbl, "ONE", NULL);
        assert(data == NULL);
        data = hashtable_get(tbl, "one", NULL);
        assert(data != NULL);
        assert(data == &one);
        data = hashtable_get(tbl, "two", NULL);
        assert(data != NULL);
        assert(data == &two);
        data = hashtable_get(tbl, "three", NULL);
        assert(data != NULL);
        assert(data == &three);
        b = hashtable_remove(tbl, "ONE");
        assert(!b);
        b = hashtable_remove(tbl, "one");
        assert(b);
        hashtable_free(tbl, mycleanup);
}

static void
inperr(struct hashtable_t *tbl, int line, char *msg, ...)
{
        va_list ap;

        fprintf(stderr, "Error at line %d: ", line);
        va_start(ap, msg);
        vfprintf(stderr, msg, ap);
        va_end(ap);
        fprintf(stderr, "\nCurrent table stats:\n");
        hashtable_diag(&test, tbl, stderr);
        exit(1);
}

static void
test_one_input(struct hashtable_t *tbl, const char *command,
                const char *key, long v, int line)
{
        unsigned int put_flags = 0;
        size_t datalen;
        void *data;
        int res;
        clock_t tick, tock;
        double lookup_time;

        put_flags = 0;
        if (!strcasecmp(command, "clobber")) {
                put_flags = HTBL_CLOBBER;
                goto put;
        } else if (!strcasecmp(command, "put")) {
put:
                res = hashtable_put(tbl, key, &v, sizeof(v), put_flags);
                if (res != 0) {
                        inperr(tbl, line, errno == ENOMEM ? "OOM"
                                     : "Collision in hashtable_put");
                }
        } else if (!strcasecmp(command, "get")) {
                tick = clock();
                data = hashtable_get(tbl, key, &datalen);
                tock = clock();
                if (v < 0) {
                        if (data != NULL)
                                inperr(tbl, line, "expected failure");
                } else if (!data) {
                        inperr(tbl, line, "could not find %s", key);
                } else if (datalen != sizeof(long)) {
                        inperr(tbl, line,
                               "Returned data length %d", datalen);
                } else if (*(long *)data != v) {
                        inperr(tbl, line, "Expected %ld but got %ld",
                               v, *(long *)data);
                }
                ++test.num_lookups;
                lookup_time = (double)(tock - tick)
                        / (double)CLOCKS_PER_SEC;
                if (test.max_lookup_time < lookup_time)
                        test.max_lookup_time = lookup_time;
                test.sum_lookup_time += lookup_time;
        } else if (!strcasecmp(command, "diag")) {
                printf("Diagnostics requested at line %d:\n"
                       "----------------------------------\n", line);
                hashtable_diag(&test, tbl, stdout);
        } else if (!strcasecmp(command, "remove")) {
                if (hashtable_remove(tbl, key) != v)
                        inperr(tbl, line, "Could not remove %s", key);
        }
}

static void
input_test(FILE *fp)
{
        char command[512], key[512];
        long v;
        unsigned int line = 0;
        struct hashtable_t *tbl;

        tbl = hashtable_create(HTBL_COPY_KEY | HTBL_COPY_DATA, NULL);
        while (fscanf(fp, "%s %s %ld", command, key, &v) == 3) {
                ++line; /* incr first - "line 0" not very human */
                test_one_input(tbl, command, key, v, line);
        }
        hashtable_free(tbl, NULL);
}

int
main(int argc, char **argv)
{
        FILE *fpin = NULL;
        bool skip_input = false;
        if (argc > 1) {
                if (!strcmp(argv[1], "-f")) {
                        if (argc < 3) {
                                fprintf(stderr,
                                        "Expected: FILE for -f arg\n");
                                return 1;
                        }
                        fpin = fopen(argv[2], "r");
                        if (!fpin) {
                                fprintf(stderr,
                                        "Cannot open input file %s (%s)\n",
                                        argv[2], strerror(errno));
                                return 1;
                        }
                } else if (!strcmp(argv[1], "--skip-input")) {
                        skip_input = true;
                } else if (!strcmp(argv[1], "-?")
                           || !strcmp(argv[1], "-h")
                           || !strcmp(argv[1], "--help")) {
                        printf("Usage: %s [-f FILE] [--skip-input]\n"
                                "    -f FILE   Scan FILE instead of stdin\n"
                                "    --skip_input\n"
                                "              Skip test that reads input\n"
                                "              script.\n",
                                argv[0]);
                        return 0;
                }
        }
        if (init_schedule() < 0) {
                fprintf(stderr, "Could not set schedule priority (%s)\n"
                                "Timings measurement be more heavily\n"
                                "influenced by other programs.\n",
                                strerror(errno));
        }

        simple_sanity_test();
        if (!skip_input)
                input_test(fpin ? fpin : stdin);

        /* No assertion fails or program crashes :) */
        printf("Test complete\n");
        return 0;
}

#endif /* TEST_HASHTABLE__ */
