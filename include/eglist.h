/*
 * eglist.h -- Typical linked list stuff
 *
 * Mostly immitated from the linux version (See include/linux/list.h)
 * except that it is a little more self contained and is a little more
 * lightweight.
 *
 * Since this is a derived work from Linux's list.h, this header falls
 * under GPLv2.
 */

#ifndef EGLIST_H
#define EGLIST_H

#ifdef __cplusplus
extern "c" {
#endif

/*
 * You can use this file almost completely independent with a Gnu
 * compiler - all you need to do is remove this include and replace
 * it with your own container_of() macro.
 */
#ifndef offsetof
# define offsetof(type, mem) ((size_t)&((type *)0)->mem)
#endif

#ifndef container_of
# define container_of(p, type, mem)                     \
  ({      const typeof(((type *)0)->mem) *p_ = (p);     \
          (type *)((char *)p_ - offsetof(type, mem));   \
  })
#endif


/**
 * struct list_head - General purpose doubly-linked list
 * @next: next list entry
 * @prev: previous list entry
 */
struct list_head {
        struct list_head *next;
        struct list_head *prev;
};

#define LIST_HEAD_INIT(name)    { .prev = &(name), .next = &(name) }

static inline void
init_list_head(struct list_head *list)
{
        list->next = list;
        list->prev = list;
}

static inline void list_add__(struct list_head *new,
                              struct list_head *prev,
                              struct list_head *next)
{
        next->prev = new;
        new->next = next;
        new->prev = prev;
        prev->next = new;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head after which to add @new
 *
 * Insert a @new after @head
 */
static inline void list_add(struct list_head *new, struct list_head *head)
{
        list_add__(new, head, head->next);
}

/**
 * list_add_tail - add a new entry
 * @new: New entry to be added
 * @head: list head before which to add @new
 *
 * Insert @new before @head
 */
static inline void list_add_tail(struct list_head *new,
                                 struct list_head *head)
{
        list_add__(new, head->prev, head);
}

static inline void list_del__(struct list_head *prev,
                              struct list_head *next)
{
        next->prev = prev;
        prev->next = next;
}

/**
 * list_del - delete entry from list.
 * @entry: the element to delete from the list
 */
static inline void list_del(struct list_head *entry)
{
        list_del__(entry->prev, entry->next);
}

/**
 * list_is_last - Test whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int list_is_last(const struct list_head *list,
                               const struct list_head *head)
{
        return list->next == head;
}

/**
 * list_is_empty - test whether a list is empty
 * @head: the list to test.
 */
static inline int list_is_empty(const struct list_head *head)
{
        return head->next == head;
}

/**
 * list_is_singular - tests whether a list has jsut one entry.
 * @head: the list to test
 */
static inline int list_is_singular(const struct list_head *head)
{
        return !list_is_empty(head) && (head->next == head->prev);
}

/**
 * list_entry - get the struct for this entry
 * @ptr:        the &struct list_head pointer.
 * @type:       the type of the struct this is embedded in.
 * @member:     the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
        container_of(ptr, type, member)

/*
 * Corrollaries of list_entry and helpers for list_for_each_entry* below
 */

/**
 * list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
        list_entry((ptr)->next, type, member)

/**
 * list_last_entry - get the last element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_last_entry(ptr, type, member) \
        list_entry((ptr)->prev, type, member)

#define list_next_entry(pos, member) \
        list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_prev_entry(pos, member) \
        list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
 * list_first_entry_or_null - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note that if the list is empty, it returns NULL.
 */
#define list_first_entry_or_null(ptr, type, member) \
	(!list_is_empty(ptr) \
         ? list_first_entry(ptr, type, member) : NULL)

/**
 * list_for_each - Iterate over a list
 * @pos:        the &struct list_head iterator
 * @head:       the head of the list
 */
#define list_for_each(pos, head) \
        for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_prev - Iterate over a list backward
 * @pos:        the &struct list_head iterator
 * @head:       the head of the list
 */
#define list_for_each_prev(pos, head) \
        for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * list_for_each_safe - Iterate over a list safe against removal of list
 *                      entry
 * @pos:        the &struct list_head iterator
 * @n:          another &struct list_head temporary pointer
 * @head:       the head of the list
 */
#define list_for_each_safe(pos, n, head) \
        for (pos = (head)->next, n = pos->next; pos != (head); \
             pos = n, n = pos->next)

/**
 * list_for_each_prev_safe - Iterate over a list backward safe against
 *                           removal of list entry
 * @pos:        The &struct list_head to use as a loop cursor.
 * @n:          Another &struct list_head to use as temporary storage
 * @head:       The head of the list
 */
#define list_for_each_prev_safe(pos, n, head) \
        for (pos = (head)->prev, n = pos->prev; pos != (head); \
             pos = n, n = pos->prev)

/**
 * list_for_each_entry - Iterate over a list of given type
 * @pos:        The type * to use as an iterator
 * @head:       The head of the list
 * @member:     The name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member)                   \
        for (pos = list_first_entry(head, typeof(*pos), member); \
             &pos->member != (head);                             \
             pos = list_next_entry(pos, member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given
 *                               type.
 * @pos:	the type * to use as an iterator.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry_reverse(pos, head, member)          \
        for (pos = list_last_entry(head, typeof(*pos), member); \
             &pos->member != (head);                            \
             pos = list_prev_entry(pos, member))

/**
 * list_for_each_entry_safe - Iterate over list of given type safe
 *                            against removal of list entry
 * @pos:        The type * to use as an iterator
 * @n:          Another type * to use as temporary storage
 * @head:       The head of the list
 * @member:     The name of the list_struct within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member)                  \
        for (pos = list_first_entry(head, typeof(*pos), member),        \
                n = list_next_entry(pos, member);                       \
             &pos->member != (head);                                    \
             pos = n, n = list_next_entry(n, member))

/**
 * list_for_each_entry_safe_reverse - iterate backwards over list safe
 *                                    against removal
 * @pos:	the type * to use as an iterator
 * @n:		another type * to use as temporary storage
 * @head:	the head for the list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = list_last_entry(head, typeof(*pos), member),		\
		n = list_prev_entry(pos, member);			\
	     &pos->member != (head); 					\
	     pos = n, n = list_prev_entry(n, member))


#ifdef __cplusplus
}
#endif

#endif /* EGLIST_H */
