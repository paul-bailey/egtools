#include <egxml.h>
#include <eglist.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAXENTITYSIZE 40
#define XMLNAMESIZE 128


/* **********************************************************************
 *              Endpoints
 ***********************************************************************/

struct xml_attr_t {
        String *a_name;
        String *a_value;
        struct list_head a_list;
};

struct xml_tag_t {
        String *t_name;
        unsigned int t_flags;
        struct list_head t_attr;
};

#define xml_foreach_attr_safe(iter_, q_, tag_) \
        list_for_each_entry_safe(iter_, q_, &(tag_)->t_attr, a_list)

static XmlAttribute *
new_attribute(void)
{
        XmlAttribute *ret = malloc(sizeof(*ret));
        if (!ret)
                return NULL;
        ret->a_name = NULL;
        ret->a_value = NULL;
        init_list_head(&ret->a_list);
        return ret;
}

static void
destroy_attribute(XmlAttribute *attr)
{
        list_del(&attr->a_list);
        if (attr->a_name != NULL)
                string_destroy(attr->a_name);
        if (attr->a_value != NULL)
                string_destroy(attr->a_value);
        free(attr);
}

static XmlTag *
xml_tag_new(void)
{
        XmlTag *tag = malloc(sizeof(*tag));
        if (!tag)
                return NULL;
        tag->t_name = NULL;
        tag->t_flags = 0;
        init_list_head(&tag->t_attr);
        return tag;
}

void
xml_tag_free(XmlTag *tag)
{
        XmlAttribute *attr, *q;
        xml_foreach_attr_safe(attr, q, tag) {
                destroy_attribute(attr);
        }
        string_destroy(tag->t_name);
        free(tag);
}


/* **********************************************************************
 *              Output functions
 ***********************************************************************/

XmlTag *
xml_tag_create(const char *name)
{
        XmlTag *tag = xml_tag_new();
        if (!tag)
                goto emalloc;
        tag->t_name = string_create(NULL);
        if (!tag->t_name)
                goto estring;
        if (string_append(tag->t_name, name) == EOF)
                goto eappend;
        return tag;

eappend:
        string_destroy(tag->t_name);
estring:
        free(tag);
emalloc:
        return NULL;
}

unsigned int
xml_tag_flags(XmlTag *tag)
{
        return tag->t_flags;
}

const char *
xml_tag_name(XmlTag *tag)
{
        return string_cstring(tag->t_name);
}

int
xml_add_attribute(XmlTag *tag, const char *name, const char *value)
{
        XmlAttribute *attr = new_attribute();
        if (!attr)
                goto eattr;
        attr->a_name = string_create(NULL);
        if (!attr->a_name)
                goto ename;
        attr->a_value = string_create(NULL);
        if (!attr->a_value)
                goto evalue;
        if (string_append(attr->a_name, name) == EOF)
                goto eappend;
        if (string_append(attr->a_value, value) == EOF)
                goto eappend;
        list_add(&attr->a_list, &tag->t_attr);
        return 0;

eappend:
        string_destroy(attr->a_value);
evalue:
        string_destroy(attr->a_name);
ename:
        free(attr);
eattr:
        return -1;
}

static void
print_indent(FILE *fp, int indent)
{
        while (indent > 0) {
                fprintf(fp, "  ");
                --indent;
        }
}

#define FLAG_TO_INDENT(v_) ((int)(((v_) >> 0) & 15))

int
xml_strprints(String *str, const char *s)
{
        int c;
        while ((c = *s++) != '\0') {
                const char *ent = xml_ent2str(c);
                if (ent != NULL) {
                        c = string_putc(str, '&');
                        if (c == EOF)
                                return EOF;
                        c = string_append(str, ent);
                        if (c == EOF)
                                return EOF;
                        c = string_putc(str, ';');
                        if (c == EOF)
                                return EOF;
                } else {
                        c = string_putc(str, c);
                        if (c == EOF)
                                return EOF;
                }
        }
        return 0;
}

void
xml_fprints(FILE *fp, const char *s)
{
        int c;
        while ((c = *s++) != '\0') {
                const char *ent = xml_ent2str(c);
                if (ent != NULL)
                        fprintf(fp, "&%s;", ent);
                else
                        fputc(c, fp);
        }
}

