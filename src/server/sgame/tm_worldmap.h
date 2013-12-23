#ifndef TM_WORLDMAP_H_
#define TM_WORLDMAP_H_

#include "../../utils/streamer.h"
#include "tm_area_node.h"

typedef tm_type<tm_entity_movable_t*> tm_p_tm_entity_movable_t;

typedef struct
{
	int*    n_entities0;
	int*    n_entities1;
	tm_int* n_entities2;
	
	entity_t***		entities0;
	tm_entity_stationary_t*** 	entities1;
	tm_p_tm_entity_movable_t**	entities2;
	
	int* 	et_ratios0;
	int* 	et_ratios1;
	tm_int* et_ratios2;

	rect_t map_r;
	rect_t map_walls[N_DIRS];
	vect_t size;
	int area;

	tm_area_node_t* area_tree;
	int depth;
} tm_worldmap_t;

extern tm_worldmap_t tm_wm;

void tm_worldmap_init( conf_t* c, coord_t szx, coord_t szy, int tdepth );

int tm_worldmap_add0( entity_t* ent );
int tm_worldmap_add1( tm_entity_stationary_t* ent );
int tm_worldmap_add2( tm_entity_movable_t* ent );

void tm_worldmap_del0( entity_t* ent );
void tm_worldmap_del1( tm_entity_stationary_t* ent );
void tm_worldmap_del2( tm_entity_movable_t* ent );

void tm_worldmap_move2( tm_entity_movable_t* ent, vect_t* move_v );

int tm_worldmap_is_vacant_from_fixed( tm_rect_t* r );
int tm_worldmap_is_vacant_from_mobile( tm_rect_t* r );
int tm_worldmap_is_vacant( tm_rect_t* r );

entity_t*     tm_worldmap_generate_ent0(int ent_type, rect_t* where );
tm_entity_stationary_t* tm_worldmap_generate_ent1(int ent_type, rect_t* where );
tm_entity_movable_t* tm_worldmap_generate_ent2(int ent_type, rect_t* where );

#ifdef RUN_TRACES
tm_entity_t* tm_worldmap_generate_ent_from_trace(int ent_type );
#endif
void tm_worldmap_generate();

tm_parea_node_set_t* tm_worldmap_get_nodes( rect_t* range );
tm_parea_node_set_t* tm_worldmap_get_leaves( rect_t* range );
tm_parea_node_set_t* tm_worldmap_get_parents( rect_t* range );

void tm_worldmap_is_valid();
void tm_worldmap_print( int verbose );



/*	OBSOLETE	*/
//tm_pentity_set_t* tm_worldmap_get_entities( rect_t* range, int etypes );
//void tm_worldmap_pack( streamer_t* st, rect_t* range );
//void tm_worldmap_pack_fixed( streamer_t* st );


#endif /*TM_WORLDMAP_H_*/

