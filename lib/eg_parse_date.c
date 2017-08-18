#include "config.h"
#include "eg-devel.h"
#include <time.h>
#include <string.h>
#include <ctype.h>

static int
mday_valid(struct tm *tm)
{
        static const int days[] = {
               31, 29, 31, 30,
               31, 30, 31, 31,
               30, 31, 30, 31
        };
        if ((unsigned)tm->tm_mon > 11)
                return 0;
        if ((unsigned)tm->tm_mday > days[tm->tm_mon])
                return 0;
        return 1;
}

static char *
parse_helper(const char *buf, const char **fmts, struct tm *tm)
{
        int i;
        char *s;
        for (i = 0; fmts[i] != NULL; i++) {
                s = strptime(buf, fmts[i], tm);
                if (s) {
                        if (tm->tm_year > 0 && mday_valid(tm))
                                break;
                        else
                                s = NULL;
                }
        }
        return (char *)s;
}

static char *
parse_date(const char *buf, struct tm *tm)
{
        static const char *date_fmts[] = {
                "%m/%d/%Y",
                "%m-%d-%Y",
                "%m/%d/%y",
                "%m-%d-%y",
                "%b %d, %Y",
                "%b %d, %y",
                "%B %d, %Y",
                "%B %d, %y",
                "%B. %d, %Y",
                "%B. %d, %y",
                "%d %b %Y",
                "%d %b %y",
                "%d %B %Y",
                "%d %B %y",
                "%d %B. %Y",
                "%d %B. %y",
                NULL,
        };
        static const char *time_fmts[] = {
                "%I:%M:%S %p",
                "%I:%M:%S%p",
                "%T",
                NULL,
        };
        char *retsave;
        char *s;

        memset(tm, 0, sizeof(*tm));
        s = parse_helper(buf, date_fmts, tm);
        if (!s)
                return NULL;

        retsave = s;
        while (isspace((int)(*s)))
                ++s;
        s = parse_helper(s, time_fmts, tm);
        return s ? s : retsave;
}

/**
 * eg_parse_date - Parse a date string, with time if it follows
 * @s:  String to parse
 * @tm: Pointer to a struct to store date.
 *
 * Return: 0 if the date fields of @tm were filled in properly, -1 if
 * there was an error.  Errors include unreadable date format or invalid
 * month or day (eg if month is February, then 30 will be an invalid day).
 *
 * This is a wrapper around various calls to strptime(), with various
 * formats of descending priority, to handle several different ways of
 * expressing date.
 */
int
eg_parse_date(const char *s, struct tm *tm)
{
        s = eg_slide(s);
        s = parse_date(s, tm);

        return s ? 0 : -1;
}
