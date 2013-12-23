#ifndef TM_GAME_H_
#define TM_GAME_H_

#include "tm_entity.h"

#include "game.h"

void tm_game_entity1_generate_size( tm_entity_stationary_t* ent );
void tm_game_entity1_pack( tm_entity_stationary_t* ent, streamer_t* st );
void tm_game_entity1_unpack( tm_entity_stationary_t* ent, streamer_t* st );

void tm_game_entity2_generate_size( tm_entity_movable_t* ent );
void tm_game_entity2_pack( tm_entity_movable_t* ent, streamer_t* st );
void tm_game_entity2_unpack( tm_entity_movable_t* ent, streamer_t* st );


rect_t tm_game_action_range( int a_id, tm_entity_movable_t* pl, rect_t* map_r );


#endif /*TM_GAME_H_*/
