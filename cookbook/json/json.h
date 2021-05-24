#ifndef JSON_H
#define JSON_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

union json_value_t {
        char *s;
        long long i;
        double f;
        bool b;
};

struct json_t {
        char *name;
        union json_value_t value;
        int type; /* one of `qbfi[{' */
        int array_size;

        /* Array indices */
        struct json_t *parent;
        struct json_t *sib_next;
        struct json_t *sib_prev;
        struct json_t *children;
};

struct json_state_t {
        size_t size;
        size_t alloc_size;
};

extern void json_print(FILE *fp, struct json_t *j, bool eol);
extern void json_free(struct json_t *j);
extern struct json_t *json_parse(FILE *fp);
extern int json_append_from_file(struct json_t *parent_node, FILE *fp);
extern struct json_t *json_create_child(struct json_t *parent_node);
extern struct json_t *json_find_child(struct json_t *parent,
                              const char *name, bool case_sensitive);

extern struct json_t *json_first_child(struct json_t *parent);
extern struct json_t *json_next_child(struct json_t *sibling);
extern struct json_t *json_parent(struct json_t *memb);
extern int json_idx(struct json_t *memb);
extern struct json_t *json_find_descendant(struct json_t *parent,
                       const char *name, bool case_sensitive, int delim);

#define JSON_FOR_EACH_CHILD_STRICT(Parent_, Child_) \
        for (Child_ = Parent_->children; Child_ != NULL; \
             Child_ = Child_->sib_next)
#define JSON_FOR_EACH_CHILD(Parent_, Child_) \
        for (Child_ = json_first_child(Parent_); \
             Child_ != NULL; \
             Child_ = json_next_child(Child_))

#endif /* JSON_H */
