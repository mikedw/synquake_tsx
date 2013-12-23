#include "server.h"
#include "sgame/tm_worldmap.h"
#include "loadbal/load_balancer.h"

//burceam
#include "loadbal/grid.h"

#ifdef __SYNTHETIC__

#include "syn_server_traces.h"


//burceam: we'll need these from loadbal/load_balancer.c
extern grid_unit_t** grid;
extern grid_border_t** borders;

server_t sv = {0};
server_thread_t* svts;
int _SAVEPOINT_FQ = 0;

unsigned int rand_seed = 0;
__thread int __tid;

value_t server_thread_generate_quest_move( rect_t* r, int pl_id )
{
	value_t pl_dir = attribute_type_rand( &entity_types[ET_PLAYER]->attr_types[PL_DIR] );
	// guide player to quest
	rect_t* q_rect = &sv.quest_locations[sv.quest_id][pl_id % sv.wl_quest_spread];
	int direction = rect_quadrant( r, q_rect );
	
	//Jeff's R-r range thing: just these two variables, and the if statement below, before the switch
	//int range_R =800;
	//int range_r =300;

        //if ((range_R >= rect_distance (r, q_rect)) and (rect_distance(r, q_rect) >= range_r)) {
	switch( direction )
	{
		// if overlapping, player is on quest => do random move
		case 0:					break;
		// NW of quest, move down or right		// see constants DIR_* in geometry.h
		case 1:	pl_dir = my_rand() % 2 + 1;	break;
		// NE of quest, move down or left
		case 2:	pl_dir = my_rand() % 2 + 2;	break;
		// SW of quest, move up or right
		case 3:	pl_dir = my_rand() % 2;		break;
		// SE of quest, move up or left
		case 4:	pl_dir = (my_rand() % 2) * 3;	break;

		// over quest,  move down /or left
		case 5:	pl_dir = DIR_DOWN; /*my_rand() % 2 + 2;*/	break;
		// under quest, move up /or right
		case 6:	pl_dir = DIR_UP; /*my_rand() % 2;*/		break;
		// right of quest, move left /or up	
		case 7:	pl_dir = DIR_LEFT; /*(my_rand() % 2) * 3;*/	break;
		// left of quest, move right /or down
		case 8:	pl_dir = DIR_RIGHT; /*my_rand() % 2 + 1;*/	break;
		default:
			assert(!"Directioning player failed\n");	break;
	}
        //}

	return pl_dir;
}

