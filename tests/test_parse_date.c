#include "eg-devel.h"
#include <string.h>
#include <time.h>
#include <stdio.h>

struct expect_t {
        const char *s;
        int res;
        int mon;
        int year;
        int mday;
};

#define prexpect(t_, what_) \
        fprintf(stderr, "Expected %s %d but got %d for \"%s\"\n", \
                #what_, (t_)->what_, tm.tm_##what_, (t_)->s);

static int
test_one(const struct expect_t *t)
{
        struct tm tm;
        int res;
        memset(&tm, 0, sizeof(tm));
        res = eg_parse_date(t->s, &tm);
        if (res != t->res) {
                fprintf(stderr, "Expected: result=%d for \"%s\"\n",
                        t->res, t->s);
                return 1;
        }

        /* Leave early if we expected an error */
        if (t->res < 0)
                return 0;

        if (tm.tm_year != t->year - 1900) {
                prexpect(t, year);
                return 1;
        }

        if (tm.tm_mday != t->mday) {
                prexpect(t, mday);
                return 1;
        }

        if (tm.tm_mon != t->mon) {
                prexpect(t, mon);
                return 1;
        }
        return 0;
}

int
main(void)
{
        static const struct expect_t expect[] = {
                { "8/15/16", 0, 7, 2016, 15 },
                { "2/29/2011", 0, 1, 2011, 29 },
                { "2/29/2011 ", 0, 1, 2011, 29 },
                { "2/30/2011", -1, 0, 0, 0 },
                { "July 27, 1996", 0, 6, 1996, 27 },
                { "July 23, 2006", 0, 6, 2006, 23 },
                { "July 23, 2006\n", 0, 6, 2006, 23 },
                { NULL, 0, 0, 0, 0 },
        };
        const struct expect_t *t;
        int ret;
        for (t = expect; t->s != NULL; t++) {
                ret = test_one(t);
                if (ret)
                        break;
        }
        return ret;
}
