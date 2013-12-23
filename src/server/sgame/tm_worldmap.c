#include "tm_worldmap.h"

#include "../server.h"
#include "../syn_server_traces.h"

tm_worldmap_t tm_wm;
tm_int* next_ent_id;

void tm_worldmap_init( conf_t* c, coord_t szx, coord_t szy, int tdepth )
{
	int i;

	vect_init( &tm_wm.size, szx, szy );

	rect_init4( &tm_wm.map_r, 0, 0, tm_wm.size.x, tm_wm.size.y );
	rect_init4( &tm_wm.map_walls[DIR_UP], 0, tm_wm.size.y, tm_wm.size.x, tm_wm.size.y );
	rect_init4( &tm_wm.map_walls[DIR_RIGHT], tm_wm.size.x, 0, tm_wm.size.x, tm_wm.size.y );
	rect_init4( &tm_wm.map_walls[DIR_DOWN], 0, 0, tm_wm.size.x, 0 );
	rect_init4( &tm_wm.map_walls[DIR_LEFT], 0, 0, 0, tm_wm.size.y );

	tm_wm.area = rect_area( &tm_wm.map_r );
	tm_wm.depth  = tdepth;
	
	tm_wm.area_tree = tm_area_node_create( tm_wm.map_r, DIR_UP, tm_wm.depth );

	tm_wm.n_entities0 = (int *) calloc( n_entity_types, sizeof(int) ); 			
	assert( tm_wm.n_entities0 );
	tm_wm.n_entities1 = (int *) calloc( n_entity_types, sizeof(int) ); 			
	assert( tm_wm.n_entities1 );
	tm_wm.n_entities2 = (tm_int *) calloc( n_entity_types, sizeof(tm_int) ); 	
	assert( tm_wm.n_entities2 );
	
	tm_wm.et_ratios0 = (int *) calloc( n_entity_types, sizeof(int) ); 			
	assert( tm_wm.et_ratios0 );
	tm_wm.et_ratios1 = (int *) calloc( n_entity_types, sizeof(int) ); 			
	assert( tm_wm.et_ratios1 );
	tm_wm.et_ratios2 = (tm_int *) calloc( n_entity_types, sizeof(tm_int) ); 	
	assert( tm_wm.et_ratios2 );
	
	tm_wm.entities0 = (entity_t ***) malloc( n_entity_types * sizeof(entity_t**) ); 				
	assert( tm_wm.entities0 );
	tm_wm.entities1 = (tm_entity_stationary_t ***) malloc( n_entity_types * sizeof(tm_entity_stationary_t**) );			
	assert( tm_wm.entities1 );
	tm_wm.entities2 = (tm_p_tm_entity_movable_t **) malloc( n_entity_types * sizeof(tm_p_tm_entity_movable_t*) );	
	assert( tm_wm.entities2 );

	next_ent_id = (tm_int*) calloc( n_entity_types, sizeof( tm_int ) ); 
	assert( next_ent_id );

	for( i = 0; i < n_entity_types; i++ )
	{	
		tm_wm.entities0[i] = (entity_t **) calloc( MAX_ENTITIES, sizeof(entity_t*) ); 
		assert( tm_wm.entities0[i] );
		tm_wm.entities1[i] = (tm_entity_stationary_t **) calloc( MAX_ENTITIES, sizeof(tm_entity_stationary_t*) ); 
		assert( tm_wm.entities1[i] );
		tm_wm.entities2[i] = (tm_p_tm_entity_movable_t *) calloc( MAX_ENTITIES, sizeof(tm_p_tm_entity_movable_t) ); 
		assert( tm_wm.entities2[i] );
		next_ent_id[i] = 0;
	}

	log_info3( "Worldmap init done - Size: %d x %d - Tree depth: %d\n", tm_wm.size.x, tm_wm.size.y, tm_wm.depth );
}


