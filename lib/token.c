#include "config.h"
#include "eg-devel.h"
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <ctype.h>
#include <stdbool.h>

int
eg_token_init(struct egtoken_t *tok)
{
        memset(tok, 0, sizeof(*tok));
        tok->string = string_create(NULL);
        if (tok->string == NULL)
                return -1;
        return 0;
}

void
eg_token_exit(struct egtoken_t *tok)
{
        if (tok->string != NULL)
                string_destroy(tok->string);
        memset(tok, 0, sizeof(*tok));
}


/**
 * eg_parse_env - Return environment variable expanded from @s
 * @s:  String containing envrionment variable, pointing to the first
 *      character after '$'.  It may be encased in {}, but it may
 *      only contain letters, numbers, or underscores.  Cute bash-ful
 *      variables like ${a:=b} are invalid.  Ditto for $#, $*, etc.
 *      The variable also may not have a name longer than
 *      %EGTOK_MAXEXPAND, defined in "eg-devel.h".
 * @endptr: Pointer to a variable to store the first character
 *      after the environment variable.  If @s is invalid (return value
 *      is NULL), @endptr will point to original @s.
 *
 * Returns: NULL if @s is invalid, a pointer to the result of getenv() if
 *      the environment variable exists (ie getevn() was successful),
 *      or a pointer to an empty string if the environment variable does
 *      not exist.
 */
const char *
eg_parse_env(const char *s, char **endptr)
{
        static char emptybuf = '\0';
        char buf[EGTOK_MAXEXPAND];
        char *pbuf;
        const char *ssave = s;
        int c;
        int quoted = false;
        char *ret;

        c = *s;
        if (c == '{') {
                quoted = true;
                ++s;
        }

        pbuf = buf;
        while (isalnum(c = *s) || c == '_') {
                *pbuf++ = c;
                ++s;
        }

        if (pbuf == buf)
                goto inval;

        if (quoted) {
                if (c != '}')
                        goto inval;
                ++s;
        }
        *endptr = (char *)s;
        ret = getenv(buf);
        if (!ret)
                ret = &emptybuf;
        return ret;

inval:
        *endptr = (char *)ssave;
        return NULL;
}

static char *
unescaped_token(const char *s, struct egtoken_t *tok,
                   const char *delims)
{
        int c = *s;

        /* tok->env_, tok->len_, and s already set */
        if (c == '\0') {
                tok->type = EGTOK_EOS;
        } else if (c == '\n') {
                tok->type = EGTOK_EOL;
                if (string_putc(tok->string, c) == EOF)
                        goto err;
                ++s;
        } else if (strchr(delims, c)) {
                tok->type = EGTOK_DELIM;
                if (string_putc(tok->string, c) == EOF)
                        goto err;
                ++s;
        } else {
                tok->type = EGTOK_EXPR;
                while (!isspace(c = *s) && !strchr(delims, c)) {
                        if (string_putc(tok->string, c) == EOF)
                                goto err;
                        ++s;
                }
        }

        return (char *)s;

err:
        return NULL;
}

/* Helper macros for expression_token() */
#define C_NOPUT -1
#define C_FINISH -2

