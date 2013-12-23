#include "tm_area_node.h"

#include "tm_worldmap.h"


void tm_area_node_init( tm_area_node_t* an, rect_t _loc, int _split_dir, int level )
{
	int i;
	assert( an && _loc.v1.x <= _loc.v2.x && _loc.v1.y <= _loc.v2.y && level >= 0 );
	assert( _split_dir == DIR_UP || _split_dir == DIR_RIGHT );

	an->loc = _loc;
	rect_init4( &an->split, -1, -1, -1, -1 );
	an->left = NULL;
	an->right = NULL;

	an->ent0_sets = new entity_set_t[ n_entity_types ];		
	assert( an->ent0_sets );
	an->ent1_sets = new entity_set_t[ n_entity_types ];		
	assert( an->ent1_sets );
	an->ent2_sets = new tm_entity_set_t[ n_entity_types ];	
	assert( an->ent2_sets );
	
	for( i = 0; i < n_entity_types; i++ )
	{
		entity_set_init( &an->ent0_sets[i] );
		entity_set_init( &an->ent1_sets[i] );
		tm_entity_set_init( &an->ent2_sets[i] );
	}


	if( level )
	{
		rect_t r_left, r_right;
		rect_split( &_loc, _split_dir, &r_left, &r_right, &an->split );

		an->left = tm_area_node_create(r_left, 1 - _split_dir, level - 1);
		an->right = tm_area_node_create(r_right, 1 - _split_dir, level - 1);
	}

}

void tm_area_node_deinit( tm_area_node_t* an )
{
	int i = 0;
	assert( an );

	if( !tm_area_node_is_leaf( an ) )
	{
		tm_area_node_destroy( an->left );
		tm_area_node_destroy( an->right );
	}
	for( i = 0; i < n_entity_types; i++ )
	{
		entity_set_deinit( &an->ent0_sets[i] );
		entity_set_deinit( &an->ent1_sets[i] );
		tm_entity_set_deinit( &an->ent2_sets[i] );
	}

	free( an->ent0_sets );
	free( an->ent1_sets );
	free( an->ent2_sets );
}



tm_area_node_t* tm_area_node_get_node_containing_rect( tm_area_node_t* an, rect_t* r )
{
	if( tm_area_node_is_leaf( an ) || rect_is_overlapping( &an->split, r ) )
		return an;

	if( rect_cmp( r, &an->split ) < 0 )
		return tm_area_node_get_node_containing_rect( an->left, r );
	else
		return tm_area_node_get_node_containing_rect( an->right, r );
}

tm_area_node_t* tm_area_node_get_node_containing_rect2( tm_area_node_t* an, tm_rect_t* r )
{
	if( tm_area_node_is_leaf( an ) || tm_rect_is_overlapping_01( &an->split, r ) )
		return an;

	if( tm_rect_cmp_10( r, &an->split ) < 0 )
		return tm_area_node_get_node_containing_rect2( an->left, r );
	else
		return tm_area_node_get_node_containing_rect2( an->right, r );
}



tm_area_node_t* tm_area_node_get_node_containing_ent0( tm_area_node_t* an, entity_t* ent )
{
	return tm_area_node_get_node_containing_rect( an, &ent->r );
}

tm_area_node_t* tm_area_node_get_node_containing_ent1( tm_area_node_t* an, tm_entity_stationary_t* ent )
{
	return tm_area_node_get_node_containing_rect( an, &ent->r );
}

tm_area_node_t* tm_area_node_get_node_containing_ent2( tm_area_node_t* an, tm_entity_movable_t* ent )
{
	return tm_area_node_get_node_containing_rect2( an, &ent->r );
}




extern __thread    int p_thread_id;


/* add "ent" into the area tree represented by "an" */
/* assumes that "ent" doesn't overlap with any existing entities in "an" */
void tm_area_node_add0( tm_area_node_t* an, entity_t* ent )
{
	entity_set_add( &an->ent0_sets[ent->ent_type], ent );
}

void tm_area_node_add1( tm_area_node_t* an, tm_entity_stationary_t* ent )
{
	entity_set_add( &an->ent1_sets[ent->ent_type], ent );
}

void tm_area_node_add2( tm_area_node_t* an, tm_entity_movable_t* ent )
{

	log_info3( "%x - area_node_add  --  ent_set: %x  ent %x\n", p_thread_id, (unsigned)&an->ent2_sets[ent->ent_type].h_meta, (unsigned)ent );

	tm_entity_set_add( &an->ent2_sets[ent->ent_type], ent );

	log_info3( "%x - area_node_add done  --  ent_set: %x  ent %x\n", p_thread_id, (unsigned)&an->ent2_sets[ent->ent_type].h_meta, (unsigned)ent );
}



