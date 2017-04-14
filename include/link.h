#pragma once

#include <types.h>

typedef struct Link
{
	struct Link *prev;
	struct Link *next;
} Link;

static inline void link_init(Link *x)
{
	x->prev = x;
	x->next = x;
}

static inline void link_after(Link *a, Link *x)
{
	Link *p = a;
	Link *n = a->next;
	n->prev = x;
	x->next = n;
	x->prev = p;
	p->next = x;
}

static inline void link_before(Link *a, Link *x)
{
	Link *p = a->prev;
	Link *n = a;
	n->prev = x;
	x->next = n;
	x->prev = p;
	p->next = x;
}

static inline void link_remove(Link *x)
{
	Link *p = x->prev;
	Link *n = x->next;
	n->prev = p;
	p->next = n;
	x->next = 0;
	x->prev = 0;
}

static inline void link_move_after(Link *a, Link *x)
{
	Link *p = x->prev;
	Link *n = x->next;
	n->prev = p;
	p->next = n;

	p       = a;
	n       = a->next;
	n->prev = x;
	x->next = n;
	x->prev = p;
	p->next = x;
}

static inline void link_move_before(Link *a, Link *x)
{
	Link *p = x->prev;
	Link *n = x->next;
	n->prev = p;
	p->next = n;

	p       = a->prev;
	n       = a;
	n->prev = x;
	x->next = n;
	x->prev = p;
	p->next = x;
}

static inline bool link_list_is_empty(Link *x)
{
	return x->next == x;
}

#define LinkData(link, T, m) \
	(T *)((char *)(link) - (unsigned long)(&(((T *)0)->m)))

#define LinkListForEach(it, list, m)                 \
	for (it = LinkData((list).next, typeof(*it), m); \
	     &it->m != &(list);                          \
	     it = LinkData(it->m.next, typeof(*it), m))
