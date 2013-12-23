#ifndef SERVER_CL_H_
#define SERVER_CL_H_

#include "../general.h"
#include "../utils/streamer.h"
#include "sgame/sgame.h"



typedef struct
{
	int cl_id;
	char name[MAX_NAME_LEN];
	addr_t addr;
	int tid;	// thread id of the server handling requests/updates for this client
	unsigned long long tolm;  // time_of_last_message
	int last_action;	  // the counter of the last action received from this client

	tm_entity_t* player;
} tm_sv_client_t;

void tm_sv_client_init( tm_sv_client_t* cl, tm_entity_t* _pl, char* _name, addr_t* addr );

#define tm_sv_client_create( _pl, _name, addr )	tm_generic_create3( tm_sv_client_t, tm_sv_client_init, _pl, _name, addr )
#define tm_sv_client_destroy( cl )		tm_generic_free( cl )

tm_entity_t* tm_server_add_cl( char* pname, addr_t* addr );
void tm_server_del_cl( int cl_id, addr_t* addr );
void tm_server_act_cl( int cl_id, addr_t* addr, streamer_t* st );
void tm_server_update_cl( tm_sv_client_t* cl, streamer_t* st );

#endif /*SERVER_CL_H_*/
