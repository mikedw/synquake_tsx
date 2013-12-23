#ifndef __SERVER_H
#define __SERVER_H

#define MAX_THREADS		16

#define __SYNTHETIC__


#include "server_stats.h"

#ifndef __SYNTHETIC__
#include "../comm/comm.h"

#include "server_cl.h"
#else
#include "syn_server_cl.h"
#endif

#include "sgame/tm_area_node.h"

typedef struct _priv_sp_t
{
	volatile elem_t*     _pos;
	volatile int         _acc;
	volatile int         _i_act;
	volatile int         n;
} priv_sp_t;

typedef struct _action_context_t
{
	int         a_id;
	int         etypes;
	rect_t      act_r;
	tm_parea_node_set_t*   pan_set_leaves;
	tm_parea_node_set_t*   pan_set_parents;
	int final_acc;
} action_context_t;

typedef struct _server_thread_t
{
	int		tid;
	int		done;

	#ifndef __SYNTHETIC__
	sock_t	s;
	#endif
	thread_t*	thread;
	server_stats_t stats;

	action_context_t*  action_contexts;
	priv_sp_t* sps;
} server_thread_t;

extern server_thread_t*	svts;
extern int _SAVEPOINT_FQ;

extern __thread int __tid;

void server_thread_init( server_thread_t* svt, int _tid );


typedef tm_type<tm_sv_client_t*>	tm_p_tm_sv_client_t;

typedef struct
{
	tm_int			n_clients;
	tm_p_tm_sv_client_t*	clients;

#ifdef __SYNTHETIC__
	int wl_cycles;
	int wl_client_count;
	unsigned long long wl_timeout;
	unsigned long long wl_stop;
#endif

	/* server */
	int port;		/* server's port */
	int num_threads;	/* number of threads */
	int update_interval;	/* time between two consecutive client updates */
	int stats_interval;

	/* MULTIPLE ACTIONS PER CYCLE */
	char* m_actions_file;
	int n_multiple_actions;
	int* m_actions;
	int randomized_actions;
	int* m_actions_ratios;
	int move_fail_stop;
	int straight_move;


	FILE* stats_f;
	FILE* f_players;
	unsigned long long next_stats_dump;
	int print_grid;
	int print_pls;
	int print_progress;

	/* execution traces */
	FILE* f_trace_players;
	FILE* f_trace_quests;
	FILE* f_trace_objects;
	FILE* f_trace_moves;
	FILE* f_trace_check;

	/* quests */
	int quest_between;	/* time between two consecutive quests */
	int quest_length;
	unsigned long long start_quest, stop_quest;
	char quest_state;
	rect_t quest_loc;
	vect_t quest_sz;

	int wl_quest_count;             //burceam: the number of quest sessions, read from cmd line
	int wl_quest_spread;            //burceam: the number of quests in a quest session, read from cmd line
	int wl_quest_duration;          //burceam: the duration of each quest session; by default, wl_cycles/wl_quest_count
	char* wl_quest_file;
	int* quest_times;               //burceam: the starting time of each quest
	rect_t** quest_locations;
	int quest_id;			// managed by thread 0 in server_thread_admin()
	int wl_proportional_quests;

	/* load-balance */
	char*	balance_name;		/* name of the load balancing algorithm */
	int	balance_type;
	double overloaded_level;	/* overloaded and light server level */
	double light_level;
	int balance_interval;		/* time between two consecutive load balances */
	int last_balance;

	barrier_t barrier;

	buffer_t map_buff;
	streamer_t* map_st;

        //burceam: heuristic 1 for deciding lock granularity; default value is zero (heuristic turned off)
        int heuristic1;
        int * h1_dbg_num_ent; //number of players with entity granularity
        int * h1_dbg_num_set; //number of players with set granularity
        //burceam: used for heuristic 2, temporary
        //array with 1 entry per player; a player's entry is set to 1 when it has a conflict
        //with a player owned by another thread (at which point both players' entries are set to 1)
        unsigned char * change_grain_to_entity; 
        unsigned char * change_grain_to_entity_for_h3; 
        
        //burceam: if 0, heuristic h2 is disabled, otherwise it is enabled.
        int heuristic2;
        //burceam: list of h_meta pointers of all area nodes
        ptr_t * hmeta_list;
        //burceam: temp, debug helpful
        int num_invocations_collision_detection[4];
        //burceam: if 0, heuristic h3 si disabled, otherwise it is enabled.
        int heuristic3;

	int done;
} server_t;

extern server_t sv;



#endif