int
xml_tag_recursive(FILE *fp, int state, struct xml_runner_t *runner)
{
        XmlAttribute *attr;
        int indent = state;
        int ret = 0;

        print_indent(fp, indent);

        fprintf(fp, "<%s", string_cstring(runner->tag->t_name));
        xml_foreach_attr(attr, runner->tag) {
                fprintf(fp, " %s=\"", string_cstring(attr->a_name));
                xml_fprints(fp, string_cstring(attr->a_value));
                fprintf(fp, "\"");
        }

        if (runner->cb) {
                fprintf(fp, ">");

                if (!!(runner->flags & XML_NL))
                        fprintf(fp, "\n");

                ret = runner->cb(fp, state + 1, runner->priv);

                if (!!(runner->flags & XML_NL))
                        print_indent(fp, indent);

                fprintf(fp, "</%s>\n", string_cstring(runner->tag->t_name));
        } else {
                fprintf(fp, " />");

                if (!!(runner->flags & XML_NL))
                        fprintf(fp, "\n");
        }
        return ret;
}


/* **********************************************************************
 *              Input functions
 ***********************************************************************/

/* like eg_slide, but permit EOL */
static const char *
spaceslide(const char *s)
{
        while (isspace((int)(*s)))
                ++s;
        return s;
}

/* already have '&' */
static int
parse_entity(const char *s, const char **endptr)
{
        char buf[MAXENTITYSIZE];
        int c, i;

        i = 0;
        while ((c = *s++) != ';') {
                if (c == '\0' || i > (sizeof(buf) - 1))
                        return EOF;
                buf[i++] = c;
        }
        buf[i] = '\0';
        *endptr = s;
        return xml_str2ent(buf);
}

static int
fparse_entity(FILE *fp)
{
        char buf[MAXENTITYSIZE];
        int i = 0;
        int c;
        while ((c = getc(fp)) != ';') {
                if (c == EOF || i == (sizeof(buf) - 1))
                        return -1;
                buf[i++] = c;
        }
        buf[i] = '\0';
        return xml_str2ent(buf);
}

char *
xml_elem_get_text(FILE *fp, unsigned int flags)
{
        char *ret = NULL;
        String *str = string_create(NULL);
        int c;
        if (!str)
                return NULL;

        for (;;) {
                c = getc(fp);
                switch (c) {
                case '<':
                        ungetc(c, fp);
                        goto done;
                case EOF:
                        goto err;
                case '&':
                        c = fparse_entity(fp);
                        if (c == EOF)
                                goto err;
                        break;
                default:
                        break;
                }
                c = string_putc(str, c);
                if (c == EOF)
                        goto err;
        }
done:
        if (!!(flags & XML_STRIP))
                string_strip(str);
        ret = strdup(string_cstring(str));

err:
        string_destroy(str);
        return ret;
}

/*
 * XXX: name confusion with xml_parse_attributes, which is completely
 * different.
 */
static int
parse_attribute_string(XmlTag *tag, const char *s, const char **endptr)
{
        int c;
        int quot;
        XmlAttribute *attr = new_attribute();
        if (!attr)
                return -1;

        attr->a_name = string_create(attr->a_name);
        attr->a_value = string_create(attr->a_value);
        if (attr->a_name == NULL || attr->a_value == NULL)
                goto err;
        while ((c = *s) != '=' && !isspace(c)) {
                if (c == '\0')
                        goto err;
                c = string_putc(attr->a_name, c);
                if  (c == EOF)
                        goto err;
                ++s;
        }
        s = spaceslide(s);
        if ((c = *s++) != '=')
                goto err;
        s = spaceslide(s);
        quot = *s++;
        if (quot != '"' && quot != '\'')
                goto err;
        while ((c = *s) != quot) {
                if (c == '\0')
                        goto err;
                if (c == '&') {
                        ++s;
                        c = parse_entity(s, &s);
                        if (c == EOF)
                                goto err;
                        --s;
                }
                c = string_putc(attr->a_value, c);
                if (c == EOF)
                        goto err;
                ++s;
        }
        if (c != quot)
                goto err;
        *endptr = s + 1;
        list_add(&attr->a_list, &tag->t_attr);
        return 0;

err:
        destroy_attribute(attr);
        return -1;
}

static XmlTag *
parse_tag_string(const char *s)
{
        XmlTag *tag;
        int c;

        s = spaceslide(s);
        if (*s == '\0')
                return NULL;

        tag = xml_tag_new();
        if (!tag)
                return NULL;

        tag->t_name = string_create(tag->t_name);
        if (!tag->t_name)
                goto err;

        if (s[0] == '/') {
                tag->t_flags |= XML_END;
                ++s;
        } else {
                tag->t_flags |= XML_START;
        }

        while (!isspace(c = *s) && c != '\0') {
                c = string_putc(tag->t_name, c);
                if (c == EOF)
                        goto err;
                ++s;
        }

        for (;;) {
                s = spaceslide(s);

                if ((c = *s) == '\0') {
                        break;
                } else if (c == '/' && s[1] == '\0') {
                        if (!!(tag->t_flags & XML_END)) {
                                /* don't allow </xxx ... /> */
                                goto err;
                        }
                        tag->t_flags |= XML_END;
                        break;
                } else {
                        if (parse_attribute_string(tag, s, &s) != 0)
                                goto err;
                }
        }
        return tag;

err:
        xml_tag_free(tag);
        return NULL;
}

