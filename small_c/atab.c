#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#define NCOL 4
#define NROW (128/NCOL)

static const struct ctrldesc_t {
        const char *name;
        const char *desc;
} CTRLDESC[32] = {
        { .name = "NUL", .desc = "null" },
        { .name = "SOH", .desc = "start of heading" },
        { .name = "STX", .desc = "start of text" },
        { .name = "ETX", .desc = "end of text" },
        { .name = "EOT", .desc = "end of transmission" },
        { .name = "ENQ", .desc = "enquiry" },
        { .name = "ACK", .desc = "acknowledge" },
        { .name = "BEL", .desc = "bell" },
        { .name = "BS",  .desc = "backspace" },
        { .name = "TAB", .desc = "horizontal tab" },
        { .name = "LF",  .desc = "\\n" },
        { .name = "VT",  .desc = "vertical tab" },
        { .name = "FF",  .desc = "\\f, new page" },
        { .name = "CR",  .desc = "\\r" },
        { .name = "SO",  .desc = "shift out" },
        { .name = "SI",  .desc = "shift in" },
        { .name = "DLE", .desc = "data link escape" },
        { .name = "DC1", .desc = "device control 1" },
        { .name = "DC2", .desc = "device control 2" },
        { .name = "DC3", .desc = "device control 3" },
        { .name = "DC4", .desc = "device control 4" },
        { .name = "NAK", .desc = "negative acknowledge" },
        { .name = "SYN", .desc = "synchronous idle" },
        { .name = "ETB", .desc = "end of trans. block" },
        { .name = "CAN", .desc = "cancel" },
        { .name = "EM",  .desc = "end of medium" },
        { .name = "SUB", .desc = "substitute" },
        { .name = "ESC", .desc = "escape" },
        { .name = "FS",  .desc = "file separator" },
        { .name = "GS",  .desc = "group separator" },
        { .name = "RS",  .desc = "record separator" },
        { .name = "US",  .desc = "unit separator" },
};

int
main(int argc, char **argv)
{
        int verbose = 0;
        int row;
        int opt;

        opt = getopt(argc, argv, "v");
        if (opt == 'v') {
                verbose = 1;
        } else if (opt != -1) {
                fprintf(stderr, "%s: Unrecognized option '%c'\n",
                        argv[0], opt);
                return 1;
        }

        if (verbose) {
                printf("Dec Hx Oct Chr                        | Dec Hx Oct Html Chr | Dec Hx Oct Html Chr | Dec Hx Oct Html Chr\n");
                printf("-------------------------------------------------------------------------------------------------------\n");
        } else {
                printf("Dec Hx Oct Chr | Dec Hx Oct Html Chr | Dec Hx Oct Html Chr | Dec Hx Oct Html Chr\n");
                printf("--------------------------------------------------------------------------------\n");
        }

        for (row = 0; row < NROW; ++row) {
                /* col 0 */
                int c, col;
                char buf[40];

                c = row;
                printf("%3d %2X %03o %-3s", c, c, c, CTRLDESC[row].name);
                if (verbose) {
                        sprintf(buf, "(%s)", CTRLDESC[row].desc);
                        printf(" %-22s", buf);
                }

                /* the other cols */
                for (col = 1; col < NCOL; col++) {
                        c = col * NROW + row;
                        sprintf(buf, "&#%d;", c);
                        printf(" | %3d %2X %03o %-6s ", c, c, c, buf);
                        if (c != 127) {
                                putchar(c);
                        } else {
                                printf("DEL");
                        }
                }
                putchar('\n');
        }
        return 0;

}
