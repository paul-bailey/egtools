#include "config.h"
#include <egstring.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

struct egstring_t {
        char *buf;
        /* keep these signed */
        ssize_t size;
        ssize_t len;
};

static void
string_init(String *string)
{
        string->buf = NULL;
        string->size = 0;
        string->len = 0;
}

/**
 * string_cstring - Get the C-string pointer to @string
 *
 * WARNING: The return value is volatile; only use the return value
 * in a single thread, before the next call to string_putc() or other
 * such functions that operate on string.  Unless time is really so
 * precious to you, use string_dup() instead.
 */
const char *
string_cstring(String *string)
{
        return string->buf;
}

/**
 * string_dup - Get an allocated duplicate C-string of object @string
 *
 * You should free() the return value when you are finished with it.
 */
char *
string_dup(String *string)
{
        if (string->buf == NULL)
                return NULL;
        return strdup(string->buf);
}

/**
 * string_join - Concatenate two strings
 * @s1: First, earlier string
 * @s2: Second, later string
 *
 * Return: A third string that is @s1 followed by @s2, or NULL if memory
 *   could not be allocated for it.  Regardless of return value, @s1 and
 *   @s2 will not be modified.  The user will still need to free them.
 */
String *
string_join(String *s1, String *s2)
{
        String *ret = string_create(NULL);
        if (ret == NULL)
                return ret;

        if (string_append(ret, string_cstring(s1)) == EOF)
                goto err;
        if (string_append(ret, string_cstring(s2)) == EOF)
                goto err;
        return ret;

err:
        string_destroy(ret);
        return NULL;
}

/**
 * string_compare - Compare string @s1 to string @2
 *
 * Return: Less than zero if @s1 is less than @s2, zero if they are
 * equal, or greater than zero fi @s1 is greater than @s2
 */
const int
string_compare(String *s1, String *s2)
{
        if (s1 == s2 || s1->buf == s2->buf)
                return 0;
        if (s1->buf == NULL || s2->buf == NULL)
                return (int)(s1->buf - s2->buf);
        return strcmp(s1->buf, s2->buf);
}

/**
 * string_ccompare - Compare a String object's C-string to another
 * C-string
 * @str: A String object to compare
 * @s: The C-string to compare @str to
 *
 * Return: Less than zero if @str contains a C-string less than @s, zero if
 * @str contains a string that is equal to @s, and greater than zero if
 * @str contains a string that is greater than @s.
 */
const int
string_ccompare(String *str, const char *s)
{
        if (str->buf == s)
                return 0;
        if (str->buf == NULL)
                return -1;
        return strcmp(str->buf, s);
}

/**
 * string_strip - Remove leading and trailing whitespace in-place
 * @str: A String object to strip
 *
 * Note: Unlike some higher-level languages like Python, this does not
 * return a new String object.  Instead the operation occurs in place,
 * losing the original state of @str.
 */
void
string_strip(String *str)
{
        char *s = str->buf;
        size_t len;
        if (!s)
                return;
        while (isspace((int)(*s)))
                ++s;
        len = strlen(s);
        if (s != str->buf) {
                memmove(str->buf, s, len);
        }
        s = &str->buf[len];
        *s-- = '\0';
        while (isspace((int)(*s)) && s >= str->buf) {
                *s-- = '\0';
                --len;
        }
        str->len = len + 1;
}

/**
 * string_create - Create a string object with an empty C-string
 * @old: Usually NULL, but in this interest of saving malloc() calls, this
 *  could be an old, no-longer-needed String object to clobber.  It may be
 *  useful in loops
 *
 * Return: Pointer to a new String object if @old is NULL, a refurbished
 * @old if @old was not NULL, or NULL if memory cannot be allocated.
 * Do not free() this value directly. Instead call string_destroy().
 */
String *
string_create(String *old)
{
        String *ret;
        int c;

        if (old) {
                ret = old;
                ret->len = 0;
        } else {
                ret = malloc(sizeof(*ret));
                if (!ret)
                        return NULL;
                string_init(ret);
        }

        c = string_putc(ret, '\0');
        if (c == EOF) {
                string_destroy(ret);
                ret = NULL;
        }

        return ret;
}

/**
 * string_length - Equivalent to strlen(string_ccstring(str)), but possibly
 *         optimized
 * @str: String object
 *
 * Return: Length of the C-string contained by @str
 */
size_t
string_length(String *str)
{
        ssize_t len = str->len - 1;
        if (len < 0)
                len = 0;
        return len;
}

/**
 * string_destroy - Free all memory allocated to @string
 * @string: String object to free
 */
void
string_destroy(String *string)
{
        if (string->buf != NULL)
                free(string->buf);
        free(string);
}

/**
 * string_putc - Append a character to the end of a String object
 * @string: String object
 * @c: Character to append
 *
 * Return: @c if successfully appended; %EOF (from stdio.h) if not.  If
 * successful, the C-string's nul-char termination will be updated.
 */
int
string_putc(String *string, int c)
{
        while ((string->len + 1) >= string->size) {
                char *tmp = realloc(string->buf, string->size + 512);
                if (!tmp)
                        return EOF;

                string->buf = tmp;
                string->size += 512;
        }
        string->buf[string->len] = c;
        /* Don't permit nul chars in string */
        if (c != '\0')
                string->len++;
        /* Keep a String always nul terminated */
        string->buf[string->len] = '\0';
        return c;
}

/**
 * string_append - Append a C-string to the end of a String object
 * @string: String object
 * @s: C-string to append
 *
 * Return: 0 if @s was successfully appended; %EOF (from stdio.h) if not.
 * If successful, the C-string's nul-char termination will be updated.
 */
int
string_append(String *string, const char *s)
{
        int c;
        while ((c = *s++) != '\0') {
                c = string_putc(string, c);
                if (c == EOF)
                        return EOF;
        }
        return 0;
}

/**
 * string_printf - Append a formated string to a String object
 * @string: String object
 * @fmt: Formatted C-string to append, followed by variable arguments.
 *
 * Return: Number of characters appended if successful; -1 if not.  If
 * successful, the C-string's nul-char termination will be updated.
 *
 * The format follows the same rules as for vasprintf(3).
 */
int
string_printf(String *string, const char *fmt, ...)
{
        va_list ap;
        char *buf = NULL;
        int ret;

        va_start(ap, fmt);
        ret = vasprintf(&buf, fmt, ap);
        va_end(ap);

        if (ret >= 0 && buf != NULL) {
                if (string_append(string, buf) < 0)
                        ret = -1;
        }

        if (buf != NULL)
                free(buf);

        return ret;
}
