#include "json.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

struct jstate_t {
        FILE *fp;
        int lineno;
        jmp_buf env;
        union json_value_t cur_token;
};

#define syntax(state_, msg, ...) do { \
        fprintf(stderr, "json_parse near line %d: " msg "\n", \
                (state_)->lineno, ##__VA_ARGS__); \
        longjmp((state_)->env, 1); \
} while (0)

static int
check_remaining_string(FILE *fp, const char *s)
{
        int c, ref;
        while ((ref = *s++) != '\0') {
                if ((c = getc(fp)) != ref)
                        return -1;
        }
        return 0;
}

static void
copy_string(struct jstate_t *state, int delim)
{
        char *end, *s = NULL;
        size_t n = 0;
        ssize_t res = getdelim(&s, &n, delim, state->fp);
        if (res <= 0)
                syntax(state, "Expected: closing delimiter");

        /* Don't include the delimiter */
        end = s + strlen(s) - 1;
        if (end > s && *end == delim)
                *end = '\0';

        state->cur_token.s = s;
}

static int
get_number(struct jstate_t *state, int c)
{
        bool isflt = false;
        char buf[64];
        char *end = &buf[sizeof(buf)-1];
        char *s = buf;
        *s++ = c;
        while ((c = getc(state->fp)) != EOF) {
                if (c == '\0')
                        break;
                if (strchr(".E", toupper(c)) != NULL)
                        isflt = true;
                else if (!isdigit(c) && c != '-' && c != '+')
                        break;

                if (s < end)
                        *s++ = c;
                else
                        syntax(state, "Numeric expression too long");
        }
        if (c != EOF)
                ungetc(c, state->fp);
        *s++ = '\0';

        end = buf;
        if (isflt)
                state->cur_token.f = strtod(buf, &end);
        else
                state->cur_token.i = strtoll(buf, &end, 0);

        if (end == buf || *end != '\0')
                syntax(state, "Cannot evaluate numeric expression");

        return isflt ? 'f' : 'i';
}

/* returns one of "{}[]:,qbfi" or EOF */
static int
get_tok(struct jstate_t *state)
{
        int c;
        bool comment = false;
        FILE *fp = state->fp;

        /* slide to token */
        do {
                while ((c = getc(fp)) != EOF) {
                        if (c == '\n') {
                                state->lineno++;
                                if (comment) {
                                        comment = false;
                                        continue;
                                }
                        }
                        if (!comment && !isspace(c))
                                break;
                }

                if (c == EOF)
                        return c;
                else if (c == '#')
                        comment = true;
        } while (comment || c == '\0' || !isprint(c));

        if (strchr("{}[],:", c) != NULL) {
                return c;
        } else if (c == '"') {
                copy_string(state, c);
                return 'q';
        } else if (isalpha(c)) {
                if (toupper(c) == 'T') {
                        if (check_remaining_string(fp, "rue") < 0)
                                goto err;
                        state->cur_token.b = true;
                } else if (toupper(c) == 'F') {
                        if (check_remaining_string(fp, "alse") < 0)
                                goto err;
                        state->cur_token.b = false;
                } else {
                        goto err;
                }
                return 'b';
        } else if (isdigit(c) || strchr(".-+", c)) {
                return get_number(state, c);
        }

err:
        syntax(state, "Unknown token type");
        return EOF;
}

static struct json_t *
new_json(struct json_t *parent)
{
        struct json_t *ret = malloc(sizeof(*ret));

        memset(ret, 0, sizeof(*ret));
        ret->parent = parent;
        if (parent) {
                struct json_t *sibs = parent->children;
                if (sibs != NULL) {
                        /* Keep children in order of creation */
                        while (sibs->sib_next != NULL) {
                                sibs = sibs->sib_next;
                        }
                        sibs->sib_next = ret;
                        ret->sib_prev = sibs;
                        ret->sib_next = NULL;
                } else {
                        parent->children = ret;
                        ret->sib_prev = NULL;
                        ret->sib_next = NULL;
                }
        }
        return ret;
}

static void
json_free_memb(struct json_t *memb)
{
        struct json_t *child;

        /* Don't leave orphans lying around */
        for (child = memb->children;
             child != NULL; child = child->sib_next) {
                json_free_memb(child);
        }

        /* Untangled its linked lists */
        if (memb->parent && memb->parent->children == memb)
                memb->parent->children = memb->sib_next;
        if (memb->sib_prev)
                memb->sib_prev->sib_next = memb->sib_next;
        if (memb->sib_next)
                memb->sib_next->sib_prev = memb->sib_prev;

        if (memb->type == 'q' && memb->value.s != NULL)
                free(memb->value.s);

        free(memb);
}

static void parse_array(struct jstate_t *state, struct json_t *parent);
static void parse_dict(struct jstate_t *state, struct json_t *parent);
static void parse_atom(struct jstate_t *state, struct json_t *parent);

