#ifndef EGTOK_H
#define EGTOK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <egstring.h>

/**
 * enum egtoken_type_t - Type of a token
 * @EGTOK_EXPR: Token is an expression which may have internal quotes
 * @EGTOK_DQUOT: Token is entirely wrapped inside double quotes
 * @EGTOK_SQUOT: Token is entirely wrapped inside single quotes
 * @EGTOK_EOL: Not a token; reached unescaped end of line.
 * @EGTOK_EOS: Not a token; reached end of the string to parse.
 * @EGTOK_DELIM: Token is a single-character delimiter.
 * @EGTOK_UNTERM_QUOT: Token is an expression that had an unterminated quote
 *               quote
 * @EGTOK_UNDEF_ESCAPE: Token is an expression that had an undefined backslash
 *               backslash escape
 * @EGTOK_UNDEF_EXPAND: %EGDOK_EXPAND was set but an invalid environment
 *              variable name was used (eg "${a" instead of "${a}" - not
 *              the same as an environment variable not being set).
 */
enum egtoken_type_t {
        EGTOK_EXPR = 1,
        EGTOK_DQUOT,
        EGTOK_SQUOT,
        EGTOK_EOL,
        EGTOK_EOS,
        EGTOK_DELIM,
        EGTOK_UNTERM_QUOT,
        EGTOK_UNDEF_ESCAPE,
        EGTOK_UNDEF_EXPAND,
};


/**
 * struct egtoken_t - Structure to handle getting tokens
 * @str: Pointer to the token string
 * @type: Type of token returned
 *
 * Initialize this once with eg_token_start(), then use it whenever
 * needed with eg_token(), and finally free any allocated memory in
 * the struct with eg_token_end().  At all times, treat the token's
 * contents as READ ONLY.
 */
struct egtoken_t {
        String *string;
        enum egtoken_type_t type;
};

#define EGTOK_EOLESC            0x0001
#define EGTOK_EXPAND            0x0004
#define EGTOK_NOESCP            0x0008

/* If using EGTOK_EXPAND, this is the longest permissible environment
 * variable length.
 */
#define EGTOK_MAXEXPAND         512

extern const char *eg_parse_env(const char *s, char **endptr);
extern int eg_token_init(struct egtoken_t *tok);
extern void eg_token_exit(struct egtoken_t *tok);
extern char *eg_token(const char *s, struct egtoken_t *tok,
                      const char *delims, unsigned int flags);


#ifdef __cplusplus
}
#endif

#endif /* EGTOK_H */
