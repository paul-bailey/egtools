#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>

int
main(int argc, char **argv)
{
        time_t t;
        struct tm *tms;
        char buf[100];
        char *endptr;

        if (argc == 1) {
                fprintf(stderr, "Expected: time stamp\n");
                return 1;
        }
        t = (time_t)strtoull(argv[1], &endptr, 0);
        if (errno || endptr == argv[1]) {
                fprintf(stderr, "Invalid time stamp '%s'\n", argv[1]);
                return 1;
        }
        tms = localtime(&t);
        if (tms == NULL) {
                perror("localtime() fail");
                return 1;
        }
        strftime(buf, sizeof(buf), "%m/%d/%Y %T", tms);
        buf[sizeof(buf) - 1] = '\0';
        printf("%s\n", buf);
        return 0;
}