int tm_worldmap_get_free_ent_id( int et_id )
{
	int base = next_ent_id[ et_id ];
	int i = 0;
	if( entity_types[et_id]->protect == PROTECT_NONE )
	{
		for( i = 0; i < MAX_ENTITIES; i++ )
			if( tm_wm.entities0[et_id][ (base+i) % MAX_ENTITIES ] == NULL )		
			   break;
	}
	else if( entity_types[et_id]->protect == PROTECT_ATTRS )
	{
		for( i = 0; i < MAX_ENTITIES; i++ )
			if( tm_wm.entities1[et_id][ (base+i) % MAX_ENTITIES ] == NULL )		
			   break;
	}
	else if( entity_types[et_id]->protect == PROTECT_ALL )
	{
		for( i = 0; i < MAX_ENTITIES; i++ )
			if( tm_wm.entities2[et_id][ (base+i) % MAX_ENTITIES ] == NULL )		
			   break;
	}
	else assert(0);
	
	assert( i < MAX_ENTITIES );
	next_ent_id[ et_id ] = (base+i+1) % MAX_ENTITIES;
	return (base+i) % MAX_ENTITIES;
}

int tm_worldmap_is_close_to_quest( rect_t* r )
{
	int i, j, dist, min_dist = tm_wm.size.x + tm_wm.size.y;
	for( i = 0; i < sv.wl_quest_count; i++ )
	{
		for( j = 0; j < sv.wl_quest_spread; j++ )
		{
			dist = rect_distance( &sv.quest_locations[i][j], r );
			if( dist < min_dist )	
			   min_dist = dist;
		}
	}
	return (min_dist < sv.n_multiple_actions * entity_types[ ET_PLAYER ]->attr_types[ PL_SPEED ].max);
}

int tm_worldmap_add0( entity_t* ent )
{
	assert( ent && entity_is_valid( ent, &tm_wm.size ) );
	int et_id = ent->ent_type;

	if( tm_wm.et_ratios0[et_id] >= ( ((double)tm_wm.area) * entity_types[et_id]->ratio) / MAX_RATIO )
	{
		log_info1( "[I] entity_type %s: ratio limit reached\n", (char *) entity_types[et_id]->name );
		return 0;
	}
	if( tm_wm.n_entities0[et_id] == MAX_ENTITIES )
	{
		log_info1( "[I] entity_type %s: MAX_ENTITIES limit reached\n", (char *) entity_types[et_id]->name );
		return 0;
	}

	tm_wm.et_ratios0[et_id] += rect_area( &ent->r );

	#ifdef GENERATE_NEAR_QUESTS
	if( et_id != ET_PLAYER && !tm_worldmap_is_close_to_quest( &ent->r ) )
	{
		entity_destroy( ent );
		return 1;
	}
	#endif

	if( ent->ent_id == -1 )
		ent->ent_id = tm_worldmap_get_free_ent_id( et_id );

	tm_wm.entities0[et_id][ent->ent_id] = ent;
	tm_wm.n_entities0[et_id]++;

	tm_area_node_t* an = tm_area_node_get_node_containing_ent0( tm_wm.area_tree, ent );
	tm_area_node_add0( an, ent );

	server_register_object0( ent );

	return 1;
}


int tm_worldmap_add1( tm_entity_stationary_t* ent )
{
	assert( ent && tm_entity_stationary_is_valid( ent, &tm_wm.size ) );
	int et_id = ent->ent_type;

	if( tm_wm.et_ratios1[et_id] >= ( ((double)tm_wm.area) * entity_types[et_id]->ratio) / MAX_RATIO )
	{
		log_info1( "[I] entity_type %s: ratio limit reached\n", (char *) entity_types[et_id]->name );
		return 0;
	}
	if( tm_wm.n_entities1[et_id] == MAX_ENTITIES )
	{
		log_info1( "[I] entity_type %s: MAX_ENTITIES limit reached\n", (char *) entity_types[et_id]->name );
		return 0;
	}

	tm_wm.et_ratios1[et_id] += rect_area( &ent->r );

	#ifdef GENERATE_NEAR_QUESTS
	if( et_id != ET_PLAYER && !tm_worldmap_is_close_to_quest( &ent->r ) )
	{
		tm_entity_stationary_destroy( ent );
		return 1;
	}
	#endif

	if( ent->ent_id == -1 )
		ent->ent_id = tm_worldmap_get_free_ent_id( et_id );
	
	tm_wm.entities1[et_id][ent->ent_id] = ent;
	tm_wm.n_entities1[et_id]++;
	
	tm_area_node_t* an = tm_area_node_get_node_containing_ent1( tm_wm.area_tree, ent );
	tm_area_node_add1( an, ent );
	
	server_register_object1( ent );

	return 1;
}


