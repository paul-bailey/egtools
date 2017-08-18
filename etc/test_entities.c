/*
 * Used to compare current table-offset str2ent() and ent2str() with
 * older binary-search algo.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>

/* Keep this table ordered, for fast string lookups */
static const struct entity_lut_t {
        int c;
        const char *s;
} ENTITY_TBL[] = {
        { '&',  "AMP" }, { '`',  "DiacriticalGrave" }, { '>',  "GT" }, { '<',  "LT" },
        { '\t', "Tab" }, { '|',  "VerticalLine" }, { '&',  "amp" }, { '\'', "apos" },
        { '*',  "ast" }, { ':',  "colon" }, { ',',  "comma" }, { '@',  "commat" },
        { '$',  "dollar" }, { '=',  "equals" }, { '`',  "grave" }, { '>',  "gt" },
        { '^',  "hat" }, { '{',  "lbrace" }, { '[',  "lbrack" }, { '{',  "lcub" },
        { '_',  "lowbar" }, { '(',  "lpar" }, { '[',  "lsqb" }, { '<',  "lt" },
        { '*',  "midast" }, { ' ',  "nbsp" }, { '#',  "num" }, { '%',  "percent" },
        { '.',  "period" }, { '+',  "plus" }, { '?',  "quest" }, { '}',  "rbrace" },
        { ']',  "rbrack" }, { '}',  "rcub" }, { ')',  "rpar" }, { ']',  "rsqb" },
        { ';',  "semi" }, { '/',  "sol" }, { '|',  "verbar" }, { '|',  "vert" },
        { 0, NULL },
};

#define NTESTS (10 * 1000 * 1000)

static double
time_test2(const char *(*fn)(int c), int i)
{
        long j;
        clock_t t1, t2;
        const char *s;

        t1 = clock();
        for (j = 0; j < NTESTS; j++) {
                s = fn(ENTITY_TBL[i].c);
                assert(s != NULL);
        }
        t2 = clock();
        return (double)(t2 - t1) / (double)CLOCKS_PER_SEC;
}

static double
time_test(int (*fn)(const char *), int i)
{
        long j;
        clock_t t1, t2;
        int c;

        t1 = clock();
        for (j = 0; j < NTESTS; j++) {
                c = fn(ENTITY_TBL[i].s);
                assert(c != EOF);
        }
        t2 = clock();
        return (double)(t2 - t1) / (double)CLOCKS_PER_SEC;
}

extern int str2ent(const char *s);
extern int str2ent2(const char *s);
extern const char *ent2str(int c);
extern const char *ent2str2(int c);

#include <egdebug.h>
static void
validate_test(void)
{
        int i;
        for (i = 0; ENTITY_TBL[i].s != NULL; i++) {
                int c1 = str2ent(ENTITY_TBL[i].s);
                int c2 = str2ent2(ENTITY_TBL[i].s);
                assert(c1 == c2);
                assert(c1 != EOF);
        }

        for (i = 0; ENTITY_TBL[i].s != NULL; i++) {
                const char *s1 = ent2str(ENTITY_TBL[i].c);
                const char *s2 = ent2str2(ENTITY_TBL[i].c);
                if (s1 == s2)
                        break;
                assert(s1 != NULL);
                assert(s2 != NULL);
                /*
                 * Can't do a simple string compare, because an entity
                 * might have more than one valid string. EG:
                 *    strcmp("verbar", "vert") != 0
                 */
                int c1 = str2ent(s1);
                int c2 = str2ent(s2);
                assert(c1 != EOF);
                assert(c1 == c2);
        }
}


static void
print_times(double t1, double t2)
{
        if (t1 > t2) {
                printf(" %10.2f *%.2f\n", t1, t2);
        } else {
                printf("*%10.2f  %.2f\n", t1, t2);
        }
}

int main(void)
{
        int i;
        double sum1 = 0.0, sum2 = 0.0;

        validate_test();
        printf("str2ent:    str2ent2:\n");
        for (i = 0; ENTITY_TBL[i].s != NULL; i++) {
                double t1 = time_test(str2ent, i);
                double t2 = time_test(str2ent2, i);
                print_times(t1, t2);
                sum1 += t1;
                sum2 += t2;
        }
        printf("Sum:\n");
        print_times(sum1, sum2);

        sum1 = 0.0;
        sum2 = 0.0;
        printf("ent2str:    ent2str2:\n");
        for (i = 0; ENTITY_TBL[i].s != NULL; i++) {
                double t1 = time_test2(ent2str, i);
                double t2 = time_test2(ent2str2, i);
                print_times(t1, t2);
                sum1 += t1;
                sum2 += t2;
        }
        printf("Sum:\n");
        print_times(sum1, sum2);

        return 0;
}
