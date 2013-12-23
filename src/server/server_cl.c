#include "server.h"
#include "sgame/tm_worldmap.h"

#ifndef __SYNTHETIC__


void tm_sv_client_init( tm_sv_client_t* cl, tm_entity_t* _pl, char* _name, addr_t* addr )
{
	assert( cl && _name && strlen(_name) < MAX_NAME_LEN && addr && _pl );
	cl->cl_id = _pl->ent_id;
	strcpy( cl->name, _name );
	cl->addr = *addr;
	cl->tid = rand_n( sv.num_threads );
	cl->tolm = get_tm();
	cl->last_action = -1;
	cl->player = _pl;
}

tm_entity_t* tm_server_add_cl( char* pname, addr_t* addr )
{
	//Begin transaction
	BEGIN_TRANSACTION();

	tm_entity_t* pl = tm_worldmap_generate_ent( ET_PLAYER );
	if( !pl )
	{	COMMIT_TRANSACTION(); return NULL;}

	int res = tm_worldmap_add( pl );
	if( !res )
	{	tm_entity_destroy( pl ); COMMIT_TRANSACTION(); return NULL;}

	sv.clients[ pl->ent_id ] = tm_sv_client_create( pl, pname, addr );
	sv.n_clients++;

	//Commit the transaction
	COMMIT_TRANSACTION();
	return pl;
}

void tm_server_del_cl( int cl_id, addr_t* addr )
{
	//Begin the transaction
	BEGIN_TRANSACTION();

	tm_sv_client_t* cl = sv.clients[cl_id]; assert( cl && addr_equal( addr, &cl->addr ) );

	sv.n_clients--;
	sv.clients[cl->cl_id] = NULL;
	tm_worldmap_del( cl->player );

	tm_entity_destroy( cl->player );
	tm_sv_client_destroy(cl);

	//Commit the transaction
	COMMIT_TRANSACTION();

	printf("[I] %s left.\n", cl->name );

}

void tm_server_act_cl( int cl_id, addr_t* addr, streamer_t* st )
{
	//Begin the transaction
	BEGIN_TRANSACTION();

	tm_sv_client_t* cl = sv.clients[cl_id]; assert( cl && addr_equal( addr, &cl->addr ) );
	tm_entity_t* pl = cl->player;

	// get the counter value for the received action
	int la = streamer_rdint( st ); assert( la >= 0 && la == cl->last_action+1 );
	cl->last_action = la;

	// get the number of actions to perform
	char n_acts = streamer_rdchar( st ); assert( n_acts >= 0 );
	char a_i;
	for( a_i = 0; a_i < n_acts; a_i++ )
	{
		// get the id of the action to be performed
		// a_id < 0 => change the value of some attribute (pseudo-action)
		// a_id > 0 => perform an actual action (eg. eat)
		// a_id represents the id of the attribute/action incremented to avoid the value "0"
		char a_id = streamer_rdchar( st );

		assert( sizeof(value_t) == sizeof(int));
		if( a_id < 0 ) tm_entity_set_attr( pl, (-a_id)-1, streamer_rdint( st ) );
		if( a_id > 0 ) tm_game_action( a_id-1, pl );
	}

	//End the transaction
	END_TRANSACTION();
}

void tm_server_update_cl( tm_sv_client_t* cl, streamer_t* st )
{
	assert( cl );
	streamer_wrint( st, cl->last_action );
	tm_entity_pack( cl->player, st );

	rect_t vr = tm_game_action_range( AC_VIEW, cl->player, &wm.map_r );
	tm_worldmap_pack( st, &vr );
}
#endif