int tm_worldmap_add2( tm_entity_movable_t* ent )
{
	assert( ent && tm_entity_movable_is_valid( ent, &tm_wm.size ) );
	int et_id = ent->ent_type;

	if( tm_wm.et_ratios2[et_id] >= ( ((double)tm_wm.area) * entity_types[et_id]->ratio) / MAX_RATIO )
	{
		log_info1( "[I] entity_type %s: ratio limit reached\n", (char *) entity_types[et_id]->name );
		return 0;
	}
	if( tm_wm.n_entities2[et_id] == MAX_ENTITIES )
	{
		log_info1( "[I] entity_type %s: MAX_ENTITIES limit reached\n", (char *) entity_types[et_id]->name );
		return 0;
	}

	if( ent->ent_id == -1 )
		ent->ent_id = tm_worldmap_get_free_ent_id( et_id );

	tm_wm.entities2[et_id][ent->ent_id] = ent;
	tm_wm.n_entities2[et_id]++;
	tm_wm.et_ratios2[et_id] += tm_rect_area( &ent->r );
	
	tm_area_node_t* an = tm_area_node_get_node_containing_ent2( tm_wm.area_tree, ent );
	tm_area_node_add2( an, ent );

	server_register_object2( ent );

	return 1;
}

void tm_worldmap_del0( entity_t* ent )
{
	assert( ent );
	int et_id = ent->ent_type;

	tm_wm.et_ratios0[et_id] -= rect_area( &ent->r );
	tm_wm.n_entities0[et_id]--;
	tm_wm.entities0[et_id][ent->ent_id] = NULL;

	tm_area_node_t* an = tm_area_node_get_node_containing_ent0( tm_wm.area_tree, ent );
	tm_area_node_del0( an, ent );
}

void tm_worldmap_del1( tm_entity_stationary_t* ent )
{
	assert( ent );
	int et_id = ent->ent_type;

	tm_wm.et_ratios1[et_id] -= rect_area( &ent->r );
	tm_wm.n_entities1[et_id]--;
	tm_wm.entities1[et_id][ent->ent_id] = NULL;

	tm_area_node_t* an = tm_area_node_get_node_containing_ent1( tm_wm.area_tree, ent );
	tm_area_node_del1( an, ent );
}

void tm_worldmap_del2( tm_entity_movable_t* ent )
{
	assert( ent );
	int et_id = ent->ent_type;

	tm_wm.et_ratios2[et_id] -= tm_rect_area( &ent->r );
	tm_wm.n_entities2[et_id]--;
	tm_wm.entities2[et_id][ent->ent_id] = NULL;

	tm_area_node_t* an = tm_area_node_get_node_containing_ent2( tm_wm.area_tree, ent );
	tm_area_node_del2( an, ent );
}



void tm_worldmap_move2( tm_entity_movable_t* ent, vect_t* move_v )
{
	#ifndef ALWAYS_MOVE
	if( move_v->x == 0 && move_v->y == 0 )	
	   return;
	#endif

 	rect_t aux;
 	rect_init4( &aux, ent->r.v1.x, ent->r.v1.y, ent->r.v2.x, ent->r.v2.y );
 	tm_area_node_t* an_1 = tm_area_node_get_node_containing_rect( tm_wm.area_tree, &aux );
 	vect_add( &aux.v1, move_v, &aux.v1 );
 	vect_add( &aux.v2, move_v, &aux.v2 );
 	tm_area_node_t* an_2 = tm_area_node_get_node_containing_rect( tm_wm.area_tree, &aux );
 	
 	#ifndef ALWAYS_MOVE
 	if( an_1 != an_2 )
 	#endif
 	{
 		tm_area_node_del2( an_1, ent );
 	}
		ent->r.v1.x = aux.v1.x;
		ent->r.v1.y = aux.v1.y;
		ent->r.v2.x = aux.v2.x;
		ent->r.v2.y = aux.v2.y;
	#ifndef ALWAYS_MOVE
	if( an_1 != an_2 )
	#endif
	{
		tm_area_node_add2( an_2, ent );
	}
}



