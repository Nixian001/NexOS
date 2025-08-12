#pragma once

typedef struct link_t {
    struct link_t *next;
    struct link_t *prev;
} link_t;

static inline void link_init(link_t *link) {
    link->next = link;
    link->prev = link;
}

static inline void link_after(link_t *a, link_t *x)
{
    link_t *p = a;
    link_t *n = a->next;
    n->prev = x;
    x->next = n;
    x->prev = p;
    p->next = x;
}

static inline void link_before(link_t *a, link_t *b) {
    link_t *p = a->prev;
    link_t *n = a;

    n->prev = b;
    b->next = n;
    b->prev = p;
    p->next = b;
}

static inline void link_remove(link_t *x)
{
    link_t *p = x->prev;
    link_t *n = x->next;
    n->prev = p;
    p->next = n;
    x->next = 0;
    x->prev = 0;
}

#define link_data(link,T,m) (T *)((char *)(link) - (unsigned long)(&(((T*)0)->m)))

#define list_foreach(it, list, m) \
for (it = link_data((list).next, typeof(*it), m); \
    &it->m != &(list); \
    it = link_data(it->m.next, typeof(*it), m))

#define list_foreach_safe(it, n, list, m) \
    for (it = link_data((list).next, typeof(*it), m), \
        n = link_data(it->m.next, typeof(*it), m); \
        &it->m != &(list); \
        it = n, \
        n = link_data(n->m.next, typeof(*it), m))