static void
parse_field_value(struct jstate_t *state, struct json_t *j)
{
        int tok = get_tok(state);
        j->type = tok;
        switch (tok) {
        case '{':
                parse_dict(state, j);
                break;
        case '[':
                parse_array(state, j);
                break;
        case 'q':
                /* Hand over pointer... easier than strdup+free */
                j->value.s = state->cur_token.s;
                break;
        case 'b':
                j->value.b = state->cur_token.b;
                break;
        case 'f':
                j->value.f = state->cur_token.f;
                break;
        case 'i':
                j->value.i = state->cur_token.i;
                break;
        default:
                json_free_memb(j);
                syntax(state, "Unexpected token");
        }
}

/* Helper for parse_array and parse_dict */
static void
check_endtok(struct jstate_t *state, int tok, int c)
{
        if (tok != c) {
                syntax(state, "Expected: `%c' at end of %s",
                        c, c == ']' ? "array" : "dict");
        }
}

static void
parse_array(struct jstate_t *state, struct json_t *parent)
{
        int tok;
        /*
         * Note: Don't get and use pointer to parent here,
         * because it could change location every time a
         * new json is allocated, due to realloc().
         * Hence the inline i2j(state, parent) calls below.
         */
        do {
                /* for debugging later */
                char namebuf[128];
                struct json_t *child = new_json(parent);
                sprintf(namebuf, "(%d)", parent->array_size);
                child->name = strdup(namebuf);
                parse_field_value(state, child);
                parent->array_size++;
        } while ((tok = get_tok(state)) == ',');
        check_endtok(state, tok, ']');
}

static void
parse_dict(struct jstate_t *state, struct json_t *parent)
{
        int tok;
        do {
                parse_atom(state, parent);
                parent->array_size++;
        } while ((tok = get_tok(state)) == ',');
        check_endtok(state, tok, '}');
}

static void
parse_atom(struct jstate_t *state, struct json_t *parent)
{
        int tok;
        struct json_t *child = new_json(parent);
        if ((tok = get_tok(state)) != 'q')
                syntax(state, "Expected: quoted string");
        /* Hand over pointer ... better than strdup + free */
        child->name = state->cur_token.s;
        if ((tok = get_tok(state)) != ':')
                syntax(state, "Expected: `:' but got %c", tok);

        parse_field_value(state, child);
}

/**
 * json_parse - Parse a file and create a JSON tree.
 * @fp: JSON file to parse
 * @info: Pointer to data containing some additional info about
 *      the tree.  If this is NULL, then json_append_from_file()
 *      and json_create_child() may not be used.
 *
 * Return: Handle to the top level of the JSON tree.
 *
 * Note: This is called a "tree" for egg-headed reasons only.
 * For swizzling purposes (some calling code is copying
 * this into SHM), the actual data structure is an array.
 */
struct json_t *
json_parse(FILE *fp)
{
        struct jstate_t state;
        struct json_t *top = NULL;

        state.fp = fp;
        state.lineno = 1;

        if (setjmp(state.env) != 0) {
                if (top != NULL)
                        json_free(top);
                return NULL;
        }
        top = new_json(NULL);
        parse_field_value(&state, top);
        return top;
}

/**
 * Parse @fp, whose contents are a descendant of @parent_node
 * @parent_node: Node in a JSON tree to append new json data to
 * @fp: File to parse
 * @info: JSON info filled in by original call to json_parse().
 *      This may not be NULL.
 *
 * Return: 0 if success, -1 if fail.  If return value is -1,
 * the JSON tree may be in an undefined state, and should be
 * freed.  Absurdly cautious people may wish to memcpy everything
 * before calling this funcion.
 *
 * FIXME: Sort of confusing how the hierarchy works here.
 */
int
json_append_from_file(struct json_t *parent_node, FILE *fp)
{
        int tok;
        struct jstate_t state;

        if (!parent_node)
                return -1;

        state.fp        = fp;
        state.lineno    = 1;

        if (setjmp(state.env) != 0)
                return -1;

        tok = get_tok(&state);
        if (tok != '{')
                syntax(&state, "Expected: { at start of file");
        parse_dict(&state, parent_node);
        return 0;
}

/**
 * json_create_child - Append a new child to a parent node
 * @parent_node: Parent node to append a child to
 *
 * Return: Handle to the child node.
 *
 * This is a simple alternative to json_append_from_file, where
 * you need to append just one or two new nodes.
 */
struct json_t *
json_create_child(struct json_t *parent_node)
{
        if (!parent_node)
                return NULL;

        return new_json(parent_node);
}

/* Un-syntax()-able code */

void
json_free(struct json_t *tbl)
{
        if (tbl != NULL)
                json_free_memb(tbl);
}

struct json_t *
json_parent(struct json_t *child)
{
        return child->parent;
}

/**
 * This returns first result whose name doesn't start with "//".
 * For strict JSON, just use parent->child
 */
struct json_t *
json_first_child(struct json_t *parent)
{
        struct json_t *child = parent->children;
        while (child != NULL && child->name[0] == '/'
               && child->name[1] == '/') {
                child = child->sib_next;
        }
        return child;
}

/**
 * This returns first result whose name doesn't start with "//".
 * For strict JSON, just use parent->child
 */