int tm_worldmap_is_vacant_from_fixed( rect_t* r )
{
	int i, etypes = 0;
	for( i = 0; i < n_entity_types; i++ )
		if( entity_types[i]->fixed )
			etypes |= (1 << i);
	return tm_area_node_is_vacant( tm_wm.area_tree, r, etypes );
}

int tm_worldmap_is_vacant_from_mobile( rect_t* r )
{
	int i, etypes = 0;
	for( i = 0; i < n_entity_types; i++ )
		if( !entity_types[i]->fixed )
			etypes |= (1 << i);
	return tm_area_node_is_vacant( tm_wm.area_tree, r, etypes );
}

int tm_worldmap_is_vacant( rect_t* r )
{
	return tm_area_node_is_vacant( tm_wm.area_tree, r, 0xffffffff );
}


entity_t* tm_worldmap_generate_ent0( int ent_type, rect_t* where )
{
	entity_t* ent = entity_create( ent_type ); assert( ent );
	entity_generate_attrs( ent );
	game_entity_generate_size( ent );

	int trials = 0;
	do
	{
		entity_generate_position( ent, where );
		trials++;
	} while( !tm_worldmap_is_vacant( &ent->r ) && trials < 100 );

	if( trials == 100 )
	{
		printf( "[W] Map too crowded to add entities.\n" );
		entity_destroy( ent ); ent = NULL;
	}
	return ent;
}

tm_entity_stationary_t* tm_worldmap_generate_ent1( int ent_type, rect_t* where )
{
	tm_entity_stationary_t* ent = tm_entity_stationary_create( ent_type ); assert( ent );
	tm_entity_stationary_generate_attrs( ent );
	tm_game_entity1_generate_size( ent );

	int trials = 0;
	do
	{
		tm_entity_stationary_generate_position( ent, where );
		trials++;
	} while( !tm_worldmap_is_vacant( &ent->r ) && trials < 100 );

	if( trials == 100 )
	{
		printf( "[W] Map too crowded to add entities.\n" );
		tm_entity_stationary_destroy( ent ); ent = NULL;
	}
	return ent;
}

tm_entity_movable_t* tm_worldmap_generate_ent2( int ent_type, rect_t* where )
{
	tm_entity_movable_t* ent = tm_entity_movable_create( ent_type ); assert( ent );
	tm_entity_movable_generate_attrs( ent );
	tm_game_entity2_generate_size( ent );

	int trials = 0;
	rect_t ent_r;
	do
	{
		tm_entity_movable_generate_position( ent, where );
		rect_init4( &ent_r, ent->r.v1.x, ent->r.v1.y, ent->r.v2.x, ent->r.v2.y );
		trials++;
	} while( !tm_worldmap_is_vacant( &ent_r ) && trials < 100 );

	if( trials == 100 )
	{
		printf( "[W] Map too crowded to add entities.\n" );
		tm_entity_movable_destroy( ent ); ent = NULL;
	}
	return ent;
}




#ifndef RUN_TRACES

void tm_worldmap_generate()
{
	log_info( "Worldmap generate ...\n" );
	
	int i;
	for( i = 0; i < n_entity_types; i++ )
	{
		if( !entity_types[i]->fixed )		
		   continue;

		if( entity_types[i]->protect == PROTECT_NONE )
		{
			entity_t* ent;
			do
			{
				ent = tm_worldmap_generate_ent0( i, &tm_wm.map_r );
			} while( ent && tm_worldmap_add0( ent ) );

			if( ent != NULL )		entity_destroy( ent );
		}
		if( entity_types[i]->protect == PROTECT_ATTRS )
		{
			tm_entity_stationary_t* ent;
			do
			{
				ent = tm_worldmap_generate_ent1( i, &tm_wm.map_r );
			} while( ent && tm_worldmap_add1( ent ) );

			if( ent != NULL )		tm_entity_stationary_destroy( ent );
		}
		if( entity_types[i]->protect == PROTECT_ALL )
		{
			tm_entity_movable_t* ent;
			do
			{
				ent = tm_worldmap_generate_ent2( i, &tm_wm.map_r );
			} while( ent && tm_worldmap_add2( ent ) );

			if( ent != NULL )		tm_entity_movable_destroy( ent );
		}
	}

	#ifdef REGISTER_TRACES
	fprintf(sv.f_trace_objects, "-1"); // invalid object type to determine EOF
	#endif

	log_info( "Worldmap generate done.\n" );
}

