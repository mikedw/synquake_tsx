#include "sgame.h"

extern __thread    int p_thread_id;

#ifdef DELAY_OPS
#define delay()		\
({			\
	int n;		\
	for( n = 0; n < DELAY_OPS; n++ )    asm volatile("nop");	\
})
#else
#define delay()
#endif


int tm_game_action_colision_detection0( tm_entity_movable_t* pl, entity_t* ent, rect_t* range, int move_dist )
{
	time_event( CHECK_PL, 0 );	
	delay();

	if( !rect_is_overlapping( range, &ent->r ) )		
		return move_dist;
	if( tm_entity_movable_is_eq0( pl, ent ) )			
		return move_dist;

	int dist = tm_rect_distance_10( &pl->r, &ent->r );		
	assert( dist != -1 );
	if( dist < move_dist )			
		move_dist = dist;
	return move_dist;
}

int tm_game_action_colision_detection1( tm_entity_movable_t* pl, tm_entity_stationary_t* ent, rect_t* range, int move_dist )
{
	time_event( CHECK_PL, 0 );	
	delay();

	if( !rect_is_overlapping( range, &ent->r ) )		
		return move_dist;
	if( tm_entity_movable_is_eq1( pl, ent ) )					
		return move_dist;

	int dist = tm_rect_distance_10( &pl->r, &ent->r );		
	assert( dist != -1 );
	if( dist < move_dist )			
		move_dist = dist;
	return move_dist;
}

int tm_game_action_colision_detection2( tm_entity_movable_t* pl, tm_entity_movable_t* ent, rect_t* range, int move_dist )
{
	time_event( CHECK_PL, 0 );	
	delay();
	log_info3( "%x - collision detection pl: %x  ent: %x\n", p_thread_id, (unsigned)pl, (unsigned) ent );

        //burceam: debugging
        //this following line causes a slowdown of 3x, from 120s to 360s running time. Verified multiple times.
        //if (sv.heuristic2 != 0) {
        //   sv.num_invocations_collision_detection [((tm_sv_client_t*)sv.clients [pl->ent_id])->tid]++;
        //}

	if( !tm_rect_is_overlapping_01( range, &ent->r ) )	
		return move_dist;
	if( tm_entity_movable_is_eq2( pl, ent ) )					
		return move_dist;

	int dist = tm_rect_distance( &pl->r, &ent->r );		
	assert( dist != -1 );
	//burceam: added stuff for heuristic 2; may be temporary
	if( dist < move_dist ) {			
		move_dist = dist;
		
		if (sv.heuristic2 != 0) {
		   //maybe add a check first - only set them to 1 if they are not 1 already
		   //also, verify that this condition actually works properly
		   if (((tm_sv_client_t*)sv.clients [pl->ent_id])->tid != ((tm_sv_client_t*)sv.clients [ent->ent_id])->tid) {
		      sv.change_grain_to_entity [pl->ent_id] = 1;
		      sv.change_grain_to_entity [ent->ent_id] = 1;
		   }
		}
		
	}
	return move_dist;
}



int tm_game_action_eat_entity0( entity_t* ent, rect_t* range )
{
	time_event( CHECK_PL, 0 );	
	delay();

	if( !rect_is_overlapping( range, &ent->r ) )	
	   return 0;

	int eat_rez = ent->attrs[AP_FOOD] > entity_types[ET_APPLE]->attr_types[AP_FOOD].min;
	assert( eat_rez == 0 || eat_rez == 1 );
	if( eat_rez )
		entity_set_attr( ent, AP_FOOD, ent->attrs[AP_FOOD] - eat_rez );

	return eat_rez;
}

int tm_game_action_eat_entity1( tm_entity_stationary_t* ent, rect_t* range )
{
	time_event( CHECK_PL, 0 );	
	delay();

	if( !rect_is_overlapping( range, &ent->r ) )	
	   return 0;

	int eat_rez = ent->attrs[AP_FOOD] > entity_types[ET_APPLE]->attr_types[AP_FOOD].min;
	assert( eat_rez == 0 || eat_rez == 1 );
	if( eat_rez )
		tm_entity_stationary_set_attr( ent, AP_FOOD, ent->attrs[AP_FOOD] - eat_rez );

	return eat_rez;
}

