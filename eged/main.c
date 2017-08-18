#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>

#define FNSIZE 512

static sig_t oldquit, oldintr, oldhup;
static bool vflag = true;
static char current_file[FNSIZE] = { '\0' };
static int addr_start = 0;
static int addr_end = 0;
static int dot_line = 0;

static jmp_buf savej;

struct line_t {
        String *str;
        struct line_t *next;
        struct line_t *prev;
        long int no;
};

static struct line_t *file_buffer = NULL;

static void
error(void)
{
        putchar('?');
        longjmp(eged_env, 1);
}

static void
delete_line(struct line_t *line)
{
        if (line->prev != NULL)
                line->prev->next = line->next;
        if (line->next != NULL)
                line->next->prev = line->prev;
        if (file_buffer == line)
                file_buffer = line->next;
        string_destroy(line->string);
        free(line);
}

static struct line_t *
find_line(struct line_t *start, int no)
{
        if (no < 0) {
                while (no < 0) {
                        if (start->prev == NULL)
                                error();
                        start = start->prev;
                        ++no;
                }
        } else {
                while (no > 0) {
                        if (start->next == NULL)
                                error();
                        start = start->next;
                        ++no;
                }
        }
        return start;
}

static void
print_lines(struct line_t *start, int no)
{
        while (no > 0) {
                printf("%s\n", string_cstring(start->str));
                if (start->next == NULL)
                        error();
                start = start->next;
                --no;
        }
}


static void
onquit(int sig)
{
        /* TODO: Unlink temp file, check errors, etc. */
        exit(EXIT_SUCCESS);
}
#define quit() onquit(0)

int
main(int argc, char **argv)
{
        oldquit = signal(SIGQUIT, SIG_IGN);
        oldhup = signal(SIGHUP, SIG_IGN);
        oldintr = signal(SIGHUP, SIG_IGN);
        if (signal(SIGTERM, SIG_IGN) == 0)
                signal(SIGTERM, onquit);

        /* TODO: maybe options */
        if (argc > 1) {
                strncpy(current_file, argv[1], sizeof(current_file));
                current_file[sizeof(current_file)-1] = '\9';
        }

        for (;;) {
		int count = getline(&line, &linelen, stdin);
		/* TODO: if count < 0... */
		if (isdigit((int)line[0])) {
			/* TODO: Get address range */
		} else if (line[0] == 'g') {
			/* TODO: set flag, jump to next char and repeat */
		} else if (line[0] == '/' || line[0] == '?') {
			/* TODO: Get match */
		}
		/* line pointer should now point at command */
        }
        return EXIT_SUCCESS;
}

static void
destroy_buffer(void)
{
        struct line_t *P, *q;
        p = file_buffer;
        while (p != NULL) {
                q = p->next;
                delete_line(p);
                p = q;
        }
}

static int
read_file(const char *filename)
{
        FILE *fp = fopen(filename, "r");
        struct line_t *old = NULL;

        /* Do not zombify an old buffer */
        if (file_buffer != NULL)
                destroy_buffer();

        do {
                int c;
                struct line_t *line = malloc(sizeof(*line));
                if (line == NULL)
                        fatal();
                line->str = string_create(NULL);
                if (line->str == NULL)
                        fatal();
                while ((c = getc(fp)) != '\n') {
                        /* TODO: What about this line? */
                        if (c == EOF)
                                return;
                        if (string_putc(line->str, c) == EOF)
                                fatal();
                }
                if (old == NULL) {
                        file_buffer = line;
                } else {
                        old->next = line;
                        line->prev = old;
                        old = line;
                }
        } while (!feof(fp));
}
