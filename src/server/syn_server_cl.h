#ifndef SYN_SERVER_CL_H_
#define SYN_SERVER_CL_H_

#include "../general.h"
#include "../utils/streamer.h"
#include "../game/tm_entity.h"
#include "../game/action.h"

#include "tm.h"

typedef value_t m_action_t[N_ACTIONS_ATTRIBUTES];

typedef struct _tm_sv_client_t
{
	int cl_id;
	char name[MAX_NAME_LEN];
	int tid;			// thread id of the server handling requests/updates for this client

	tm_ullong tolm;			// time_of_last_message
	tm_int last_action;		// the counter of the last action received from this client
	int last_dist;

	tm_entity_movable_t* player;

	m_action_t* m_actions;
}tm_sv_client_t;

void tm_sv_client_init( tm_sv_client_t* cl, tm_entity_movable_t* _pl, char* _name );

#define tm_sv_client_create( _pl, _name )		tm_generic_create2( tm_sv_client_t, tm_sv_client_init, _pl, _name )

tm_entity_movable_t* tm_server_add_cl( char* pname );
void tm_server_act_cl_synthetic( int cl_id, int cycle );

#endif /*SERVER_CL_H_*/