/* delete "ent" from the area tree represented by "an" */
/* assumes that "ent" is placed correctly in "an" */
void tm_area_node_del0( tm_area_node_t* an, entity_t* ent )
{
	entity_set_del( &an->ent0_sets[ent->ent_type], ent );
}

void tm_area_node_del1( tm_area_node_t* an, tm_entity_stationary_t* ent )
{
	entity_set_del( &an->ent1_sets[ent->ent_type], ent );
}

void tm_area_node_del2( tm_area_node_t* an, tm_entity_movable_t* ent )
{
	log_info3( "%x - area_node_del  --  ent_set: %x  ent %x\n", p_thread_id, (unsigned)&an->ent2_sets[ent->ent_type].h_meta, (unsigned)ent );

	tm_entity_set_del( &an->ent2_sets[ent->ent_type], ent );

	log_info3( "%x - area_node_del  done  --  ent_set: %x  ent %x\n", p_thread_id, (unsigned)&an->ent2_sets[ent->ent_type].h_meta, (unsigned)ent );
}



/* checks whether the area occupied by "r" is free within "an" */
int tm_area_node_is_vacant( tm_area_node_t* an, rect_t* r, int etypes )
{
	int i;
	elem_t * pos;
	tm_elem_t * tm_pos;
	
	if( !rect_is_overlapping( &an->loc, r ) )	
	   return 1;

	for( i = 0; i < n_entity_types; i++ )
	{
		if( (etypes & (1 << i)) == 0 )	
		   continue;
		
		if( entity_types[i]->protect == PROTECT_NONE )
			entity_set_for_each( pos, &an->ent0_sets[i] )
				if( rect_is_overlapping( r, &((entity_t*) pos)->r ) )
					return 0;

		if( entity_types[i]->protect == PROTECT_ATTRS )
			entity_set_for_each( pos, &an->ent1_sets[i] )
				if( rect_is_overlapping( r, &((tm_entity_stationary_t*) pos)->r ) )
					return 0;

		if( entity_types[i]->protect == PROTECT_ALL )
			tm_entity_set_for_each( tm_pos, &an->ent2_sets[i] )
				if( tm_rect_is_overlapping_01( r, &((tm_entity_movable_t*) tm_pos)->r ) )
					return 0;
	}

	if( !tm_area_node_is_leaf( an ) )
	{
		if( !tm_area_node_is_vacant( an->left, r, etypes ) )		
		   return 0;
		if( !tm_area_node_is_vacant( an->right, r, etypes ) )		
		   return 0;
	}

	return 1;
}




/* recursively collects into "pan_set" nodes */
/* that have some overlap with "range" */
void tm_area_node_get_nodes( tm_area_node_t* an, rect_t* range, tm_parea_node_set_t* pan_set )
{
	if( !rect_is_overlapping( range, &an->loc ) )	
	   return;

	tm_parea_node_set_add( pan_set, tm_parea_node_create( an ) );

	if( !tm_area_node_is_leaf( an ) )
	{
		tm_area_node_get_nodes( an->left, range, pan_set );
		tm_area_node_get_nodes( an->right, range, pan_set );
	}
}

/* recursively collects into "pan_set" leaves */
/* that have some overlap with "range" */
void tm_area_node_get_leaves( tm_area_node_t* an, rect_t* range, tm_parea_node_set_t* pan_set )
{
	if( !rect_is_overlapping( range, &an->loc ) )	
	   return;

	if( !tm_area_node_is_leaf( an ) )
	{
		tm_area_node_get_leaves( an->left, range, pan_set );
		tm_area_node_get_leaves( an->right, range, pan_set );
		return;
	}
	
	tm_parea_node_set_add( pan_set, tm_parea_node_create( an ) );
}

/* recursively collects into "pan_set" parents */
/* that have some overlap with "range" */
void tm_area_node_get_parents( tm_area_node_t* an, rect_t* range, tm_parea_node_set_t* pan_set )
{
	if( tm_area_node_is_leaf( an ) || !rect_is_overlapping( range, &an->loc ) )	
	   return;

	tm_parea_node_set_add( pan_set, tm_parea_node_create( an ) );
	
	tm_area_node_get_parents( an->left, range, pan_set );
	tm_area_node_get_parents( an->right, range, pan_set );
}



