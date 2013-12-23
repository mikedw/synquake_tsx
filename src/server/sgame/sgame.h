#ifndef SGAME_H_
#define SGAME_H_

#include "../../game/tm_game.h"
#include "tm_worldmap.h"
#include "../server.h"

void tm_game_action( int a_id, tm_entity_movable_t* pl, int cycle );
void tm_game_multiple_action( tm_sv_client_t* cl, int cycle );

#endif /*SGAME_H_*/
