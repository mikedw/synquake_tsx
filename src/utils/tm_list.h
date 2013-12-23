#ifndef __TM_LIST_H
#define __TM_LIST_H

#include "tm.h"

class tm_list_t //: public tm_obj
{
public:
	tm_type<tm_list_t*> next;
	tm_type<tm_list_t*> prev;
	tm_type<unsigned int> key;
};

typedef class tm_list_t tm_elem_t;

#define tm_list_for_each( pos, list )		for( pos = (list)->next; pos != (list); pos = pos->next )
#define tm_list_for_each_safe(pos, nxt, list)	for( pos = (list)->next, nxt = pos->next; pos != (list); pos = nxt, nxt = pos->next )

static
#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
inline void tm_elem_init( tm_elem_t* elem, unsigned int _key )
{
	elem->next = NULL;
	elem->prev = NULL;
	elem->key = _key;
}

static
#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
inline void tm_list_init( tm_list_t* list )
{
	list->next = list;	
	list->prev = list;
	list->key = 0xffffffff;
}

static
#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
inline void tm__list_add( tm_elem_t* elem, tm_elem_t* prev, tm_elem_t* next )
{
	elem->next = next;
	elem->prev = prev;

#ifndef ADD_NP
	prev->next = elem;
	next->prev = elem;
#else
	next->prev = elem;
	prev->next = elem;
#endif
}

static
#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
inline void tm_list_add_first( tm_list_t* list, tm_elem_t* elem )
{
	tm__list_add( elem, list, list->next );
}

static
#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
inline void tm_list_add( tm_list_t* list, tm_elem_t* elem )
{
	tm__list_add( elem, list->prev, list );
}

static
#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
inline void tm___list_del( tm_elem_t* prev, tm_elem_t* next )
{
#ifndef DEL_NP
	prev->next = next;
	next->prev = prev;
#else
	next->prev = prev;
	prev->next = next;
#endif
}

static
#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
inline void tm_list_del( tm_elem_t* elem )
{
	tm___list_del( elem->prev, elem->next );
	elem->next = NULL;
	elem->prev = NULL;
}

static
#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
inline void tm_list_move( tm_list_t* list, tm_elem_t* elem )
{
	tm___list_del( elem->prev, elem->next );
	tm_list_add( list, elem );
}

static
#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
inline int tm_list_empty( tm_list_t* list )
{
	return list->next == list;
}

#endif