void tm_area_node_is_valid_entity0( tm_area_node_t* an, int* n_ents, int i, entity_t* ent )
{
	assert( ent->ent_type == i && ent == tm_wm.entities0[i][ent->ent_id] );
	assert( rect_is_contained( &ent->r, &an->loc ) );
	assert( tm_area_node_is_leaf( an ) || rect_is_overlapping( &an->split, &ent->r ) );
	assert( entity_is_valid( ent, &tm_wm.size ) );

	n_ents[i]++;
}

void tm_area_node_is_valid_entity1( tm_area_node_t* an, int* n_ents, int i, tm_entity_stationary_t* ent )
{
	assert( ent->ent_type == i && ent == tm_wm.entities1[i][ent->ent_id] );
	assert( rect_is_contained( &ent->r, &an->loc ) );
	assert( tm_area_node_is_leaf( an ) || rect_is_overlapping( &an->split, &ent->r ) );
	assert( tm_entity_stationary_is_valid( ent, &tm_wm.size ) );

	n_ents[i]++;
}

void tm_area_node_is_valid_entity2( tm_area_node_t* an, int* n_ents, int i, tm_entity_movable_t* ent )
{
	assert( ent->ent_type == i && ent == tm_wm.entities2[i][ent->ent_id] );
	assert( tm_rect_is_contained_10( &ent->r, &an->loc ) );
	assert( tm_area_node_is_leaf( an ) || tm_rect_is_overlapping_01( &an->split, &ent->r ) );
	assert( tm_entity_movable_is_valid( ent, &tm_wm.size ) );

	n_ents[i]++;
}

void tm_area_node_is_valid( tm_area_node_t* an, int* n_ents )
{
	assert( an && an->loc.v1.x <= an->loc.v2.x && an->loc.v1.y <= an->loc.v2.y );

	elem_t * pos;
	tm_elem_t * tm_pos;

	int i;
	for( i = 0; i < n_entity_types; i++ )
	{
		if( entity_types[i]->protect == PROTECT_NONE )
			entity_set_for_each( pos, &an->ent0_sets[i] )
				tm_area_node_is_valid_entity0( an, n_ents, i, (entity_t*) pos );

		if( entity_types[i]->protect == PROTECT_ATTRS )
			entity_set_for_each( pos, &an->ent1_sets[i] )
				tm_area_node_is_valid_entity1( an, n_ents, i, (tm_entity_stationary_t*) pos );

		if( entity_types[i]->protect == PROTECT_ALL )
			tm_entity_set_for_each( tm_pos, &an->ent2_sets[i] )
				tm_area_node_is_valid_entity2( an, n_ents, i, (tm_entity_movable_t*) tm_pos );
	}

	if( !tm_area_node_is_leaf( an ) )
	{
		tm_area_node_is_valid( an->left, n_ents );
		tm_area_node_is_valid( an->right, n_ents );
	}
}


void tm_area_node_print( tm_area_node_t* an, int level, int verbose, level_stats_t* level_stats )
{
	int i;

	if( verbose )
	{
		for( i = 0; i < level; i++ )	printf("\t");
		printf( "%d - Loc (%d,%d) (%d,%d) - ", level, an->loc.v1.x, an->loc.v1.y, an->loc.v2.x, an->loc.v2.y );
		for( i = 0; i < n_entity_types; i++ )	
			printf( "%d ", entity_set_size( &an->ent0_sets[i] ) + entity_set_size( &an->ent1_sets[i] ) + 
					tm_entity_set_size( &an->ent2_sets[i] ));
		printf( "\n" );
	}

	int set_size;
	for( i = 0; i < n_entity_types; i++ )
	{
		set_size = entity_set_size( &an->ent0_sets[i] ) + entity_set_size( &an->ent1_sets[i] ) + 
				tm_entity_set_size( &an->ent2_sets[i] );

		level_stats[level].n_ents[i] += set_size;
		if( level_stats[level].min_ents[i] == -1 || set_size < level_stats[level].min_ents[i] )
			level_stats[level].min_ents[i] = set_size;
		if( level_stats[level].max_ents[i] == -1 || set_size > level_stats[level].max_ents[i] )
			level_stats[level].max_ents[i] = set_size;
	}

     if( an->left )
     {
        tm_area_node_print( an->left, level+1, verbose, level_stats );
        tm_area_node_print( an->right, level+1, verbose, level_stats );
     }
}
