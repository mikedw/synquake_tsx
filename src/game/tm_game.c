#include "tm_game.h"

void tm_game_entity1_generate_size( tm_entity_stationary_t* ent )
{
	assert( ent != NULL );
	if( ent->ent_type == ET_PLAYER )
	{
		ent->size.x = ent->attrs[PL_SIZE];
		ent->size.y = ent->size.x;
	}
	if( ent->ent_type == ET_APPLE )
	{
		ent->size.x = ent->attrs[AP_SIZE];
		ent->size.y = ent->size.x;
	}
	if( ent->ent_type == ET_WALL )
	{
		if( rand_range(0,1) == 0 )
		{
			ent->size.x = ent->attrs[WL_SIZE_X];
			ent->size.y = entity_types[ent->ent_type]->attr_types[WL_SIZE_Y].min;
		}
		else
		{
			ent->size.y = ent->attrs[WL_SIZE_Y];
			ent->size.x = entity_types[ent->ent_type]->attr_types[WL_SIZE_X].min;
		}
	}
}

void tm_game_entity1_pack( tm_entity_stationary_t* ent, streamer_t* st )
{
	streamer_wrint( st, ent->ent_id );

	// if the entity is mobile pack its location
	if( !entity_types[ent->ent_type]->fixed )
		rect_pack( &ent->r, st );

	// pack only the relevant attributes based on entity type
	int i, n_attr = entity_types[ent->ent_type]->n_attr;
	switch( ent->ent_type )
	{
		case ET_PLAYER:
			i = PL_DIR;
			break;
		case ET_APPLE:
			i = AP_FOOD;
			break;
		default:
			i = n_attr;
	}
	assert( sizeof(value_t) == sizeof(int));
	for( ; i < n_attr; i++ )
		streamer_wrint( st, ent->attrs[i] );
}

void tm_game_entity1_unpack( tm_entity_stationary_t* ent, streamer_t* st )
{
	if( !entity_types[ent->ent_type]->fixed )
	{
		rect_unpack( &ent->r, st );
		vect_substract( &ent->r.v2, &ent->r.v1, &ent->size );
	}

	int i, n_attr = entity_types[ent->ent_type]->n_attr;
	switch( ent->ent_type )
	{
		case ET_PLAYER:
			i = PL_DIR;
			break;
		case ET_APPLE:
			i = AP_FOOD;
			break;
		default:
			i = n_attr;
	}
	assert( sizeof(value_t) == sizeof(int));
	for( ; i < n_attr; i++ )
		ent->attrs[i] = streamer_rdint( st );
}








void tm_game_entity2_generate_size( tm_entity_movable_t* ent )
{
	assert( ent != NULL );
	if( ent->ent_type == ET_PLAYER )
	{
		ent->size.x = ent->attrs[PL_SIZE];
		ent->size.y = ent->size.x;
	}
	if( ent->ent_type == ET_APPLE )
	{
		ent->size.x = ent->attrs[AP_SIZE];
		ent->size.y = ent->size.x;
	}
	if( ent->ent_type == ET_WALL )
	{
		if( rand_range(0,1) == 0 )
		{
			ent->size.x = ent->attrs[WL_SIZE_X];
			ent->size.y = entity_types[ent->ent_type]->attr_types[WL_SIZE_Y].min;
		}
		else
		{
			ent->size.y = ent->attrs[WL_SIZE_Y];
			ent->size.x = entity_types[ent->ent_type]->attr_types[WL_SIZE_X].min;
		}
	}
}

void tm_game_entity2_pack( tm_entity_movable_t* ent, streamer_t* st )
{
	streamer_wrint( st, ent->ent_id );

	// if the entity is mobile pack its location
	if( !entity_types[ent->ent_type]->fixed )
		tm_rect_pack( &ent->r, st );

	// pack only the relevant attributes based on entity type
	int i, n_attr = entity_types[ent->ent_type]->n_attr;
	switch( ent->ent_type )
	{
		case ET_PLAYER:
			i = PL_DIR;
			break;
		case ET_APPLE:
			i = AP_FOOD;
			break;
		default:
			i = n_attr;
	}
	assert( sizeof(value_t) == sizeof(int));
	for( ; i < n_attr; i++ )
		streamer_wrint( st, ent->attrs[i] );
}

void tm_game_entity2_unpack( tm_entity_movable_t* ent, streamer_t* st )
{
	if( !entity_types[ent->ent_type]->fixed )
	{
		tm_rect_unpack( &ent->r, st );
		ent->size.x = ent->r.v2.x - ent->r.v1.x;
		ent->size.y = ent->r.v2.y - ent->r.v1.y;
	}

	int i, n_attr = entity_types[ent->ent_type]->n_attr;
	switch( ent->ent_type )
	{
		case ET_PLAYER:
			i = PL_DIR;
			break;
		case ET_APPLE:
			i = AP_FOOD;
			break;
		default:
			i = n_attr;
	}
	assert( sizeof(value_t) == sizeof(int));
	for( ; i < n_attr; i++ )
		ent->attrs[i] = streamer_rdint( st );
}









rect_t tm_game_action_range( int a_id, tm_entity_movable_t* pl, rect_t* map_r )
{
	assert( a_id >= 0 && a_id < n_actions && pl && pl->ent_type == ET_PLAYER );
	action_range_t ar = action_ranges[a_id];
	rect_t r;

	if( a_id == AC_MOVE )
		ar.front *= pl->attrs[PL_SPEED];

	if( pl->attrs[PL_DIR] == DIR_UP )	rect_init4( &r, -ar.left, -ar.behind, ar.right, ar.front );
	if( pl->attrs[PL_DIR] == DIR_RIGHT )	rect_init4( &r, -ar.behind, -ar.right, ar.front, ar.left );
	if( pl->attrs[PL_DIR] == DIR_DOWN )	rect_init4( &r, -ar.right, -ar.front, ar.left, ar.behind );
	if( pl->attrs[PL_DIR] == DIR_LEFT )	rect_init4( &r, -ar.front, -ar.left, ar.behind, ar.right );

	r.v1.x += pl->r.v1.x;	r.v1.y += pl->r.v1.y;
	r.v2.x += pl->r.v2.x;	r.v2.y += pl->r.v2.y;
	rect_crop( &r, map_r, &r );
	return r;
}

