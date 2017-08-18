#ifndef EGXML_H
#define EGXML_H

#include <eg-devel.h>

#ifdef __cplusplus
extern "c" {
#endif

#include <stdio.h>

#define XML_NL         (0x0010U)
#define XML_ENTITY     (0x0020U)
#define XML_STRIP      (0x0040U)
#define XML_END        (0x0080U)
#define XML_START      (0x0100U)

typedef struct xml_tag_t XmlTag;
typedef struct xml_attr_t XmlAttribute;

/*
 *   Output
 */
struct xml_runner_t {
        XmlTag *tag;
        void *priv;
        unsigned int flags;
        int (*cb)(FILE *fp, int state, void *priv);
};

extern XmlTag *xml_tag_create(const char *name);
extern int xml_add_attribute(XmlTag *tag,
                              const char *name, const char *value);
extern int xml_strprints(String *str, const char *s);
extern void xml_fprints(FILE *fp, const char *s);
extern int xml_tag_recursive(FILE *fp, int state,
                             struct xml_runner_t *runner);

extern void xml_tag_free(XmlTag *tag);

/*
 *   Input (sorry, encoding = ASCII only)
 */
struct xml_prologue_t {
        float version;
        char encoding[20];
};

struct xml_attr_parser {
        const char *name;
        int (*parse)(void *priv, const char *val);
};

struct xml_elem_parser {
        const char *name;
        int (*parse)(FILE *fp, void *priv, XmlTag *tag);
};
extern int xml_get_prologue(FILE *fp, struct xml_prologue_t *prol);
extern XmlTag *xml_tag_parse(FILE *fp);
extern const char *xml_tag_name(XmlTag *tag);
extern unsigned int xml_tag_flags(XmlTag *tag);
extern XmlAttribute *xml_tag_attribute(XmlTag *tag, XmlAttribute *last);
extern const char *xml_attribute_name(XmlAttribute *attr);
extern const char *xml_attribute_value(XmlAttribute *attr);
#define xml_foreach_attr(iter_, tag_) \
        for (iter_ = xml_tag_attribute(tag_, NULL); \
             iter_ != NULL; \
             iter_ = xml_tag_attribute(tag_, iter_))
extern int xml_parse_attributes(void *priv,
                                const struct xml_attr_parser *tbl,
                                XmlTag *tag);
extern int xml_parse_elements(FILE *fp, void *priv,
                              const struct xml_elem_parser *tbl,
                              const char *term);
extern char *xml_elem_get_text(FILE *fp, unsigned int flags);

extern int xml_str2ent(const char *s);
extern const char *xml_ent2str(int c);

#ifdef __cplusplus
}
#endif

#endif /* EGXML_H */
