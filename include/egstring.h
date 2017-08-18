#ifndef EGSTRING_H
#define EGSTRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h> /* for size_t def */

#ifdef __GNUC__
  /* this is really nice */
# define eg_check_printf__(fmt_, argno_) \
        __attribute__((format(printf, fmt_, argno_)))
#else
# define eg_check_printf__(a_,b_)
#endif

typedef struct egstring_t String;
extern const char *string_cstring(String *string);
extern char *string_dup(String *string);
extern String *string_join(String *s1, String *s2);
extern const int string_compare(String *s1, String *s2);
extern const int string_ccompare(String *str, const char *s);
extern void string_strip(String *str);
extern String *string_create(String *old);
extern size_t string_length(String *string);
extern void string_destroy(String *string);
extern int string_putc(String *string, int c);
extern int string_append(String *string, const char *new);
extern int string_printf(String *string,
                         const char *fmt, ...) eg_check_printf__(2, 3);

#ifdef __cplusplus
}
#endif
#endif /* EGSTRING_H */