int tm_game_action_eat_entity2( tm_entity_movable_t* ent, rect_t* range )
{
	time_event( CHECK_PL, 0 );	
	delay();

	if( !tm_rect_is_overlapping_01( range, &ent->r ) )	
	   return 0;

	int eat_rez = ent->attrs[AP_FOOD] > entity_types[ET_APPLE]->attr_types[AP_FOOD].min;
	assert( eat_rez == 0 || eat_rez == 1 );
	if( eat_rez )
		tm_entity_movable_set_attr( ent, AP_FOOD, ent->attrs[AP_FOOD] - eat_rez );

	return eat_rez;
}


int tm_game_action_attack_entity0( tm_entity_movable_t* pl, entity_t* ent, rect_t* range )
{
	time_event( CHECK_PL, 0 );	
	delay();

	if( !rect_is_overlapping( range, &ent->r ) )		
	   return 0;
	if( tm_entity_movable_is_eq0( pl, ent ) )					
	   return 0;

	int attack_rez = pl->attrs[PL_LIFE] - ent->attrs[PL_LIFE];
	if( attack_rez > 0 )			
	   attack_rez = 1;
	if( attack_rez < 0 )			
	   attack_rez = -1;

	if( attack_rez )
		entity_set_attr( ent, PL_LIFE, ent->attrs[PL_LIFE] - attack_rez );
	// signals that the player has been attacked, used for rendering
	if( attack_rez > 0 )	
	   entity_set_attr( ent, PL_HIT, 1 );

	return attack_rez;
}

int tm_game_action_attack_entity1( tm_entity_movable_t* pl, tm_entity_stationary_t* ent, rect_t* range )
{
	time_event( CHECK_PL, 0 );	
	delay();

	if( !rect_is_overlapping( range, &ent->r ) )		
	   return 0;
	if( tm_entity_movable_is_eq1( pl, ent ) )					
	   return 0;

	int attack_rez = pl->attrs[PL_LIFE] - ent->attrs[PL_LIFE];
	if( attack_rez > 0 )			
	   attack_rez = 1;
	if( attack_rez < 0 )			
	   attack_rez = -1;

	if( attack_rez )
		tm_entity_stationary_set_attr( ent, PL_LIFE, ent->attrs[PL_LIFE] - attack_rez );
	// signals that the player has been attacked, used for rendering
	if( attack_rez > 0 )	
	   tm_entity_stationary_set_attr( ent, PL_HIT, 1 );

	return attack_rez;
}

int tm_game_action_attack_entity2( tm_entity_movable_t* pl, tm_entity_movable_t* ent, rect_t* range )
{
	time_event( CHECK_PL, 0 );	
	delay();

	if( !tm_rect_is_overlapping_01( range, &ent->r ) )	
	   return 0;
	if( tm_entity_movable_is_eq2( pl, ent ) )					
	   return 0;

	int attack_rez = pl->attrs[PL_LIFE] - ent->attrs[PL_LIFE];
	if( attack_rez > 0 )			
	   attack_rez = 1;
	if( attack_rez < 0 )			
	   attack_rez = -1;

	if( attack_rez )
		tm_entity_movable_set_attr( ent, PL_LIFE, ent->attrs[PL_LIFE] - attack_rez );
	// signals that the player has been attacked, used for rendering
	if( attack_rez > 0 )	
	   tm_entity_movable_set_attr( ent, PL_HIT, 1 );

	return attack_rez;
}



