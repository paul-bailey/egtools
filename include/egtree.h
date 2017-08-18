/* NOT a balanced binary tree!  This is here for convenience, not speed */
#ifndef EGTREE_H
#define EGTREE_H

#include <eglist.h>

#ifdef __cplusplus
extern "C" {
#endif

struct egtree_t {
        struct egtree_t *parent;
        struct list_head siblings;
        struct list_head children;
};

static inline void
egtree_init(struct egtree_t *new, struct egtree_t *parent)
{
        new->parent = parent;
        if (parent)
                list_add(&new->siblings, &parent->children);
        else
                init_list_head(&new->siblings);
        init_list_head(&new->chidren);
}

/* in case "parent" was not yet available at "egtree_init" time */
static inline void
egtree_add_child(struct egtree_t *new, struct egtree_t *parent)
{
        new->parent = parent;
        list_add(&new->sibling, &parent->children);
}

/*
 * WARNING: *_before*() is not removal-safe. Best to do *_after*()
 * when deleting trees.
 */

static inline void
egtree_recurse_after(struct egtree_t *tree, void *priv,
                     void (*cb)(struct egtree_t *, void *))
{
        struct egtree_t *t, *v;
        list_for_each_entry_safe(t, v, &tree->children, siblings) {
                egtree_recurse_after(t, priv, cb);
        }
        cb(t, priv);
}

static inline void
egtree_recurse_before(struct egtree_t *tree, void *priv,
                      void (*cb)(struct egtree_t *, void *))
{
        struct egtree_t *t;
        cb(t, priv);
        list_for_each_entry(t, &tree->children, siblings) {
                egtree_recurse_before(t, priv, cb);
        }
}

static inline void
egtree_recurse_after_reverse(struct egtree_t *tree, void *priv,
                        void (*cb)(struct egtree_t *, void *))
{
        struct egtree_t *t, *v;
        list_for_each_entry_safe_reverse(t, v, &tree->children, siblings) {
                egtree_recurse_after(t, priv, cb);
        }
        cb(t, priv);
}

static inline void
egtree_recurse_before_reverse(struct egtree_t *tree, void *priv,
                        void (*cb)(struct egtree_t *, void *))
{
        struct egtree_t *t;
        cb(t, priv);
        list_for_each_entry_reverse(t, &tree->children, siblings) {
                egtree_recurse_after(t, priv, cb);
        }
}

#ifdef __cplusplus
}
#endif

#endif /* EGTREE_H */