/* Token is figured out to be some kind of expression */
static char *
expression_token(const char *s, struct egtoken_t *tok,
                 const char *delims, unsigned int flags)
{
        int c;
        int quoted = false;
        int quote_type = '\0';

        c = *s;
        if (c == '"' || c == '\'') {
                tok->type = c == '"' ? EGTOK_DQUOT : EGTOK_SQUOT;
                quoted = true;
                quote_type = c;
        } else {
                tok->type = EGTOK_EXPR;
                --s;
        }

        do {
                /*
                 * If we go unquoted once, then we're an ordinary
                 * expression that may or may not be escaped by quotes.
                 */
                if (!quoted
                    && (tok->type == EGTOK_DQUOT || tok->type == EGTOK_SQUOT)) {
                        tok->type = EGTOK_EXPR;
                }

                c = *(++s);
                if (c == '\0') {
                        if (quoted)
                                tok->type = EGTOK_UNTERM_QUOT;
                        c = C_FINISH;
                } else if (c == '\\') {
                        static const char *escstr = "0vntbf";
                        static const char *esctbl = "\0\v\n\t\b\f";
                        char *escptr;
                        c = *(++s);
                        if (c == '\0') {
                                tok->type = EGTOK_UNDEF_ESCAPE;
                                c = C_FINISH;
                        } else if ((escptr = strchr(escstr, c)) != NULL) {
                                /* translate an escape char */
                                c = esctbl[escptr - escstr];
                        } else if (c == '\n') {
                                /* Escape newline if we're allowed */
                                if (!!(flags & EGTOK_EOLESC)) {
                                        c = C_NOPUT;
                                } else {
                                        tok->type = EGTOK_UNDEF_ESCAPE;
                                        c = C_FINISH;
                                }
                        } else if (!strchr("\"\\'", c)) {
                                /* Unknown escape char */
                                tok->type = EGTOK_UNDEF_ESCAPE;
                        }
                } else if (c == '$') {
                        if (!!(flags & EGTOK_EXPAND)) {
                                const char *env = eg_parse_env(s + 1,
                                                         (char **)&s);
                                if (env) {
                                        c = C_NOPUT;
                                        if (string_append(tok->string, s)
                                            == EOF) {
                                                goto err;
                                        }
                                } else {
                                        /* Just copy the '$' */
                                        tok->type = EGTOK_UNDEF_EXPAND;
                                }
                                /*
                                 * eg_parse_env() points to first char
                                 * after var (or after $ if it fails).
                                 * To stay consistent with loop, point to
                                 * last char in var (or at $ if fail).
                                 */
                                --s;
                        }
                        /* else, just put '$' */
                } else if (quoted) {
                        if (c == quote_type) {
                                quote_type = '\0';
                                quoted = false;
                        } else if (c == '\n') {
                                if (!(flags & EGTOK_EOLESC)) {
                                        tok->type = EGTOK_UNTERM_QUOT;
                                        c = C_FINISH;
                                }
                        }
                } else if (c == '\n' || strchr(delims, c) || isblank(c)) {
                        c = C_FINISH;
                }

                if (c != C_NOPUT) {
                        if (string_putc(tok->string, c) == EOF)
                                goto err;
                }
        } while (c != C_FINISH);
        /*
         * currently, we're pointing at the end of the token, not start
         * of the next one.  Advance to "first character after..."
         */
        if (*s != '\0')
                ++s;

        /* final token_putc() handled below */
        return (char *)s;

err:
        return NULL;
}

/**
 * eg_token - Get a token
 * @s: String to parse
 * @tok: A token that has been initialized.  It does not need to be
 *         initialized for every call to eg_token(), just the first
 *         time.
 * @delims: A string containing the delimiters that will result in
 *         @tok's @type field being %EGTOK_DELIM.
 * @flags: Flags; see discussion below.
 *
 * Return: Pointer to the next character after the token, or NULL if some
 * critical error resulted in @tok not being edited.
 *
 * Since unescaped, unquoted whitespace is always to be a separator, any
 * leading whitespace is skipped.  There will be no need to do that
 * manually if the return value points to whitespace.
 *
 * The @flags field:
 *
 * %EGTOK_EOLESC: Allow quotes or backslashes to escape '\n'.
 *
 * %EGTOK_EXPAND: If an unescaped '$' character is encountered, expand
 * the following environment variable name to an actual environment
 * variable, or to an empty string if the environment variable does
 * not exist (see eg_parse_env()).
 *
 * %EGTOK_NOESCP: Don't escape a damn thing, by quotes or by backslashes.
 * Copy all characters into @tok verbatim until you hit whitespace or the
 * next delimiter.  This nullifies any other flags.
 */
char *
eg_token(const char *s, struct egtoken_t *tok,
         const char *delims, unsigned int flags)
{
        int c;

        tok->string = string_create(tok->string);
        if (tok->string == NULL)
                return NULL;

        while (isblank(c = *s))
                ++s;

        if (!!(flags & EGTOK_NOESCP)) {
                /* Prefer the country to the court? */
                s = unescaped_token(s, tok, delims);
        } else if (c == '\0') {
                tok->type = EGTOK_EOS;
        } else if (c == '\n' || strchr(delims, c)) {
                tok->type = (c == '\n') ? EGTOK_EOL : EGTOK_DELIM;
                if (string_putc(tok->string, c) == EOF)
                        return NULL;
                ++s;
        } else {
                s = expression_token(s, tok, delims, flags);
        }

        return (char *)s;
}
