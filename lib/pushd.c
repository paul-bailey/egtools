#include "config.h"
#include "eg-devel.h"
#include <unistd.h>
#include <stdlib.h>
/* TODO: If HAVE_SYS_PARAM_H.., gets MAXPATHLEN */
#include <sys/param.h>

#ifndef MAXPATHLEN
# define MAXPATHLEN 1024
#endif

#define MAXPATHSTACK 100

static char *path_stack[MAXPATHSTACK];
static int path_stack_ptr = 0;

int
eg_pushd(const char *dir)
{
        char *p;
        if ((unsigned)path_stack_ptr >= MAXPATHSTACK) {
                /* XXX: Too many calls, need to set errno */
                goto estack;
        }
        p = malloc(MAXPATHLEN);
        if (!p)
                goto estack;
        if (getcwd(p, MAXPATHLEN) == NULL)
                goto ecwd;
        p[MAXPATHLEN - 1] = '\0';
        if (chdir(dir) < 0)
                goto ecwd;
        path_stack[path_stack_ptr] = p;
        ++path_stack_ptr;
        return 0;

ecwd:
        free(p);
estack:
        return -1;
}

int
eg_popd(void)
{
        int res;
        --path_stack_ptr;
        if (path_stack_ptr < 0) {
                /* XXX: would be caused by bug, need to set errno */
                return -1;
        }
        res = chdir(path_stack[path_stack_ptr]);
        free(path_stack[path_stack_ptr]);
        return res;
}