int tm_game_action_node( int a_id, tm_entity_movable_t* pl, rect_t* range, int etypes, tm_area_node_t* an, int acc )
{
	int i;
	elem_t*     pos;
	tm_elem_t*  tm_pos;

	log_info3( "%x - game_action_node_start pl: %x  an: %x\n", p_thread_id, (unsigned)pl, (unsigned) &an->ent2_sets[pl->ent_type].h_meta );

	for( i = 0; i < n_entity_types; i++ )
	{
		if( ((1 << i) & etypes) == 0 )	
		   continue;

		if( entity_types[i]->protect == PROTECT_NONE )
		entity_set_for_each( pos, &an->ent0_sets[i] )
		{
			entity_t* ent = (entity_t*) pos;

			assert( rect_is_contained( &ent->r, &an->loc ) );
			assert( tm_area_node_is_leaf( an ) || rect_is_overlapping( &an->split, &ent->r ) );

			if( a_id == AC_MOVE )	acc = tm_game_action_colision_detection0( pl, ent, range, acc );
			if( a_id == AC_EAT )	acc += tm_game_action_eat_entity0( ent, range );
			if( a_id == AC_ATTACK )	acc += tm_game_action_attack_entity0( pl, ent, range );

			#ifndef CHECK_ALL
			if( a_id == AC_MOVE && acc == 0 )	
			   break;
			#endif
		}

		if( entity_types[i]->protect == PROTECT_ATTRS )
		entity_set_for_each( pos, &an->ent1_sets[i] )
		{
			tm_entity_stationary_t* ent = (tm_entity_stationary_t*) pos;

			assert( rect_is_contained( &ent->r, &an->loc ) );
			assert( tm_area_node_is_leaf( an ) || rect_is_overlapping( &an->split, &ent->r ) );

			if( a_id == AC_MOVE )	acc = tm_game_action_colision_detection1( pl, ent, range, acc );
			if( a_id == AC_EAT )	acc += tm_game_action_eat_entity1( ent, range );
			if( a_id == AC_ATTACK )	acc += tm_game_action_attack_entity1( pl, ent, range );

			#ifndef CHECK_ALL
			if( a_id == AC_MOVE && acc == 0 )	
			   break;
			#endif
		}

		if( entity_types[i]->protect == PROTECT_ALL )
		tm_entity_set_for_each( tm_pos, &an->ent2_sets[i] )
		{
			tm_entity_movable_t* ent = (tm_entity_movable_t*) tm_pos;

			assert( tm_rect_is_contained_10( &ent->r, &an->loc ) );
			assert( tm_area_node_is_leaf( an ) || tm_rect_is_overlapping_01( &an->split, &ent->r ) );

			if( a_id == AC_MOVE )	acc = tm_game_action_colision_detection2( pl, ent, range, acc );
			if( a_id == AC_EAT )	acc += tm_game_action_eat_entity2( ent, range );
			if( a_id == AC_ATTACK )	acc += tm_game_action_attack_entity2( pl, ent, range );

			#ifndef CHECK_ALL
			if( a_id == AC_MOVE && acc == 0 )	
			   break;
			#endif
		}

		#ifndef CHECK_ALL
		if( a_id == AC_MOVE && acc == 0 )	
		   break;
		#endif
	}

	log_info3( "%x - game_action_node_end pl: %x  an: %x\n", p_thread_id, (unsigned)pl, (unsigned) &an->ent2_sets[pl->ent_type].h_meta );

	return acc;
}

int tm_game_action_finalize( tm_entity_movable_t* pl, int a_id, int acc )
{
	log_info2( "%x - game_action_finalize_start pl: %x\n", p_thread_id, (unsigned)pl );

	if( a_id == AC_MOVE && acc != 0 )
	{
		value_t dir = pl->attrs[PL_DIR];
		int dist = tm_rect_distance_10( &pl->r, &tm_wm.map_walls[dir] );	
		assert( dist != -1 );
		if( dist < acc )		
		   acc = dist;
	}

	if( a_id == AC_MOVE )
	{
		vect_t move_v;
		vect_scale( &dirs[pl->attrs[PL_DIR]], acc, &move_v );
		tm_worldmap_move2( pl, &move_v );
	}
	if( a_id == AC_EAT && acc != 0 )
		tm_entity_movable_set_attr( pl, PL_LIFE, pl->attrs[PL_LIFE] + acc );
	if( a_id == AC_ATTACK && acc != 0 )
	{
		tm_entity_movable_set_attr( pl, PL_LIFE, pl->attrs[PL_LIFE] + acc );
		if( acc < 0 )		
		   tm_entity_movable_set_attr( pl, PL_HIT, 1 );
	}

	log_info2( "%x - game_action_finalize_end pl: %x\n", p_thread_id, (unsigned)pl );
	return acc;
}