#else	// RUN_TRACES


entity_t* tm_worldmap_generate_ent_from_trace( int ent_type )
{
	int i, check;
	value_t atr;
	short v1x, v1y, v2x, v2y;
	entity_t* ent = NULL;

	FILE* f_in;
	if( ent_type != ET_PLAYER )
	{
		f_in = sv.f_trace_objects;
		fscanf( f_in, "%d", &ent_type);

		if( ent_type == -1 ) 		
		   return NULL;
		assert( ent_type == ET_APPLE || ent_type == ET_WALL );
	}
	else	f_in = sv.f_trace_players;

	entity_type_t* et = entity_types[ent_type];
	ent = entity_create( ent_type ); assert( ent );
	fscanf( f_in, "%hd %hd %hd %hd", &v1x, &v1y, &v2x, &v2y);
	rect_init4( &ent->r, v1x, v1y, v2x, v2y );

	#ifdef TRACE_DEBUG
	printf( "%s [%hd;%hd - %hd;%hd]: \n", et->name, v1x, v1y, v2x, v2y );
	#endif

	for (i = 0; i < et->n_attr; ++i)
	{
		fscanf( f_in, "%d %d", &check, &atr);	assert( check == i );
		ent->attrs[i] = atr;

		#ifdef TRACE_DEBUG
		printf("\t Attr[%d]=%d\n", i, atr);
		#endif
	}

	if(ent_type == ET_PLAYER)
	{
		ent->size.x = ent->attrs[ PL_SIZE ];
		ent->size.y = ent->size.x;
	}
	if( ent_type == ET_APPLE )
	{
		ent->size.x = ent->attrs[ AP_SIZE ];
		ent->size.y = ent->size.x;
	}
	if( ent_type == ET_WALL )
	{
		ent->size.x = ent->attrs[ WL_SIZE_X ];
		ent->size.y = ent->attrs[ WL_SIZE_Y ];
	}

	return ent;
}

void tm_worldmap_generate()
{
	log_info( "Worldmap generate ...\n" );
	
	entity_t* ent;
	while(1)
	{
		ent = tm_worldmap_generate_ent_from_trace( ET_APPLE ); // or ET_WALL, just different than ET_PLAYER
		if( !ent )			
		   break;

		if( entity_types[ ent->ent_type ]->protect == PROTECT_NONE )
			if( !tm_worldmap_add0( ent ) )		
			   break;

		if( entity_types[ ent->ent_type ]->protect == PROTECT_ATTRS )
		{
			tm_entity_stationary_t* ent1 = tm_entity_stationary_create( ent_type ); 
			assert( ent1 );
			tm_entity_copy10( ent1, ent );
			entity_destroy( ent );
			if( !tm_worldmap_add1( ent1 ) )		
			   break;
		}
		
		if( entity_types[ ent->ent_type ]->protect == PROTECT_ALL )
		{
			tm_entity_movable_t* ent2 = tm_entity_movable_create( ent_type ); 
			assert( ent2 );
			tm_entity_copy20( ent2, ent );
			entity_destroy( ent );
			if( !tm_worldmap_add2( ent2 ) )		
			   break;
		}
	}

	#ifdef REGISTER_TRACES
	fprintf(sv.f_trace_objects, "-1"); // invalid object type to determine EOF
	#endif

	log_info( "Worldmap generate done.\n" );
}


#endif	 // RUN_TRACES





void tm_worldmap_is_valid()
{
	int i, j;
	int* n_ents = (int*) malloc( n_entity_types * sizeof(int) );

	for( i = 0; i < n_entity_types; i++ )
	{
		n_ents[i] = 0;
		
		for( j = 0; j < tm_wm.n_entities0[i]; j++ )
			assert( entity_is_valid( tm_wm.entities0[i][j], &tm_wm.size ) );
			
		for( j = 0; j < tm_wm.n_entities1[i]; j++ )
			assert( tm_entity_stationary_is_valid( tm_wm.entities1[i][j], &tm_wm.size ) );
			
		for( j = 0; j < tm_wm.n_entities2[i]; j++ )
			assert( tm_entity_movable_is_valid( tm_wm.entities2[i][j], &tm_wm.size ) );
	}

	tm_area_node_is_valid( tm_wm.area_tree, n_ents );

	for( i = 0; i < n_entity_types; i++ )
		assert( n_ents[i] == tm_wm.n_entities0[i] + tm_wm.n_entities1[i] + tm_wm.n_entities2[i] );
}

