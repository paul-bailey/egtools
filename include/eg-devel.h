#ifndef EG_DEVEL_H
#define EG_DEVEL_H

#include <egstring.h>
#include <egxml.h>
#include <egtok.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <setjmp.h>

/*
 * Misc. helpers not in another header
 */

extern void eg_putchars(int c, int len);
extern void eg_putspaces(int n);
extern const char *eg_slide(const char *s);
extern const char *eg_delimslide(const char *s, int delim);
extern const char *eg_delimsslide(const char *s, const char *cmp);
extern void eg_paragraph(const char *s, unsigned int startcol,
                         unsigned int endcol, unsigned int position);

struct tm;
extern int eg_parse_date(const char *s, struct tm *tm);

extern uint32_t eg_fletcher32(uint16_t *data, size_t len);

#define EG_BINARY    (0x0001U)
#define EG_RECURSIVE (0x0002U)
extern int eg_dir_foreach(const char *path, unsigned int flags,
                          int (*fn)(FILE *, void *), void *priv);
extern int eg_sdir_foreach(const char *path, unsigned int flags,
                           int (*fn)(const char *, void *), void *priv);

extern int eg_popd(void);
extern int eg_pushd(const char *dir);

/* TODO: Deprecate this - it was a bad idea */
extern void bail_on(int cond, jmp_buf env, int ret);
extern int file_backup(const char *dst_path, const char *src_path);

#ifdef __cplusplus
}
#endif

#endif /* EG_DEVEL_H */