#define		SAVE_ACTION_STATE()				\
({								\
	if( _SAVEPOINT_FQ )					\
	{							\
		svt->sps[ sp_i ]._pos = pos;			\
		svt->sps[ sp_i ]._acc = acc;			\
		svt->sps[ sp_i ]._i_act = i_act;		\
		sp_i++;						\
								\
		new_sp_i = SAVE_TRANSACTION( sp_i );		\
		if( new_sp_i )					\
		{						\
			sp_i = new_sp_i;			\
			pos = svt->sps[ sp_i-1 ]._pos;		\
			acc = svt->sps[ sp_i-1 ]._acc;		\
			i_act = svt->sps[ sp_i-1 ]._i_act;	\
			svt->sps[ sp_i-1 ].n++;			\
		}						\
	}							\
})


void tm_game_multiple_action( tm_sv_client_t* cl, int cycle )
{
	volatile elem_t * pos;
	volatile int i_act, acc;

	assert( cl && cl->player && cl->player->ent_type == ET_PLAYER );
	tm_entity_movable_t* pl = cl->player;

	volatile unsigned long long now_0, now_1, now_2, now_3, time_get_nodes;
	now_0=0, now_1=0, now_2=0, now_3=0, time_get_nodes=0;

	server_thread_t* svt = &svts[__tid];
	action_context_t* ctx = svt->action_contexts;

	log_info2( "%x - game_multiple_action_start pl: %x\n\n\n\n\n", p_thread_id, (unsigned)pl );
	now_0 = get_c();

	BEGIN_TRANSACTION();
	for( i_act = 0; i_act < sv.n_multiple_actions; i_act++ )
	{
		ctx[i_act].a_id = cl-> m_actions[i_act][M_ACT_ID];
		assert( ctx[i_act].a_id >= 0 && ctx[i_act].a_id < n_actions );

		log_info3( "%x - game_action_start pl: %x  i_act: %d\n\n\n", p_thread_id, (unsigned)pl, i_act );

		ctx[i_act].pan_set_leaves = NULL;
		ctx[i_act].pan_set_parents = NULL;

		acc = 0;
		if( ctx[i_act].a_id == AC_MOVE )
		{
			tm_entity_movable_set_attr( pl, PL_SPEED, cl-> m_actions[i_act][M_ACT_SPD] );
			tm_entity_movable_set_attr( pl, PL_DIR, cl-> m_actions[i_act][M_ACT_DIR] );
			//pl->attrs[PL_SPEED]._t = cl->m_actions[i_act][M_ACT_SPD];
			//pl->attrs[PL_DIR]._t = cl->m_actions[i_act][M_ACT_DIR];
			acc = action_ranges[AC_MOVE].front * pl->attrs[PL_SPEED];
		}

		ctx[i_act].etypes = game_action_etypes( ctx[i_act].a_id );
		ctx[i_act].act_r = tm_game_action_range( ctx[i_act].a_id, pl, &tm_wm.map_r );

		//       Process leaves
		//now_1 = get_c();
		ctx[i_act].pan_set_leaves = tm_worldmap_get_leaves( &ctx[i_act].act_r );
		//time_get_nodes += (get_c() - now_1);
		//if( now_1 >= sv.wl_stop )
		//   break;

		tm_parea_node_set_for_each( pos, ctx[i_act].pan_set_leaves )
		{
			tm_area_node_t* an = ((tm_parea_node_t*) pos)->an;
			acc = tm_game_action_node( ctx[i_act].a_id, pl, &ctx[i_act].act_r, ctx[i_act].etypes, an, acc );
			#ifndef CHECK_ALL
			if( ctx[i_act].a_id == AC_MOVE && acc == 0 )	
			   break;
			#endif
		}

		//       Process parents
		#ifndef CHECK_ALL
		if( !(ctx[i_act].a_id == AC_MOVE && acc == 0) )
		#endif
		{
			log_info3( "%x - game_action_parents pl: %x  a_id: %d\n\n\n", p_thread_id, (unsigned)pl, i_act );
			//now_1 = get_c();
			ctx[i_act].pan_set_parents = tm_worldmap_get_parents( &ctx[i_act].act_r );
			//time_get_nodes += (get_c() - now_1);

			tm_parea_node_set_for_each( pos, ctx[i_act].pan_set_parents )
			{
				tm_area_node_t* an = ((tm_parea_node_t*) pos)->an;
				acc = tm_game_action_node( ctx[i_act].a_id, pl, &ctx[i_act].act_r, ctx[i_act].etypes, an, acc );
				#ifndef CHECK_ALL
				if( ctx[i_act].a_id == AC_MOVE && acc == 0 )	
				   break;
				#endif
			}
		}

		acc = tm_game_action_finalize( pl, ctx[i_act].a_id, acc );

		log_info3( "%x - game_action_end pl: %x  i_act: %d\n\n\n", p_thread_id, (unsigned)pl, i_act );

		ctx[i_act].final_acc = acc;
		if( ctx[i_act].a_id == AC_MOVE && acc == 0 && sv.move_fail_stop )	
		   break;

	}

	END_TRANSACTION();


	now_2 = get_c();

	int i;
	for( i = 0; i <= i_act && i < sv.n_multiple_actions; i++ )
	{
		tm_parea_node_set_destroy( ctx[i].pan_set_leaves );
		if( ctx[i].pan_set_parents )
			tm_parea_node_set_destroy( ctx[i].pan_set_parents );
	}
	now_3 = get_c();

	cl->last_dist = 0;
	for( i = 0; i <= i_act && i < sv.n_multiple_actions; i++ )
		if( ctx[i].a_id == AC_MOVE )	cl->last_dist += ctx[i].final_acc;

	log_info2( "%x - game_multiple_action_end pl: %x\n\n\n\n\n", p_thread_id, (unsigned)pl );

	time_event( GET_NODES, time_get_nodes );
	time_event( ACTION, (now_2 - now_0 - time_get_nodes) );
	time_event( DEST_NODES, (now_3 - now_2) );
}


