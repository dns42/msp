#ifndef CRT_LIST_H
#define CRT_LIST_H

#include <crt/defs.h>

struct list {
    struct list *prev;
    struct list *next;
};

#define LIST(_head) \
    (struct list) { .next = (_head), .prev = (_head) }

static inline void
list_init(struct list *elem)
{
    elem->prev = elem;
    elem->next = elem;
}

static inline void
__list_insert(struct list *prev, struct list *elem, struct list *next)
{
    prev->next = elem;
    elem->prev = prev;
    elem->next = next;
    next->prev = elem;
}

static inline void
list_insert_after(struct list *prev, struct list *elem)
{
    __list_insert(prev, elem, prev->next);
}

static inline void
list_insert_before(struct list *next, struct list *elem)
{
    __list_insert(next->prev, elem, next);
}

static inline void
list_insert_head(struct list *head, struct list *elem)
{
    list_insert_after(head, elem);
}

static inline void
list_insert_tail(struct list *head, struct list *elem)
{
    list_insert_before(head, elem);
}

static inline void
list_remove(struct list *elem)
{
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
}

static inline void
list_remove_init(struct list *elem)
{
    list_remove(elem);
    list_init(elem);
}

static inline int
list_is_empty(struct list *head)
{
    return head->next == head;
}

#define list_entry(_elem, _type, _memb) \
    containerof(_elem, _type, _memb)

#define __list_first_entry(_head, _type, _memb) \
    list_entry((_head)->next, _type, _memb)

#define __list_last_entry(_head, _type,  _memb) \
    list_entry((_head)->prev, _type, _memb)

#define __list_prev_entry(_entry, _memb)                        \
    list_entry((_entry)->_memb.next, typeof(*(_entry)), _memb)

#define __list_next_entry(_entry, _memb)                        \
    list_entry((_entry)->_memb.next, typeof(*(_entry)), _memb)

#define list_first_entry(_head, _type, _memb)   \
    list_is_empty(_head)                        \
    ? NULL                                      \
    : __list_first_entry(_head, _type, _memb)

#define list_last_entry(_head, _type, _memb)    \
    list_is_empty(_head)                        \
    ? NULL                                      \
    : __list_last_entry(_head, _type, _memb)

#define list_prev_entry(_head, _entry, _memb)                       \
    ({                                                              \
        typeof(_entry) __prev;                                      \
        __prev = __list_prev_entry((_entry), _memb);                \
        &__prev->entry == (_head) ? NULL : __prev;                  \
    })

#define list_next_entry(_head, _entry, _memb)                       \
    ({                                                              \
        typeof(_entry) __next;                                      \
        __next = __list_next_entry(_entry, _memb);                  \
        &__next->entry == (_head) ? NULL : __next;                  \
    })

#define list_for_each_entry(_head, _entry, _memb)                     \
    for ((_entry) = __list_first_entry(_head,                         \
                                       typeof(*(_entry)), _memb);     \
         (_entry) != list_entry(_head,                                \
                                typeof(*(_entry)), _memb);            \
         (_entry) = __list_next_entry(_entry, _memb))

#define list_for_each_entry_reverse(_head, _entry, _memb)              \
    for ((_entry) = __list_last_entry(_head,                          \
                                      typeof(*(_entry)), _memb);      \
         (_entry) != list_entry(_head,                                \
                                typeof(*(_entry)), _memb);            \
         (_entry) = __list_prev_entry(_entry, _memb))

#define list_for_each_entry_continue_reverse(_head, _entry, _memb)    \
    for ((_entry) = __list_prev_entry(_entry, _memb);                 \
         (_entry) != list_entry(_head,                                \
                                typeof(*(_entry)), _memb);            \
         (_entry) = __list_prev_entry(_entry, _memb))

#define list_for_each_entry_safe(_head, _entry, _next, _memb)         \
    for ((_entry) = __list_first_entry(_head,                         \
                                       typeof(*(_entry)), _memb);     \
         (_next) = __list_next_entry(_entry, _memb),                  \
             (_entry) != list_entry(_head,                            \
                                    typeof(*(_entry)), _memb);        \
         (_entry) = (_next))

#endif

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