void server_thread_generate_moves( server_thread_t* svt, int cycle )
{
	int i, j;
	tm_sv_client_t* cl;

        //change quest_times to have quests on/off
	int is_quest = ( sv.wl_quest_count &&
				cycle >= sv.quest_times[sv.quest_id] &&
				cycle  < sv.quest_times[sv.quest_id] + sv.wl_quest_duration
				);

	for( i = 0; i < sv.n_clients; i++ )
	{
		cl = sv.clients[i];
		if( cl->tid != svt->tid )	continue;

		tm_entity_movable_t* pl = cl->player;
		rect_t pl_r;
		rect_init4( &pl_r, pl->r.v1.x, pl->r.v1.y, pl->r.v2.x, pl->r.v2.y );
		value_t first_pl_dir = -1;
		for( j = 0; j < sv.n_multiple_actions; j++ )
		{
			if( !sv.randomized_actions )	cl->m_actions[j][M_ACT_ID] = sv.m_actions[j];
			else
			{
				int k, aux = rand_n( 100 );
				for( k = 0; k < n_actions; k++ )
					if( aux < sv.m_actions_ratios[k] )	break;
				assert( k < n_actions );
				cl->m_actions[j][M_ACT_ID] = k;
			}

			if( cl->m_actions[j][M_ACT_ID] == AC_MOVE )
			{
			        //burceam: so, if I understand this correctly, we give random direction and speed
			        //to players; then a bit below, (if( is_quest && ( (act_index % 2) != 0 ) )
			        //we generate a quest move, which will supersede the randomly-establised direction.
				cl->m_actions[j][M_ACT_SPD] = attribute_type_rand( &entity_types[ET_PLAYER]->attr_types[PL_SPEED] );
				cl->m_actions[j][M_ACT_DIR] = attribute_type_rand( &entity_types[ET_PLAYER]->attr_types[PL_DIR] );

				int act_index = cycle * sv.n_multiple_actions + j;
				
				if( is_quest && ( (act_index % 2) == 0 ) )
				{
					cl->m_actions[j][M_ACT_SPD] = cl->m_actions[j][M_ACT_SPD] / 4;
					if( entity_types[ET_PLAYER]->attr_types[PL_SPEED].min > cl->m_actions[j][M_ACT_SPD] )
						cl->m_actions[j][M_ACT_SPD] = entity_types[ET_PLAYER]->attr_types[PL_SPEED].min;
				}
				
				if( is_quest && ( (act_index % 2) != 0 ) )
					cl->m_actions[j][M_ACT_DIR] = server_thread_generate_quest_move( &pl_r, pl->ent_id );

				if( sv.straight_move )
				{
					if( first_pl_dir == -1 )	
					   first_pl_dir = cl->m_actions[j][M_ACT_DIR];
					else						
					   cl->m_actions[j][M_ACT_DIR] = first_pl_dir;
				}

				vect_t move_v;
				vect_scale( &dirs[cl->m_actions[j][M_ACT_DIR]], cl->m_actions[j][M_ACT_SPD], &move_v );
				vect_add( &pl_r.v1, &move_v, &pl_r.v1 );
				vect_add( &pl_r.v2, &move_v, &pl_r.v2 );
				// TO DO: change to allow multiple move records in file
				//server_register_player_move( pl, i );

				#ifdef PRINT_MOVES
				printf( "cl:%3d cycle:%3d j: %d ;   ", cl->cl_id, cycle, j );
				printf( "pos: %3d,%3d - last_dist %d ;  ", (coord_t)pl->r.v1.x, (coord_t)pl->r.v1.y, cl->last_dist );
				printf( "act: %5s spd: %d dir: %5s\n", action_names[ cl->m_actions[j][M_ACT_ID] ],
					cl->m_actions[j][M_ACT_SPD], dir_names[ cl->m_actions[j][M_ACT_DIR] ] );
				#endif
			}
		}
	}
}



void server_thread_preprocess_workload( server_thread_t* svt, int cycle )
{
#ifndef RUN_TRACE_ALL_MOVES
	server_thread_generate_moves( svt, cycle );
#else
	server_thread_replay_moves( svt, cycle );
#endif
}



extern __thread    int p_thread_id;

void server_thread_process_workload( server_thread_t* svt, int cycle )
{
	tm_sv_client_t* cl;
	int i;

	log_info2( "%x - process_workload  --  cycle: %d\n\n\n\n\n", p_thread_id, cycle );

	for( i = 0; i < sv.n_clients; i++ )
	{
		cl = sv.clients[i];
		if( cl->tid == svt->tid )
			tm_server_act_cl_synthetic( cl->cl_id, cycle );
	}
}

void server_thread_admin( int cycle )
{
	if( sv.wl_quest_count &&
			cycle == (sv.quest_times[sv.quest_id] + sv.wl_quest_duration) &&
			sv.quest_id < sv.wl_quest_count - 1 )
		sv.quest_id++;

	if( sv.print_progress && (cycle % sv.print_progress) == 0 )
	{
		printf( "%d ", cycle / sv.print_progress );
		if( cycle + sv.print_progress >= sv.wl_cycles )		printf( "\n" );
		fflush( stdout );
	}

	if( cycle % 10 >= 0 )		loadb_balance( cycle );
}

void* server_thread_run( void* arg )
{
	int cycle;
	unsigned long long now_0 = 0, now_1 = 0, now_2 = get_c();
	server_thread_t* svt = (server_thread_t*) arg;
	__tid = svt->tid;

	thread_set_affinity( __tid );
	thread_set_local_id( __tid );

	log_info( "Starting workload...\n" );


	for( cycle = 0; cycle < sv.wl_cycles && now_2 < sv.wl_stop; cycle++ )
	{
		server_thread_preprocess_workload( svt, cycle );

		barrier_wait( &sv.barrier );					
		now_0 = get_c();

		server_thread_process_workload( svt, cycle );	
		now_1 = get_c();

		barrier_wait( &sv.barrier );					
		now_2 = get_c();

		time_event( PROCESS, (now_1 - now_0) );
		time_event( BARRIER, (now_2 - now_1) );
		time_event( TOTAL, (now_2 - now_0) );

		if( svt->tid == 0 )	
		   server_thread_admin( cycle );

		barrier_wait(&sv.barrier);
	}

	barrier_wait( &sv.barrier );
	return 0;
}