#define  SAVE_ONE_ACTION_STATE()				\
({								\
	if( _SAVEPOINT_FQ )					\
	{							\
		svt->sps[ sp_i ]._pos = pos;			\
		svt->sps[ sp_i ]._acc = acc;			\
		sp_i++;						\
								\
		new_sp_i = SAVE_TRANSACTION( sp_i );		\
		if( new_sp_i )					\
		{						\
			sp_i = new_sp_i;			\
			pos = svt->sps[ sp_i-1 ]._pos;		\
			acc = svt->sps[ sp_i-1 ]._acc;		\
			svt->sps[ sp_i-1 ].n++;			\
		}						\
	}							\
})

void tm_game_action( int a_id, tm_entity_movable_t* pl, int cycle )
{
	volatile unsigned long long now_0, now_1, now_2, now_3, now_tmp, time_get_nodes;
	now_0=0, now_1=0, now_2=0, now_3=0, now_tmp=0, time_get_nodes=0;
	assert( a_id >= 0 && a_id < n_actions && pl && pl->ent_type == ET_PLAYER );

	rect_t ar = tm_game_action_range( a_id, pl, &tm_wm.map_r );
	int etypes = game_action_etypes( a_id );

	volatile int acc = 0;
	if( a_id == AC_MOVE )
		acc = action_ranges[AC_MOVE].front * pl->attrs[PL_SPEED];

	volatile elem_t * pos;
	now_0 = get_c();

	tm_parea_node_set_t* pan_set_parents = NULL;
	tm_parea_node_set_t* pan_set_leaves = tm_worldmap_get_leaves( &ar );
	now_1 = get_c();

	BEGIN_TRANSACTION();

	//       Process leaves
	tm_parea_node_set_for_each( pos, pan_set_leaves )
	{
		tm_area_node_t* an = ((tm_parea_node_t*) pos)->an;
		acc = tm_game_action_node( a_id, pl, &ar, etypes, an, acc );
		#ifndef CHECK_ALL
		if( a_id == AC_MOVE && acc == 0 )	
		   break;
		#endif
	}

	//       Process parents
	#ifndef CHECK_ALL
	if( !(a_id == AC_MOVE && acc == 0) )
	#endif
	{
		now_tmp = get_c();
		pan_set_parents = tm_worldmap_get_parents( &ar );
		time_get_nodes += (get_c() - now_tmp);

		tm_parea_node_set_for_each( pos, pan_set_parents )
		{
			tm_area_node_t* an = ((tm_parea_node_t*) pos)->an;
			acc = tm_game_action_node( a_id, pl, &ar, etypes, an, acc );
			#ifndef CHECK_ALL
			if( a_id == AC_MOVE && acc == 0 )	
			   break;
			#endif
		}
	}
	acc = tm_game_action_finalize( pl, a_id, acc );

	END_TRANSACTION();
	now_2 = get_c();

	tm_parea_node_set_destroy( pan_set_leaves );
	if( pan_set_parents )
		tm_parea_node_set_destroy( pan_set_parents );
	now_3 = get_c();

	time_event( GET_NODES, (now_1 - now_0) + time_get_nodes );
	time_event( ACTION, (now_2 - now_1 - time_get_nodes) );
	time_event( DEST_NODES, (now_3 - now_2) );
}




