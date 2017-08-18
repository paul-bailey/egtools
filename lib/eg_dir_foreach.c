#include "config.h"
#include "eg-devel.h"
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>

struct wrap_t {
        int (*fn)(FILE *, void *);
        const char *permissions;
        void *priv;
};

static int
wrap(const char *name, void *priv)
{
        struct wrap_t *w = (struct wrap_t *)priv;
        FILE *fp = fopen(name, w->permissions);
        if (!fp)
                return -1;
        return w->fn(fp, w->priv);
}

int
eg_dir_foreach(const char *path, unsigned int flags,
               int (*fn)(FILE *, void *), void *priv)
{
        struct wrap_t w;
        w.priv = priv;
        w.permissions = !!(flags & EG_BINARY) ? "rb" : "r";
        w.fn = fn;
        return eg_sdir_foreach(path, flags, wrap, &w);
}

int
eg_sdir_foreach(const char *path, unsigned int flags,
                int (*fn)(const char *, void *), void *priv)
{
        DIR *dir;
        struct dirent *ent;
        int ret = -1;

        dir = opendir(path);
        if (!dir)
                return -1;

        if (chdir(path))
                return -1;
        while ((ent = readdir(dir)) != NULL) {
                struct stat st;
                stat(ent->d_name, &st);
                if (!S_ISDIR(st.st_mode)) {
                        if (fn(ent->d_name, priv) < 0)
                                goto out;
                } else if (!!(flags & EG_RECURSIVE)) {
                        /* Ignore hidden and repetitive */
                        if (ent->d_name[0] == '.')
                                continue;
                        if (eg_sdir_foreach(ent->d_name, flags, fn, priv)
                            < 0) {
                                goto out;
                        }
                }
                /* else, continue */
        }
        ret = 0;

out:
        if (chdir("..") != 0)
                ret = -1;
        closedir(dir);
        return ret;
}