void server_thread_init( server_thread_t* svt, int _tid )
{
	assert( svt && _tid >= 0 );
	svt->tid = _tid;
	svt->done = 0;

	server_stats_reset( &svt->stats );
	svt->action_contexts = (action_context_t*) calloc( sv.n_multiple_actions, sizeof( action_context_t ) );
	svt->sps = (priv_sp_t*) calloc( sv.n_multiple_actions * (1 << (tm_wm.depth+1)), sizeof( priv_sp_t ) );
}

void server_generate_clients( int count )
{
	int i;
	unsigned char s[4] = { 192, 168, 1, 1 };
	char pname[MAX_NAME_LEN];
	tm_entity_movable_t* new_pl;

	log_info( "Generating clients... \n" );

	for( i = 0; i < count; i++ )
	{
		sprintf( pname, "Player_%u.%u.%u.%u", s[0], s[1], s[2], s[3] );
		if( s[3] == 255 )
		{
			s[2]++;
			s[3] = 1;
		}
		else s[3]++;

		new_pl = tm_server_add_cl( pname );
		if( !new_pl )	break;

		log_info1( "[I] %s generated.\n", pname );
		server_register_player( new_pl );
	}

	log_info1( "[I] %d players generated.\n", i );
}


void server_init_quests()
{
	sv.quest_id = 0;

	if( sv.wl_quest_count == 0 )		
	   return;

        //burceam: divided this by 2 so that for each quest session, we have quests for the
        //first half of the session, and no quests for the second half
	//sv.wl_quest_duration = sv.wl_cycles / sv.wl_quest_count / 2;
	sv.wl_quest_duration = sv.wl_cycles / sv.wl_quest_count;


	assert( sv.wl_quest_duration > 0 );

	sv.quest_times = (int*) malloc( sv.wl_quest_count * sizeof(int) );				
	assert( sv.quest_times );
	sv.quest_locations = (rect_t**) malloc( sv.wl_quest_count * sizeof(rect_t*) );	
	assert( sv.quest_locations );

	int i;
	for( i = 0; i < sv.wl_quest_count; i++ )
	{
		sv.quest_locations[i] = (rect_t*) malloc( sv.wl_quest_spread * sizeof(rect_t) );
		assert( sv.quest_locations[i] );
		//burceam: above I halved the quest duration, so quests will only last half 
		//as long; but I still want them to start at the same times, so I'm 
		//multiplying this one by two (because now the quest duration is half what it was).
		//I'm also adding sw.wl_quest_duration because I want quest sessions to be 
		//like this: first half = no quests; second half = quests; and this repeats
		//for each quest session, so we have basically quest cycles.
		//sv.quest_times[i] = sv.wl_quest_duration * i * 2 + sv.wl_quest_duration;
		sv.quest_times[i] = sv.wl_quest_duration * i;

	}

#ifndef RUN_QUESTS
	server_generate_quests_random();
#else
	server_load_quests();
#endif

}

void server_generate_quests_random()
{
	int i, j;
	for( i = 0; i < sv.wl_quest_count; i++ )
	{
		for( j = 0; j < sv.wl_quest_spread; j++ )
		{
			sv.quest_locations[i][j].v1.x = rand() % ((int) tm_wm.size.x - 2) + 1; // borders are walls
			sv.quest_locations[i][j].v1.y = rand() % ((int) tm_wm.size.y - 2) + 1; // place quest inside map
			sv.quest_locations[i][j].v2.x = sv.quest_locations[i][j].v1.x;
			sv.quest_locations[i][j].v2.y = sv.quest_locations[i][j].v1.y;

			server_register_quest( i, j );
		}
	}
}

