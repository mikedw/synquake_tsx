#ifndef SERVER_STATS_H_
#define SERVER_STATS_H_

#ifndef __SYNTHETIC__


#include "server_cl.h"

/* STATS INDEXES   */


#define		WAIT_REQ			0
#define		RECV_REQ			1
#define		PROC_REQ			2
#define		SV_PROC_REQ			3

#define		PROC_UPD			4
#define		SEND_UPD			5
#define		SV_PROC_UPD			6

#define		N_STATS				7



#else


#include "syn_server_cl.h"

/* STATS INDEXES   */


#define		TOTAL				0
#define		PROCESS				1
#define		GET_NODES			2
#define		ACTION				3
#define		DEST_NODES			4

#define		BARRIER				5

#define		CHECK_PL			6
#define		PER_CHECK			7

#define		N_STATS				8


#endif

typedef struct
{
	unsigned long long tm[N_STATS];
	unsigned long long n[N_STATS];
} server_stats_t;

#define time_event( ev_id, _tm )				\
({												\
	svts[__tid].stats.tm[(ev_id)] += (_tm);		\
	svts[__tid].stats.n[(ev_id)]++;				\
})


void server_stats_init();
void server_stats_deinit();
void server_stats_reset( server_stats_t* svt_stats );
void server_stats_dump( FILE* f_out );
void server_stats_print_map( FILE* f_out );

#endif /* SERVER_STATS_H_ */
