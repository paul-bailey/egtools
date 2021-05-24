#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static void
die(int signo)
{
        fprintf(stderr, "Trapped signal %d\n", signo);
        _exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
        FILE *fp;
        struct json_t *j;

        if (argc <= 1) {
                fprintf(stderr, "Expected: json file");
                return 1;
        }
        signal(SIGTERM, die);
        signal(SIGQUIT, die);
        signal(SIGSEGV, die);

        fp = fopen(argv[1], "r");
        j = json_parse(fp);
        fclose(fp);

        if (j != NULL) {
                printf("Without EOL:\n");
                json_print(stdout, j, false);

                /* because json_print() should not have when eol is false */
                putchar('\n');

                printf("With EOL:\n");
                json_print(stdout, j, true);
                json_free(j);
        }

        return 0;
}