void server_init_multiple_actions()
{
	char nm[MAX_FILE_READ_BUFFER];
	FILE* actions_f = fopen( sv.m_actions_file, "r" );
	if( !actions_f )
	{
		sv.n_multiple_actions = 1;
		sv.randomized_actions = 0;
		sv.move_fail_stop = 1;
		sv.straight_move = 1;

		sv.m_actions = (int*) malloc( sv.n_multiple_actions * sizeof(int) );
		sv.m_actions[0] = AC_MOVE;

		printf( "Using default configuration for actions: 1 Move per cycle (move_fail_stop=true)  (straight_move=true).\n" );
		return;
	}

	char str[64];
	int line = 0, cnt = 0, i, ratio;
	while ( fgets(nm, MAX_FILE_READ_BUFFER, actions_f) != NULL )
	{
		if ( nm[0] == 10  || nm[0] == 13 )	continue;
		if ( nm[0] == '#' || nm[0] == '[' )	continue;

		if( line == 0 )
		{
			if( sscanf( nm, "%d %d %d %d", &sv.n_multiple_actions, &sv.randomized_actions, &sv.move_fail_stop, &sv.straight_move ) != 4 )
			{		printf( "%s: Ignoring line \"%s\"", sv.m_actions_file, nm );continue;	}
			assert( sv.n_multiple_actions > 0 && sv.randomized_actions >= 0 && sv.randomized_actions <= 1 );
			assert( sv.move_fail_stop >= 0 && sv.move_fail_stop <= 1 );
			assert( sv.straight_move >= 0 && sv.straight_move <= 1 );
			if( !sv.randomized_actions )
				sv.m_actions = (int*) calloc( sv.n_multiple_actions, sizeof(int) );
			else
				sv.m_actions_ratios = (int*) calloc( n_actions, sizeof(int) );
		}
		else
		{
			if( !sv.randomized_actions )
			{
				if( sscanf( nm, "%s", str ) != 1 )
				{		printf( "%s: Ignoring line \"%s\"", sv.m_actions_file, nm );continue;	}
				for( i = 0; i < n_actions; i++ )
					if( !strcmp( str, action_names[i] ) )	break;
				assert( i < n_actions );
				sv.m_actions[ cnt ] = i;
				cnt++;
				if( cnt == sv.n_multiple_actions )	break;
			}
			else
			{
				if( sscanf( nm, "%s %d", str, &ratio ) != 2 )
				{		printf( "%s: Ignoring line \"%s\"\n", sv.m_actions_file, nm );continue;	}
				assert( ratio >=0 && ratio <= 100 );
				for( i = 0; i < n_actions; i++ )
					if( !strcmp( str, action_names[i] ) )	break;
				assert( i < n_actions );
				sv.m_actions_ratios[ i ] = ratio;
				cnt++;
				if( cnt == n_actions )	break;
			}
		}
		line++;
	}

	if( !sv.randomized_actions )
	{
		assert( cnt == sv.n_multiple_actions );
	}
	else
	{
		int rez = 0;
		for( i = 0; i < n_actions; i++ )	rez += sv.m_actions_ratios[ i ];
		assert( rez == 100 );
		for( i = 1; i < n_actions; i++ )	sv.m_actions_ratios[ i ] += sv.m_actions_ratios[ i-1 ];
	}
}

void server_generate_workload()
{
	server_generate_clients( sv.wl_client_count );

#ifdef RUN_TRACE_ALL_MOVES
	server_load_player_moves();
#endif
}