/*			OBSOLETE

void tm_game_action_move( tm_entity_t* pl, tm_pentity_set_t* pe_set )
{
	int move_dist = action_ranges[AC_MOVE].front * pl->attrs[PL_SPEED];

	// perform collision detection against all the entities in the range of the MOVE action
	elem_t * pos;
	tm_pentity_set_for_each( pos, pe_set )
	{
		tm_entity_t* ent = ((tm_pentity_t*) pos)->ent;
		move_dist = tm_game_action_colision_detection( pl, ent, move_dist );
	}

	value_t dir = pl->attrs[PL_DIR];
	int dist = tm_rect_distance_10( &pl->r, &tm_wm.map_walls[dir] );	assert( dist != -1 );
	if( dist < move_dist )		move_dist = dist;

	vect_t move_v;
	vect_scale( &dirs[pl->attrs[PL_DIR]], move_dist, &move_v );
	tm_worldmap_move( pl, &move_v );
}

void tm_game_action_eat( tm_entity_t* pl, tm_pentity_set_t* pe_set )
{
	int gain = 0;
	elem_t * pos;
	tm_pentity_set_for_each( pos, pe_set )
	{
		tm_entity_t* ent = ((tm_pentity_t*) pos)->ent;		assert( ent->ent_type == ET_APPLE );
		gain += tm_game_action_eat_entity( ent );
	}

	tm_entity_set_attr( pl, PL_LIFE, pl->attrs[PL_LIFE] + gain );
}

void tm_game_action_attack( tm_entity_t* pl, tm_pentity_set_t* pe_set )
{
	int gain = 0;
	elem_t * pos;
	tm_pentity_set_for_each( pos, pe_set )
	{
		tm_entity_t* ent = ((tm_pentity_t*) pos)->ent;		assert( ent->ent_type == ET_PLAYER );
		gain += tm_game_action_attack_entity( pl, ent );
	}
	tm_entity_set_attr( pl, PL_LIFE, pl->attrs[PL_LIFE] + gain );
	// signals that the player has been attacked, used for rendering
	if( gain < 0 )				tm_entity_set_attr( pl, PL_HIT, 1 );
}


void tm_game_action2( int a_id, tm_entity_t* pl )
{
	assert( a_id >= 0 && a_id < n_actions && pl && pl->ent_type == ET_PLAYER );

	rect_t ar = tm_game_action_range( a_id, pl, &tm_wm.map_r );		// get the area from the map affected by this action
	int etypes = game_action_etypes( a_id );						// get the bitset of entity_types affected by this action

	BEGIN_TRANSACTION();


	tm_pentity_set_t* pe_set = tm_worldmap_get_entities( &ar, etypes ); // get all the entities present in the "ar" area

	if( a_id == AC_MOVE )
		tm_game_action_move( pl, pe_set );
	if( a_id == AC_EAT )
		tm_game_action_eat( pl, pe_set );
	if( a_id == AC_ATTACK )
		tm_game_action_attack( pl, pe_set );


	COMMIT_TRANSACTION();


	tm_pentity_set_destroy( pe_set );
}

*/