struct json_t *
json_next_child(struct json_t *sibling)
{
        struct json_t *ret = sibling;
        do {
                ret = ret->sib_next;
        } while (ret != NULL
                 && ret->name[0] == '/' && ret->name[1] == '/');
        return ret;
}

struct json_t *
json_find_child(struct json_t *parent, const char *name, bool case_sensitive)
{
        struct json_t *child;

        if (*name == '[') {
                /*
                 * Numerical array.
                 * Just count children rather than strcmp()ing
                 * all the ASCII numbers with name.
                 */
                char *endptr;
                int idx, count;
                ++name;

                /* Don't try to dereference a non-numerical array */
                if (parent->type != '[')
                        return NULL;

                /*
                 * Don't get cute with Python-like negative indices.
                 * Only dereference with positive index numbers.
                 */
                idx = strtoul(name, &endptr, 0);
                if (endptr == name || endptr[0] != ']' || endptr[1] != '\0')
                        return NULL;

                count = 0;
                JSON_FOR_EACH_CHILD(parent, child) {
                        if (count == idx)
                                return child;
                        count++;
                }
        } else {
                /* Associative array. */
                int (*cmp)(const char *, const char *);

                if (parent->type != '{')
                        return NULL;

                cmp = case_sensitive ? strcmp : strcasecmp;

                JSON_FOR_EACH_CHILD(parent, child) {
                        if (!cmp(child->name, name))
                                return child;
                }
        }
        return NULL;
}

/*
 * Like json_find_child, but namelen determines end of
 * string rather than the nulchar.
 * Extracted from json_find_descendant to explicitly
 * avoid putting a bunch of buf[]'s on the stack, since it
 * is a recursive function.
 */
static struct json_t *
json_find_child_nonul(struct json_t *parent, const char *name,
                      bool case_sensitive, size_t namelen)
{
        char buf[512];

        /* Waaay too long member name */
        if (namelen >= sizeof(buf))
                return NULL;

        strncpy(buf, name, namelen);
        buf[namelen] = '\0';

        return json_find_child(parent, buf, case_sensitive);
}

struct json_t *
json_find_descendant(struct json_t *parent, const char *name,
                     bool case_sensitive, int delim)
{
        const char *end;
        size_t len;
        struct json_t *child;

        /*
         * Square brackets in ``name" are reserved for numerical arrays.
         *
         * In a perfect world, delim is a period `.',
         * allowing ``name" to be something that looks like javascript.
         * In our world, delim is probably a hyphen `-'.
         */
        if (delim == '[')
                return NULL;

        for (end = name; *end != '\0'; end++) {
                if (*end == delim
                    || (end != name && *end == '[')) {
                        break;
                }
        }

        /* We have reached the end of our recursion */
        if (*end == '\0')
                return json_find_child(parent, name, case_sensitive);
        len = end - name;

        /* Move past delimiter, if not [#] */
        if (*end == delim)
                ++end;

        child = json_find_child_nonul(parent, name, case_sensitive, len);
        if (!child)
                return NULL;

        return json_find_descendant(child, end, case_sensitive, delim);
}

int
json_count_children(struct json_t *memb)
{
        int count = 0;
        struct json_t *child;

        /*
         * XXX: If we don't skipp '//' entries,
         * then this could be reduced to just
         *
         *      return memb->array_size;
         */

        JSON_FOR_EACH_CHILD(memb, child)
                ++count;
        return count;
}

static void
prnlsp(FILE *fp, int depth, bool eol)
{
        if (!eol)
                return;

        putc('\n', fp);
        while (depth-- > 0) {
                putc(' ', fp);
                putc(' ', fp);
        }
}

static void
json_print_r(FILE *fp, struct json_t *j, int depth, bool eol)
{
        prnlsp(fp, depth, eol);

        /* If we're not an array member, print name */
        if (j->parent != NULL && j->parent->type != '[')
                fprintf(fp, "\"%s\": ", j->name);

        if (j->type == '[' || j->type == '{') {
                struct json_t *child;
                int nchild;

                putc(j->type, fp);

                nchild = 0;
                JSON_FOR_EACH_CHILD_STRICT(j, child) {
                        if (nchild > 0)
                                putc(',', fp);
                        json_print_r(fp, child, depth + 1, eol);
                        ++nchild;
                }

                prnlsp(fp, depth, eol);
                putc(j->type + 2, fp);
        } else {
                switch (j->type) {
                case 'q':
                        fprintf(fp, "\"%s\"", j->value.s);
                        break;
                case 'f':
                        fprintf(fp, "%.15e", j->value.f);
                        break;
                case 'i':
                        fprintf(fp, "%lld", j->value.i);
                        break;
                case 'b':
                        fprintf(fp, "%s", j->value.b ? "true" : "false");
                        break;
                default:
                        /* TODO: BUG */
                        break;
                }
        }
}

void
json_print(FILE *fp, struct json_t *tbl, bool eol)
{
        json_print_r(fp, tbl, 0, eol);
        if (eol)
                putc('\n', fp);
}