void server_init( char* conf_file_name, int map_szx, int map_szy, int tdepth, int speed_min, int speed_max,
					int apple_map_ratio, int apple_pl_ratio, int wall_map_ratio, int wall_pl_ratio, char* balname )
{
	int i;

	rand_seed = (unsigned int) time( NULL );
	srand( (unsigned int) time( NULL ) );

	conf_t* c = conf_create();	assert(c);
	conf_parse_file( c, conf_file_name );

	/* server */
	sv.port = conf_get_int( c, "server.port" );
	//sv.num_threads		= conf_get_int( c, "server.number_of_threads" );
	sv.update_interval = conf_get_int( c, "server.update_interval" );
	sv.stats_interval = conf_get_int( c, "server.stats_interval" );

	assert( sv.port > 1023 );
	assert( sv.num_threads > 0 && sv.num_threads <= MAX_THREADS );
	assert( sv.update_interval > 0 );
	assert( sv.stats_interval > 0 );

	/* quests */
	sv.quest_between = conf_get_int( c, "server.quest_between" );
	sv.quest_length = conf_get_int( c, "server.quest_length" );

	assert( sv.quest_between > 0 && sv.quest_length > 0 );
	assert( sv.quest_between > sv.update_interval && sv.quest_length > sv.update_interval );

	/* initialize clients array */
	sv.n_clients = 0;
	sv.clients = new tm_p_tm_sv_client_t[MAX_ENTITIES];
	assert( sv.clients );

	/* initialize world */
	server_traces_init();
	actions_init( c );
	server_init_multiple_actions();

	entity_types_init( c );
	// override the speed settings read from the config file
	entity_types[ ET_PLAYER ]->attr_types[ PL_SPEED ].min = speed_min;
	entity_types[ ET_PLAYER ]->attr_types[ PL_SPEED ].max = speed_max;
	// override the ratio settings read from the config file
	entity_types[ ET_APPLE ]->ratio    = apple_map_ratio;
	entity_types[ ET_APPLE ]->pl_ratio = apple_pl_ratio;
	entity_types[ ET_WALL ]->ratio    = wall_map_ratio;
	entity_types[ ET_WALL ]->pl_ratio = wall_pl_ratio;

	tm_worldmap_init( c, map_szx, map_szy, tdepth );
	server_init_quests();

	tm_worldmap_generate();
	tm_worldmap_is_valid();
/*
        //burceam: if heuristic1 is turned on, allocate structures for feedback/info
        sv.h1_dbg_num_ent = NULL;
        sv.h1_dbg_num_set = NULL;
        //temporarily turned it always on, for potential study; 
        //if (sv.heuristic1 != 0) 
        {
           sv.h1_dbg_num_ent = (int *) malloc (sv.wl_cycles * sizeof (int));
           sv.h1_dbg_num_set = (int *) malloc (sv.wl_cycles * sizeof (int));
           assert ((sv.h1_dbg_num_ent != NULL) && (sv.h1_dbg_num_set != NULL));
           int i;
           for (i = 0; i < sv.wl_cycles; i++) {
              sv.h1_dbg_num_ent [i] = 0;
              sv.h1_dbg_num_set [i] = 0;
           }
        }
        
        //burceam: for heuristic 2, hopefully temporary
        //note that this may need to be resized at some point: new players may join,
        //and existing players can drop out.
        //change_grain_to_entity_for_h3 is obviously meant to be used by heuristic h3.
        {
           sv.change_grain_to_entity = (unsigned char *) malloc (sv.wl_client_count * sizeof (unsigned char));
           sv.change_grain_to_entity_for_h3 = (unsigned char *) malloc (sv.wl_client_count * sizeof (unsigned char));
           int i;
           for (i = 0; i < sv.wl_client_count; i ++) {
              sv.change_grain_to_entity [i] = 0;
              sv.change_grain_to_entity_for_h3 [i] = 0;
           }
        }
        
        //burceam: this field is used for debugging
        sv.num_invocations_collision_detection [0] = 0;
        sv.num_invocations_collision_detection [1] = 0;
        sv.num_invocations_collision_detection [2] = 0;
        sv.num_invocations_collision_detection [3] = 0;
        
        //burceam: create and initialize the list of area node h_meta pointers.
        //this MUST be done after tm_worldmap_init(), where I think the area node tree is created
        //and initialized. We need the depth here.
        {
           int i, num_area_nodes = 1;
           //IMPORTANT: the _actual_ depth of the tree is tdepth+1 !! 
           //root is level "depth", and they keep building until level reaches 0, including for 0!
           //(nodes at level 0 are the leaves). So for depth=8 entered on cmd line, we really have 9 levels.
           num_area_nodes = 1 << (tm_wm.depth + 1);
           sv.hmeta_list = (ptr_t *) malloc (num_area_nodes * sizeof (ptr_t));
           for (i = 0; i < num_area_nodes; i++) 
              sv.hmeta_list [i] = NULL;
        }
*/
	/* initialize synthetic workload */
	server_generate_workload();
	tm_worldmap_is_valid();

	loadb_init( balname );

	/* initialize syncronization & server threads */
	barrier_init( &sv.barrier, sv.num_threads );
	server_stats_init();
	sv.done = 0;

	svts = (server_thread_t*) malloc( sv.num_threads * sizeof(server_thread_t) );
	for( i = 0; i < sv.num_threads; ++i )		server_thread_init( &svts[i], i );

	log_info( "[I] Server init done.\n" );
}

void server_deinit()
{
	sv.done = 1;

	int i;
	for( i = 1; i < sv.num_threads; ++i )	thread_join( svts[i].thread );

	tm_worldmap_is_valid();

	server_stats_dump( sv.stats_f );
	server_stats_dump( stdout );

	server_stats_deinit();
	server_traces_deinit();

	log_info( "[I] Server deinit done.\n" );
}


