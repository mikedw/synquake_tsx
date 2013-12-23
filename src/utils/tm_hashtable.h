#ifndef TM_HASHTABLE_H_
#define TM_HASHTABLE_H_

#include "../general.h"
#include "tm_list.h"

typedef struct _tm_hashtable_t: public tm_obj
{
	tm_type<tm_list_t*> bkts;
	tm_int n_bkts;
	tm_int n;
	tm_int fx;
} tm_hashtable_t;


#define tm_hashtable_for_each( pos, cnt, h )			\
  for( cnt = 0; cnt < (h)->n_bkts; cnt++ )	tm_list_for_each( pos, &((tm_list_t*)(h)->bkts)[cnt] )

#define tm_hashtable_for_each_safe( pos, nxt, cnt, h )	\
  for( cnt = 0; cnt < (h)->n_bkts; cnt++ )	tm_list_for_each_safe( pos, nxt, &((tm_list_t*)(h)->bkts)[cnt] )


#define tm_bucket_for_each( pos, h, key )				\
  tm_list_for_each( pos, &((tm_list_t*)(h)->bkts)[ key % ((h)->n_bkts) ] )

#ifndef LOCK_PER_WORD
#define		LOCK_PER_WORD	0
#define		LOCK_PER_ENT	1
#define		LOCK_PER_SET	2
#define		LOCK_GLOBAL	3
#endif

extern int lock_granularity;
extern unsigned int g_lock[3];

static
#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
inline void tm_hashtable_init( tm_hashtable_t* h, unsigned int _n_bkts, unsigned int _fx )
{
	unsigned int i;
	assert( h && _n_bkts > 0 && _fx > 0 );

	h->n = 0;
	h->fx = _fx;
	h->n_bkts = _n_bkts;

	h->bkts = (tm_list_t*) mgr_on_new( _n_bkts * sizeof( tm_list_t ) );	
	assert( (tm_list_t*)h->bkts );

	for( i = 0; i < _n_bkts; i++ )
		tm_list_init( &((tm_list_t*) h->bkts)[i] );
}

static
#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
inline void tm_hashtable_grow( tm_hashtable_t* h )
{
	int i;
	tm_list_t* old_bkts = h->bkts;
	int old_n_bkts = h->n_bkts;
	tm_hashtable_init( h, h->n_bkts * 2, h->fx );

	tm_elem_t *elem, *nxt;
	for( i = 0; i < old_n_bkts; i++ )
		tm_list_for_each_safe( elem, nxt, &old_bkts[i] )
			tm_list_move( &((tm_list_t*) h->bkts)[(int) elem->key % (int) h->n_bkts], elem );

	free( old_bkts );
}

static
#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
inline void tm_hashtable_add( tm_hashtable_t* h, tm_elem_t* elem )
{
	assert( h && elem );
	tm_list_add( &((tm_list_t*) h->bkts)[(int) elem->key % (int) h->n_bkts], elem );
	h->n = h->n + 1;

	if( h->n > h->n_bkts * h->fx )
		tm_hashtable_grow( h );
}

static
#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
inline void tm_hashtable_del( tm_hashtable_t* h, tm_elem_t* elem )
{
	assert( h && elem );
	tm_list_del( elem );
	h->n = h->n - 1;
}

 #define tm_hashtable_find( h, elem, elem_eq )          \
({                                                     \
   assert( h && elem );                                \
   tm_elem_t *pos, *rez = NULL;                        \
   bucket_for_each( pos, h, (elem)->key )              \
   if( elem_eq( pos, elem) )                           \
   {                                                   \
         rez = pos;                                    \
         break;                                        \
   }                                                   \
   rez;                                                \
})

 #define tm_hashtable_remove( h, elem, elem_eq )             \
({                                                           \
   assert( h && elem );                                      \
   tm_elem_t *rez = tm_hashtable_find( h, elem, elem_eq );   \
   if( rez ) tm_hashtable_del( h, rez );                     \
})

#define tm_hashtable_clear( h, elem_destroy )               \
({                                                          \
	int i_cnt = 0;                                      \
	tm_list_t *pos, *nxt;                               \
	assert( h );                                        \
                                                            \
	tm_hashtable_for_each_safe( pos, nxt, i_cnt, h )    \
	{                                                   \
		tm_list_del( pos );                         \
		elem_destroy( pos );                        \
	}                                                   \
	(h)->n = 0;                                         \
})

#define tm_hashtable_deinit( h, elem_destroy )              \
({                                                          \
	assert(h);                                          \
	tm_hashtable_clear( h, elem_destroy );              \
	free( (h)->bkts );                             \
})

#define tm_hashtable_create()                          tm_generic_create2( tm_hashtable_t, tm_hashtable_init, 16, 4 )
#define tm_hashtable_create2( _n_bkts, _fx )           tm_generic_create2( tm_hashtable_t, tm_hashtable_init, _n_bkts, _fx )
#define tm_hashtable_destroy( h, elem_destroy )        tm_generic_destroy1( h, tm_hashtable_deinit, elem_destroy )


#endif /*TM_HASHTABLE_H_*/

