#include "eg-devel.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

void
eg_putchars(int c, int len)
{
        while (len > 0) {
                putchar(c);
                --len;
        }
}

void
eg_putspaces(int n)
{
        eg_putchars(' ', n);
}

const char *
eg_slide(const char *s)
{
        while (isblank((int)(*s)))
                ++s;
        return s;
}

const char *
eg_delimslide(const char *s, int delim)
{
        while (*s == delim)
                ++s;
        return s;
}

const char *
eg_delimsslide(const char *s, const char *cmp)
{
        int c;
        while (strchr(cmp, (c = *s)) != NULL && c != '\0')
                ++s;
        return s;
}

static int
isworddelim(int c)
{
        /* true also if c == '\0' */
        return strchr("\n\t ", c) != NULL;
}

static int
wordfits(int pos, int end, const char *s)
{
        const char *ssave = s;
        while (!isworddelim(*s))
                ++s;
        pos += (s - ssave);
        return pos < end;
}

static const char *
putword(const char *s)
{
        while (!isworddelim(*s)) {
                putchar(*s);
                ++s;
        }
        return s;
}

void
eg_paragraph(const char *s, unsigned int startcol,
             unsigned int endcol, unsigned int position)
{
        while (1) {
                if (position >= startcol) {
                        putchar('\n');
                        position = 0;
                }

                eg_putspaces(startcol - position);
                position = startcol;

                while (wordfits(position, endcol, s)) {
                        const char *ssave = s;
                        s = putword(s);
                        s = eg_slide(s);

                        if (*s == '\n') {
                                position = startcol;
                                break;
                        } else if (*s == '\0') {
                                putchar('\n');
                                return;
                        } else {
                                ++position;
                                putchar(' ');
                        }
                        position += (s - ssave);
                }
        }
}

#if 0
int
eg_ncols_ioc(void)
{
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_col;
}
#endif

void
bail_on(int cond, jmp_buf env, int ret)
{
        if (cond)
                longjmp(env, ret);
}

int
file_backup(const char *dst_path, const char *src_path)
{
        int c;
        int ret = -1;
        FILE *dst = NULL, *src = NULL;

        dst = fopen(dst_path, "w");
        if (!dst)
                goto err;
        src = fopen(src_path, "r");
        if (!src)
                goto err;
        while ((c = getc(src)) != EOF) {
                c = putc(c, dst);
                if (c == EOF)
                        goto err;
        }
        ret =  0;
err:
        if (dst != NULL)
                fclose(dst);
        if (src != NULL)
                fclose(src);
        return ret;
}