void decode_entity_ratio( int ent_map_ratio, int ent_pl_ratio, int* r_ent_map_ratio, int* r_ent_pl_ratio, int mapx, int mapy, int depth )
{
	long long int aux;
	if( ent_map_ratio < 0 )
	{
		ent_pl_ratio = - ent_map_ratio;
		if( sv.wl_quest_count )
			aux = ( long long int ) sv.wl_client_count * ent_pl_ratio * ( 1 << depth ) * 1000 / ( mapx * mapy * 4 * sv.wl_quest_spread );
		else
			aux = ( long long int ) sv.wl_client_count * ent_pl_ratio * 1000 / ( mapx * mapy );
		ent_map_ratio = aux;
	}
	else
	{
		if( sv.wl_quest_count )
			aux = ( long long int) ent_map_ratio * mapx * mapy * 4 * sv.wl_quest_spread / ( sv.wl_client_count * ( 1 << depth ) * 1000 );
		else
			aux = ( long long int) ent_map_ratio * mapx * mapy / ( sv.wl_client_count * 1000 );
		ent_pl_ratio = aux;
	}

	*r_ent_map_ratio = ent_map_ratio;
	*r_ent_pl_ratio = ent_pl_ratio;
}


int main( int argc, char *argv[] )
{
	if( argc < 15 )
		pexit1( "Usage: %s N_THREADS N_CLIENTS N_CYCLES N_QUESTS QSpread QuestsFile MapSizeX MapSizeY "
			"TreeDepth SpeedMax AppleRatio WallRatio BalanceType ActionsFile [print] [timeout]\n"
			"For more help check readme.txt .\n", argv[0] );

	sv.num_threads = atoi( argv[1] );
	sv.wl_client_count = atoi( argv[2] );
	sv.wl_cycles = atoi( argv[3] );
	sv.wl_quest_count = atoi( argv[4] );
	sv.wl_quest_spread = atoi( argv[5] );
	sv.wl_quest_file = argv[6];
	assert( !sv.wl_quest_count || sv.wl_quest_spread );

	int mapx = atoi( argv[7] );
	int mapy = atoi( argv[8] );
	int depth = atoi( argv[9] );
	int speed_max = atoi( argv[10] );
	int apple_map_ratio = atoi( argv[11] );
	int wall_map_ratio =  atoi( argv[12] );
	char* balance = argv[13];
	sv.m_actions_file = argv[14];

	sv.wl_proportional_quests = 0;
	if( sv.wl_quest_count < 0 )
	{
		sv.wl_quest_count = -sv.wl_quest_count;
		sv.wl_proportional_quests = 1;
	}

	int apple_pl_ratio;
	int wall_pl_ratio;
	
	// mike: ignore warning, apple_pl_ratio assigned in the function
	decode_entity_ratio( apple_map_ratio, apple_pl_ratio, &apple_map_ratio, &apple_pl_ratio, mapx, mapy, depth );
	// mike: ignore warning, wall_pl_ratio assigned in the function
	decode_entity_ratio( wall_map_ratio, wall_pl_ratio, &wall_map_ratio, &wall_pl_ratio, mapx, mapy, depth );

	if( argc >= 16 )
	{
		int print_v = atoi( argv[15] );
		if( print_v < 0 )	sv.print_pls = 1;
		print_v = abs( print_v );
		assert( print_v >= 0 && print_v <= 100 );

		sv.print_grid = (sv.wl_cycles * print_v ) / 100;
		if( print_v && sv.print_grid == 0 )		sv.print_grid = 1;

		if( sv.print_grid == 0 || sv.print_grid == sv.wl_cycles )
			sv.print_progress = ( sv.wl_cycles < 10 ) ? 1 : (sv.wl_cycles/10);
	}

	if( argc >= 17 )		sv.wl_timeout = t_to_c( atoi( argv[16] ) );
	else				sv.wl_timeout = t_to_c( 3600 );

	server_init( (char*)"./config/default.cfg", mapx, mapy, depth, speed_max/4, speed_max, apple_map_ratio, apple_pl_ratio,
								wall_map_ratio, wall_pl_ratio, balance );

	sv.wl_stop = get_c() + sv.wl_timeout;

	int i;
	for( i = 1; i < sv.num_threads; ++i )
	{
		svts[i].thread = thread_create( server_thread_run, (void*)&svts[i] );
		assert( svts[i].thread );
	}

	server_thread_run( &svts[0] );
	server_deinit();

	return 0;
}

#endif
