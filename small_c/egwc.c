/*
 * TODO: Add license stuff, etc.
 * This is derivative of GNU,
 * so it's GPLv3
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>

typedef unsigned long count_t; /* counter type */

/* Current file counters: chars, words, lines */

/* Totals counters: chars, words, lines */
count_t total_ccount = 0;
count_t total_wcount = 0;
count_t total_lcount = 0;

static void
error_print(int perr, char *fmt, va_list ap)
{
        vfprintf(stderr, fmt, ap);
        if (perr)
                perror(" ");
        else
                fprintf(stderr, "\n");
        exit(1);
}

/* Print error message & high-tail it */
static void
errf(char *fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        error_print(0, fmt, ap);
        va_end(ap);
}

static void
perrf(char *fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        error_print(1, fmt, ap);
        va_end(ap);
}

/* Output counters for given file */
void
report(char *file, count_t ccount, count_t wcount, count_t lcount)
{
        printf("%6lu %6lu %6lu %s\n", lcount, wcount, ccount, file);
}

static int roffify = 0;

static int
isexclusive(int c, int lastc)
{
        if (!roffify)
                return 0;

        /* TODO: What about macros that apply text? */
        return (c == '"' && lastc == '\\')
               || (c == '.' && lastc == '\n');
}

/* Process file FILE */
void
counter(char *file)
{
        count_t ccount = 0;
        count_t wcount = 0;
        count_t lcount = 0;
        int exclude = 0;
        int c;
        int lastc = '\n';
        FILE *fp = fopen(file, "r");
        if (!fp)
                perrf("cannot open file `%s'", file);

        while ((c = getc(fp)) != EOF) {
                ++ccount;
                if (c == '\n') {
                        ++lcount;
                        exclude = 0;
                } else if (isexclusive(c, lastc)) {
                        /* loophole in this check, just fix it here. */
                        if (!exclude && c == '"' && lastc == '\\')
                                --wcount;
                        exclude = 1;
                } else if (!isspace(c) && isspace(lastc) && !exclude) {
                        ++wcount;
                }

                lastc = c;
        }
        fclose(fp);

        report(file, ccount, wcount, lcount);
        total_ccount += ccount;
        total_wcount += wcount;
        total_lcount += lcount;
}


int
main(int argc, char **argv)
{
        int i;
        int opt;

        while ((opt = getopt(argc, argv, "r")) != -1) {
                switch (opt) {
                case 'r':
                        roffify = 1;
                        break;
                default:
                case '?':
                        errf("usage: wc [-r] FILE [FILE...]");
                        break;
                }
        }

        if ((argc - optind) < 1)
                errf("usage: wc FILE [FILE...]");

        for (i = optind; i < argc; i++)
                counter(argv[i]);

        if ((argc - optind) > 1)
                report("total", total_ccount, total_wcount, total_lcount);
        return 0;
}
