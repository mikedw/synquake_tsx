#ifndef TM_ENTITY_H_
#define TM_ENTITY_H_

#include "../utils/tm_hset.h"
#include "../utils/tm_geometry.h"

#include "entity.h"


#define	LOCK_PER_WORD	0
#define	LOCK_PER_ENT	1
#define	LOCK_PER_SET	2
#define	LOCK_GLOBAL	3
#define	N_LOCK_GRAN	4

#define	LOCK_PER_WORD_S	"word"
#define	LOCK_PER_ENT_S	"ent"
#define	LOCK_PER_SET_S	"set"
#define	LOCK_GLOBAL_S	"global"

extern int lock_granularity;
extern char lock_gran_names[N_LOCK_GRAN][10];


typedef tm_type<value_t>	tm_value_t;

/*************************************
			TM_ENTITY STATIONARY
*************************************/


class tm_entity_stationary_t
{
public :
	elem_t e;    /* required for "hset" integration (needs to be the first member of the structure) */
	rect_t r;    /* position as defined by 2 oposing corners representing the bounding box */
	vect_t size;

	int ent_type; /* = ent_id of the entity_type corresponding to this entity */
	int ent_id;   /* ent_id within this entity's type */
	tm_value_t attrs[0]; /* values of the attributes for this entity */
};



#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
void tm_entity_stationary_init( tm_entity_stationary_t* ent, int _ent_type );

#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
void tm_entity_stationary_destroy( tm_entity_stationary_t* ent );

#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
tm_entity_stationary_t* tm_entity_stationary_create( int _ent_type );

#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
void tm_entity_stationary_generate_attrs( tm_entity_stationary_t* ent );

#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
void tm_entity_stationary_generate_position( tm_entity_stationary_t* ent, rect_t* map_r );

#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
int tm_entity_stationary_is_valid( tm_entity_stationary_t* ent, vect_t* map_sz );

#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
void tm_entity_stationary_set_attr( tm_entity_stationary_t* ent, int attr_id, value_t attr_val );

#define tm_entity_stationary_is_eq0( ent1, ent2 )		( ent1->ent_id == ent2->ent_id && ent1->ent_type == ent2->ent_type )
#define tm_entity_stationary_is_eq1( ent1, ent2 )		( ent1->ent_id == ent2->ent_id && ent1->ent_type == ent2->ent_type )
#define tm_entity_stationary_is_eq2( ent1, ent2 )		( ent1->ent_id == ent2->ent_id && ent1->ent_type == ent2->ent_type )

void tm_entity_stationary_pack_fixed( tm_entity_stationary_t* ent, streamer_t* st );
void tm_entity_stationary_unpack_fixed( tm_entity_stationary_t* ent, streamer_t* st );

void tm_entity_stationary_pack( tm_entity_stationary_t* ent, streamer_t* st );
void tm_entity_stationary_unpack( tm_entity_stationary_t* ent, streamer_t* st );

void tm_entity_copy10( tm_entity_stationary_t* ent1, entity_t* ent );


/*************************************
			TM_ENTITY Movable
*************************************/


class tm_entity_movable_t
{
public :
	tm_elem_t e; /* required for "hset" integration (needs to be the first member of the structure) */
	tm_rect_t r; /* position as defined by 2 oposing corners representing the bounding box */
	vect_t size;

	int ent_type; /* = ent_id of the entity_type corresponding to this entity */
	int ent_id;   /* ent_id within this entity's type */
	tm_value_t attrs[0]; /* values of the attributes for this entity */
};



#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
void tm_entity_movable_init( tm_entity_movable_t* ent, int _ent_type );

#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
void tm_entity_movable_destroy( tm_entity_movable_t* ent );

#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
tm_entity_movable_t* tm_entity_movable_create( int _ent_type );

#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
void tm_entity_movable_generate_attrs( tm_entity_movable_t* ent );

#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
void tm_entity_movable_generate_position( tm_entity_movable_t* ent, rect_t* map_r );

#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
int tm_entity_movable_is_valid( tm_entity_movable_t* ent, vect_t* map_sz );

#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
void tm_entity_movable_set_attr( tm_entity_movable_t* ent, int attr_id, value_t attr_val );

#define tm_entity_movable_is_eq0( ent1, ent2 )		( ent1->ent_id == ent2->ent_id && ent1->ent_type == ent2->ent_type )
#define tm_entity_movable_is_eq1( ent1, ent2 )		( ent1->ent_id == ent2->ent_id && ent1->ent_type == ent2->ent_type )
#define tm_entity_movable_is_eq2( ent1, ent2 )		( ent1->ent_id == ent2->ent_id && ent1->ent_type == ent2->ent_type )

void tm_entity_movable_pack_fixed( tm_entity_movable_t* ent, streamer_t* st );
void tm_entity_movable_unpack_fixed( tm_entity_movable_t* ent, streamer_t* st );

void tm_entity_movable_pack( tm_entity_movable_t* ent, streamer_t* st );
void tm_entity_movable_unpack( tm_entity_movable_t* ent, streamer_t* st );

void tm_entity_copy20( tm_entity_movable_t* ent2, entity_t* ent );



/*		TM_ENTITY SET		*/

#define tm_entity_set_t                     tm_hset_t
#define _tm_entity_destroy( ent )           tm_entity_movable_destroy( ((tm_entity_movable_t*)ent) )

#define tm_entity_set_init( ent_s )         tm_hset_init2( ent_s, 1, 1000000000 )
#define tm_entity_set_deinit( ent_s )       tm_hset_deinit( ent_s, _tm_entity_destroy )

#define tm_entity_set_add( ent_s, _ent)     tm_hset_add( ent_s, _ent )
#define tm_entity_set_del( ent_s, _ent )    tm_hset_del( ent_s, _ent )

#define tm_entity_set_for_each(pos, ent_s)  tm_hset_key_for_each( pos, ent_s, 0 )
#define tm_entity_set_size( ent_s )	    tm_hset_size( ent_s )





/*		TM ENTITY POINTER ELEMENT		*/
//	OBSOLETE
/*
typedef struct
{
	elem_t e;
	tm_entity_t* ent;
} tm_pentity_t;

inline static void tm_pentity_init( tm_pentity_t* pe, tm_entity_t* ent )
{
	assert( pe && ent );
	elem_init( &pe->e, 0 );
	pe->ent = ent;
}

#define tm_pentity_create( ent )				tm_generic_create1( tm_pentity_t, tm_pentity_init, ent )
#define tm_pentity_destroy( pent )				tm_generic_free( pent )
*/



/*		TM ENTITY POINTER SET		*/
//	OBSOLETE
/*
#define tm_pentity_set_t						hset_t

#define tm_pentity_set_init( pent_s )			hset_init( pent_s )
#define tm_pentity_set_deinit( pent_s )			hset_deinit( pent_s, tm_pentity_destroy )
#define tm_pentity_set_create()					hset_create2( 1, 1000000000 )
#define tm_pentity_set_destroy( pent_s )		hset_destroy( pent_s, tm_pentity_destroy )
#define tm_pentity_set_add( pent_s, pent )		hset_add( pent_s, pent )
#define tm_pentity_set_del( pent_s, pent )		hset_del( pent_s, pent )

#define tm_pentity_set_for_each( pos, pent_s )		hset_key_for_each( pos, pent_s, 0 )
#define tm_pentity_set_size( pent_s )				hset_size( pent_s )
*/

#endif /*TM_ENTITY_H_*/
