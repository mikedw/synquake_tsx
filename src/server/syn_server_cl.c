#include "server.h"
#include "sgame/tm_worldmap.h"
#include "sgame/sgame.h"

#ifdef __SYNTHETIC__


void tm_sv_client_init( tm_sv_client_t* cl, tm_entity_movable_t* _pl, char* _name )
{
	assert( cl && _name && strlen(_name) < MAX_NAME_LEN && _pl );
	cl->cl_id = _pl->ent_id;
	strcpy( cl->name, _name );

	int aux1 = (cl->cl_id % (sv.num_threads * sv.num_threads)) / sv.num_threads;
	int aux2 = ((cl->cl_id % sv.num_threads) + aux1) % sv.num_threads;
	cl->tid = aux2;//cl->cl_id % sv.num_threads ;//0;//rand_n( sv.num_threads );

	cl->tolm = get_tm();
	cl->last_action = -1;
	cl->player = _pl;

	assert( sv.n_multiple_actions );
	cl->m_actions = (m_action_t*) calloc ( sv.n_multiple_actions, sizeof(m_action_t) );
	assert( cl->m_actions );
}

tm_entity_movable_t* tm_server_add_cl( char* pname )
{
	tm_entity_movable_t* pl = NULL;

	BEGIN_TRANSACTION();

	#ifndef RUN_TRACES
		rect_t where = tm_wm.map_r;

		#ifdef GENERATE_NEAR_QUESTS
		if( sv.wl_quest_count )
		{
			double aux = sqrt( sv.wl_quest_spread );
			double fx[ N_DIRS ] = { tm_wm.size.y / aux / 2, tm_wm.size.x / aux / 2,
									tm_wm.size.y / aux / 2, tm_wm.size.x / aux / 2 };
			rect_t qr = sv.quest_locations[ 0 ][ sv.n_clients % sv.wl_quest_spread ];
			rect_expand( &qr, fx, &where );
			rect_crop( &where, &tm_wm.map_r, &where );
		}
		#endif
		
		pl = tm_worldmap_generate_ent2( ET_PLAYER, &where );
	#else
		entity_t* ent = tm_worldmap_generate_ent_from_trace( ET_PLAYER );
		if( ent )
		{
			pl = tm_entity_movable_create( ET_PLAYER ); assert( pl );
			tm_entity_copy20( pl, ent );
			entity_destroy( ent );
		}
	#endif

	if( pl )
	{
		int res = tm_worldmap_add2( pl );
		if( res )
		{
			sv.clients[ pl->ent_id ] = tm_sv_client_create( pl, pname );
			sv.n_clients = sv.n_clients + 1;
		}
		else
		{
			tm_entity_movable_destroy( pl );
			pl = NULL;
		}
	}

	END_TRANSACTION();

	return pl;
}

void tm_server_act_cl_synthetic( int cl_id, int cycle )
{
	tm_sv_client_t* cl = sv.clients[cl_id];	assert(cl);
	tm_entity_movable_t* pl = cl->player;		assert(pl);

	tm_game_multiple_action( cl, cycle );
	//tm_game_action( action_id, pl, cycle );
}
#endif