void tm_worldmap_print( int verbose )
{
	int i, j;
	level_stats_t* level_stats = (level_stats_t*) malloc( (tm_wm.depth+1) * sizeof( level_stats_t ) );
	assert( level_stats );
	for( i = 0; i < tm_wm.depth+1; i++ )
	{
		level_stats[i].n_ents   = (int*)malloc( n_entity_types * sizeof( int ) );
		assert( level_stats[i].n_ents );
		level_stats[i].min_ents = (int*)malloc( n_entity_types * sizeof( int ) );
		assert( level_stats[i].min_ents );
		level_stats[i].max_ents = (int*)malloc( n_entity_types * sizeof( int ) );
		assert( level_stats[i].max_ents );
		for( j = 0; j < n_entity_types; j++ )
		{
			level_stats[i].n_ents[j] = 0;
			level_stats[i].min_ents[j] = -1;
			level_stats[i].max_ents[j] = -1;
		}
	}

	tm_area_node_print( tm_wm.area_tree, 0, verbose, level_stats );

	printf( "EntityTree: for each level : n_ents(avg_ents)(min_ents : max_ents) \n" );
	for( i = 0; i < n_entity_types; i++ )
	{
		printf( "%6s: ", entity_types[i]->name );
		for( j = 0; j < tm_wm.depth+1; j++ )
			printf( "%3d(%3d)(%3d:%3d) ", level_stats[j].n_ents[i], level_stats[j].n_ents[i] / (1<<j),
					level_stats[j].min_ents[i], level_stats[j].max_ents[i] );
		printf( "\n" );
	}
	printf( "\n" );
}



tm_parea_node_set_t* tm_worldmap_get_nodes( rect_t* range )
{
	tm_parea_node_set_t* pan_set = tm_parea_node_set_create();
	tm_area_node_get_nodes( tm_wm.area_tree, range, pan_set );
	return pan_set;
}

tm_parea_node_set_t* tm_worldmap_get_leaves( rect_t* range )
{
	tm_parea_node_set_t* pan_set = tm_parea_node_set_create();
	tm_area_node_get_leaves( tm_wm.area_tree, range, pan_set );
	return pan_set;
}

tm_parea_node_set_t* tm_worldmap_get_parents( rect_t* range )
{
	tm_parea_node_set_t* pan_set = tm_parea_node_set_create();
	tm_area_node_get_parents( tm_wm.area_tree, range, pan_set );
	return pan_set;
}


/*

void tm_worldmap_pack( streamer_t* st, rect_t* range )
{
	int i;
	for( i = 0; i < n_entity_types; i++ )
	{
		if( i == ET_WALL )
			continue;
		tm_pentity_set_t* pe_set = tm_worldmap_get_entities( range, 1 << i );

		assert( i != ET_PLAYER || pentity_set_size( pe_set ) > 0 );
		streamer_wrint( st, pentity_set_size( pe_set ) );

		elem_t * pos;
		tm_pentity_set_for_each( pos, pe_set )
			tm_game_entity_pack( ((tm_pentity_t*)pos)->ent, st );

		tm_pentity_set_destroy( pe_set );
	}
}

void tm_worldmap_pack_fixed( streamer_t* st )
{
	int i, j;

	for( i = 0; i < n_entity_types; i++ )
	{
		if( !entity_types[i]->fixed )		continue;

		streamer_wrint( st, tm_wm.n_entities[i] );
		for( j = 0; j < tm_wm.n_entities[i]; j++ )
			tm_entity_pack_fixed( tm_wm.entities[i][j], st );
	}
}


	OBSOLETE
tm_pentity_set_t* tm_worldmap_get_entities( rect_t* range, int etypes )
{
	tm_pentity_set_t* pe_set = tm_pentity_set_create();
	tm_area_node_get_entities( tm_wm.area_tree, range, etypes, pe_set );
	return pe_set;
}
*/

