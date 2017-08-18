#include <eghash.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <egdebug.h>
#include <fcntl.h>

#define METHOD eg_djb_hash
#define TABSIZ 1

static void
print(struct eghash_t *key, struct eghash_t *data)
{
        printf("%-2d: %s\n", *(int *)data->data, (char *)key->data);
}

static void
diag(HashTable *tbl)
{
        struct eghash_diag_t diag;
        eghash_diag(tbl, &diag);
        printf("hits: %lu\n", diag.hits);
        printf("misses: %lu\n", diag.misses);
        printf("size: %lu\n", diag.size);
        printf("collisions: %lu\n", diag.collisions);
        printf("maxcollisions: %lu\n", diag.maxcollisions);
}

int
main(void)
{
        const char *months[] = {
                "January", "February", "March",
                "April", "May", "June",
                "July", "August", "September",
                "October", "November", "December", };
        int i, res;
        time_t t;
        char name[100];
        HashTable *tbl;
        struct eghash_t key, data;
        eghash_iter_t state;
        FILE *fp;
        int fd;

        t = time(NULL);
        sprintf(name, "tmp_%lu", (unsigned long)t);

        tbl = eghash_create(TABSIZ, METHOD);
        assert(tbl != NULL);


        for (i = 0; i < 12; i++) {
                int j = i + 1; /* because that's how we number our months */
                key.data = (void *)months[i];
                key.size = strlen(months[i]) + 1;
                /* This should be safe if eghash truly copies it */
                data.data = &j;
                data.size = sizeof(j);
                res = eghash_put(tbl, &key, &data);
                assert(res == 0);
        }
        for (i = 0; i < 12; i++) {
                key.data = (void *)months[i];
                key.size = strlen(months[i]) + 1;
                res = eghash_get(tbl, &key, &data);
                assert(res == 0);
                print(&key, &data);
        }
        printf("\tNow testing sync-ing\n");

        /* Since open() varies between Darwin and Gnu/Linux */
        fp = fopen(name, "wb");
        assert(fp != NULL);
        fd = fileno(fp);
        res = eghash_sync(tbl, fd);
        assert(res == 0);
        fclose(fp);

        eghash_destroy(tbl);

        fd = open(name, O_RDONLY);
        assert(fd >= 0);
        tbl = eghash_open(fd, METHOD);
        assert(tbl != NULL);
        close(fd);
        for (i = 0; i < 12; i++) {
                key.data = (void *)months[i];
                key.size = strlen(months[i]) + 1;
                res = eghash_get(tbl, &key, &data);
                assert(res == 0);
                print(&key, &data);
        }
        diag(tbl);

        printf("\tNow testing eghash_iterate()\n");
        memset(&state, 0, sizeof(state));
        while ((res = eghash_iterate(tbl, &state, &key, &data)) != -1) {
                print(&key, &data);
        }
        eghash_destroy(tbl);
        unlink(name);
        return 0;
}
