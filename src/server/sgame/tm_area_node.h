#ifndef TM_AREA_NODE_H_
#define TM_AREA_NODE_H_

#include "../../general.h"
#include "../../utils/geometry.h"
#include "../../utils/tm_hset.h"
#include "../../game/tm_game.h"


typedef struct _tm_area_node_t
{
	rect_t loc;
	rect_t split;

	struct _tm_area_node_t* left;
	struct _tm_area_node_t* right;

	entity_set_t* ent0_sets;
	entity_set_t* ent1_sets;
	tm_entity_set_t* ent2_sets;
}tm_area_node_t;


#define tm_area_node_create( _loc, _split_dir, level )	tm_generic_create3( tm_area_node_t, tm_area_node_init, _loc, _split_dir, level )
#define tm_area_node_destroy( an )			tm_generic_destroy( an, tm_area_node_deinit )

#define tm_area_node_is_leaf( an )			( an->left == NULL )


extern tm_area_node_t tm_area_tree;


void tm_area_node_init( tm_area_node_t* an, rect_t _loc, int _split_dir, int level);
void tm_area_node_deinit( tm_area_node_t* an );

tm_area_node_t* tm_area_node_get_node_containing_rect( tm_area_node_t* an, rect_t* r );
tm_area_node_t* tm_area_node_get_node_containing_rect2( tm_area_node_t* an, tm_rect_t* r );

tm_area_node_t* tm_area_node_get_node_containing_ent0( tm_area_node_t* an, entity_t* ent );
tm_area_node_t* tm_area_node_get_node_containing_ent1( tm_area_node_t* an, tm_entity_stationary_t* ent );
tm_area_node_t* tm_area_node_get_node_containing_ent2( tm_area_node_t* an, tm_entity_movable_t* ent );

void tm_area_node_add0( tm_area_node_t* an, entity_t* ent );
void tm_area_node_add1( tm_area_node_t* an, tm_entity_stationary_t* ent );
void tm_area_node_add2( tm_area_node_t* an, tm_entity_movable_t* ent );

void tm_area_node_del0( tm_area_node_t* an, entity_t* ent );
void tm_area_node_del1( tm_area_node_t* an, tm_entity_stationary_t* ent );
void tm_area_node_del2( tm_area_node_t* an, tm_entity_movable_t* ent );

int  tm_area_node_is_vacant( tm_area_node_t* an, rect_t* r, int etypes );

void tm_area_node_is_valid( tm_area_node_t* an, int* n_ents );

void tm_area_node_assign_lock( tm_area_node_t* an );

//	OBSOLETE
//void tm_area_node_get_entities( tm_area_node_t* an, rect_t* range, int etypes, tm_pentity_set_t* pe_set );



/*		TM AREA_NODE POINTER ELEMENT		*/

typedef struct
{
	elem_t e;
	tm_area_node_t* an;
} tm_parea_node_t;

inline static void tm_parea_node_init( tm_parea_node_t* pan, tm_area_node_t* an )
{
	assert( pan && an );
	elem_init( &pan->e, 0 );
	pan->an = an;
}

#define tm_parea_node_create( an )	tm_generic_create1( tm_parea_node_t, tm_parea_node_init, an )
#define tm_parea_node_destroy( pan )	tm_generic_free( pan )




/*		TM AREA_NODE POINTER SET		*/

#define tm_parea_node_set_t			hset_t

#define tm_parea_node_set_init( pan_s )		hset_init( pan_s )
#define tm_parea_node_set_deinit( pan_s )	hset_deinit( pan_s, tm_parea_node_destroy )
#define tm_parea_node_set_create()		hset_create2( 1, 1000000000 )
#define tm_parea_node_set_destroy( pan_s )	hset_destroy( pan_s, tm_parea_node_destroy )
#define tm_parea_node_set_add( pan_s, pan )	hset_add( pan_s, pan )
#define tm_parea_node_set_del( pan_s, pan )	hset_del( pan_s, pan )

#define tm_parea_node_set_for_each( pos, pan_s )	hset_key_for_each( pos, pan_s, 0 )
#define tm_parea_node_set_size( pan_s )		hset_size( pan_s )


void tm_area_node_get_nodes( tm_area_node_t* an, rect_t* range, tm_parea_node_set_t* pan_set );
void tm_area_node_get_leaves( tm_area_node_t* an, rect_t* range, tm_parea_node_set_t* pan_set );
void tm_area_node_get_parents( tm_area_node_t* an, rect_t* range, tm_parea_node_set_t* pan_set );



typedef struct _level_stats_t
{
	int* n_ents;
	int* min_ents;
	int* max_ents;
} level_stats_t;
 
void tm_area_node_print( tm_area_node_t* an, int level, int verbose, level_stats_t* level_stats );


#endif /*TM_AREA_NODE_H_*/