static String *
get_tag_string(FILE *fp, String *old)
{
        int c;
        String *ret = old == NULL ? string_create(NULL) : old;
        if (ret == NULL)
                return NULL;

        while (isspace(c = getc(fp)))
                ;
        if (c != '<')
                goto syntax;
        while ((c = getc(fp)) != '>' && c != EOF) {
                c = string_putc(ret, c);
                if (c == EOF)
                        goto syntax;
        }
        if (c != '>')
                goto syntax;
        return ret;

syntax:
        string_destroy(ret);
        return NULL;
}

XmlTag *
xml_tag_parse(FILE *fp)
{
        XmlTag *ret;
        String *str = get_tag_string(fp, NULL);
        if (str == NULL)
                return NULL;

        ret = parse_tag_string(string_cstring(str));
        string_destroy(str);
        return ret;
}

XmlAttribute *
xml_tag_attribute(XmlTag *tag, XmlAttribute *last)
{
        XmlAttribute *ret;
        if (!last) {
                ret = list_first_entry_or_null(&tag->t_attr,
                                               XmlAttribute, a_list);
        } else {
                ret = list_next_entry(last, a_list);
                if (&ret->a_list == &tag->t_attr)
                        ret = NULL;
        }
        return ret;
}

/* return -1 if error, 1 if no prologue exists, 0 if exists */
int
xml_get_prologue(FILE *fp, struct xml_prologue_t *prol)
{
        int count;
        long pos = ftell(fp);
        if (pos < 0)
                return -1;

        count = fscanf(fp, " <?xml version=\"%f\" encoding=\"%19s\" ",
                       &prol->version, prol->encoding);
        if (count != 2) {
                fseek(fp, pos, SEEK_SET);
                return 1;
        }
        return 0;
}

int
xml_parse_attributes(void *priv,
                     const struct xml_attr_parser *tbl,
                     XmlTag *tag)
{
        XmlAttribute *attr;
        const struct xml_attr_parser *t;
        xml_foreach_attr(attr, tag) {
                const char *val = string_cstring(attr->a_value);
                const char *name = string_cstring(attr->a_name);
                for (t = tbl; t->parse != NULL; t++) {
                        if (!strcmp(name, t->name)) {
                                if (t->parse(priv, val) != 0)
                                        return -1;
                                else
                                        break;
                        }
                }
                if (t->parse == NULL)
                        return -1;
        }
        return 0;
}

static int
xml_parse_root(FILE *fp, void *priv, const struct xml_elem_parser *tbl)
{
        XmlTag *tag;
        const char *tagname;
        int ret = -1;

        tag = xml_tag_parse(fp);
        if (!tag)
                return -1;
        tagname = string_cstring(tag->t_name);

        /* We shouldn't have "</xx..>" already */
        if ((tag->t_flags & (XML_START | XML_END)) == XML_END)
                goto err;

        if (strcmp(tbl->name, tagname))
                goto err;

        if (tbl->parse(fp, priv, tag) < 0)
                goto err;

        ret = 0;
err:
        xml_tag_free(tag);
        return ret;
}

int
xml_parse_elements(FILE *fp, void *priv,
                   const struct xml_elem_parser *tbl, const char *term)
{
        int end;
        if (term == NULL)
                return xml_parse_root(fp, priv, tbl);

        end = 0;
        do {
                XmlTag *tag;
                const char *tagname;
                const struct xml_elem_parser *t;

                tag = xml_tag_parse(fp);
                if (!tag)
                        return -1;

                tagname = string_cstring(tag->t_name);
                if ((tag->t_flags & (XML_START | XML_END)) == XML_END) {
                        /* Got "</xx..>", may only be our own */
                        end = (strcmp(term, tagname) == 0) ? 1 : -1;
                }

                /* else, got <xx..> or <xx.. />, a child element */

                if (end == 0) {
                        for (t = tbl; t->parse != NULL; ++t) {
                                if (!strcmp(t->name, tagname)) {
                                        if (t->parse(fp, priv, tag) < 0)
                                                end = -1;
                                        break;
                                }
                        }
                        if (t->parse == NULL)
                                end = -1;
                }
                xml_tag_free(tag);
        } while (end == 0);
        return end >= 0 ? 0 : -1;
}
