/* Work on this. */
#include <stdio.h>
#include <stdlib.h>

typedef struct Tree {
        char t_info;
        struct Tree *t_left;
        struct Tree *t_right;
} Tree;

Tree *root;

#if 0
Tree *dtree(Tree *root, char key)
{
        Tree *p, *q;
        if (root == NULL)
                return root;
        if (root->t_info == key) {
                if (root->t_left == root->t_right) {
                        free(root); // empty tree
                        return NULL;
                } else if (root->t_left == NULL) {
                        p = root->t_right;
                        free(root);
                        return p;
                } else if (root->t_right == NULL) {
                        p = root->t_left;
                        free(root);
                        return p;
                } else {
                        p2 = root->t_right;
                        p = root->t_right;
                        while (p->t_left)
                                p = p->t_left;
                        p->t_left = root->t_left;
                        free(root);
                        return p2;
                }
        } else if (root->info < key) {
                root->right = dtree(root->right, key);
        } else {
                root->left = dtree(root->left, key);
        }
        return root;
}
// use above as: root = dtree(root, key);
#endif

static void
printtree(Tree *r, int l)
{
        int i;
        if (!r)
                return;
        printtree(r->t_right, l + 1);
        for (i = 0; i < l; ++i)
                printf("   ");

        printf("%c\n", r->t_info);
        printtree(r->t_left, l + 1);
}

#if 0
Tree *balance1(Tree *parent, Tree *child)
{
        printf("Balancing one pair\n");
        if (child == NULL) {
                printf("Returning NULL\n");
                return parent;
        }
        Tree *grand = child->t_right;
        if (!grand) {
                printf("No granchild\n");
                return parent;
        }

        printf("balancing %c and %c\n", parent->t_info, child->t_info);
        if (child->t_info < grand->t_info && child->t_info > parent->t_info) {
                child->t_left = parent;
                parent->t_right = NULL;
                return child;
        }
        else
                return parent;
}

void balance(void)
{
        Tree *p, *q;

        printf("Gonna balance\n");
        if (root == NULL) {
                printf("Null root\n");
                return;
        }
        printf("root ain't null\n");
        p = root;
        for (;;) {
                q = balance1(p, p->t_right);
                if (q == p)
                        break;
                root = q;
                p = p->t_right;
        }
}
#else
#define balance(...) do { (void)0; } while (0)
#endif

static Tree *
stree(Tree *root, Tree *r, char info)
{
        if (r == NULL) {
                r = malloc(sizeof(Tree));
                if (r == NULL) {
                        fprintf(stderr, "Out of memory\n");
                        exit(0);
                }
                r->t_left = NULL;
                r->t_right = NULL;
                r->t_info = info;
                if (root == NULL) {
                        /* First entry */
                        return r;
                }
                if (info < root->t_info)
                        root->t_left = r;
                else
                        root->t_right = r;
                return r;
        }
        if (info < r->t_info)
                stree(r, r->t_left, info);
        else
                stree(r, r->t_right, info);

        return NULL;
}

int main(void)
{
        char c;
        char s[512];
        char *ps;
        root = NULL;
        printf("Enter a bunch of letters: ");
        ps = fgets(s, sizeof(s) - 1, stdin);
        for (ps = s; *ps != '\0'; ++ps) {
                if (!root)
                        root = stree(root, root, *ps);
                else
                        stree(root, root, *ps);
        }
        balance();

        printtree(root, 0);
        return 0;
}
