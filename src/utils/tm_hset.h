#ifndef TM_HSET_H_
#define TM_HSET_H_

#include "tm_hashtable.h"

#define tm_hset_t                   		tm_hashtable_t

#define tm_hset_init2( hset, _n_bkts, _fx )	tm_hashtable_init( hset, _n_bkts, _fx )
#define tm_hset_create2( _n_bkts, _fx )		tm_hashtable_create2( _n_bkts, _fx )

#define tm_hset_deinit( hset, elem_destroy )	tm_hashtable_deinit( hset, elem_destroy )
#define tm_hset_destroy( hset, elem_destroy )	tm_hashtable_destroy( hset, elem_destroy )

#define tm_hset_add( hset, elem )		tm_hashtable_add( hset, &(elem)->e )
#define tm_hset_del( hset, elem )		tm_hashtable_del( hset, &(elem)->e )

#define tm_hset_for_each( pos, cnt, hset )	tm_hashtable_for_each( pos, cnt, hset )
#define tm_hset_key_for_each( pos, hset, key )	tm_bucket_for_each( pos, hset, key )

#define tm_hset_size( hset )			((int)(hset)->n)

#endif /*TM_HSET_H_*/


