#include "tm_entity.h"

/*************************************
			TM_ENTITY Stationary
*************************************/

void tm_entity_stationary_init( tm_entity_stationary_t* ent, int _ent_type )
{
	assert( ent && _ent_type >= 0 && _ent_type < n_entity_types );
	elem_init( &ent->e, 0 );
	ent->ent_id = -1;
	ent->ent_type = _ent_type;
}

tm_entity_stationary_t* tm_entity_stationary_create( int _ent_type )
{
	assert( _ent_type >= 0 && _ent_type < n_entity_types );
	tm_entity_stationary_t* ent = (tm_entity_stationary_t*) mgr_on_new( sizeof(tm_entity_stationary_t) + entity_types[_ent_type]->n_attr
			* sizeof(tm_value_t) );

	tm_entity_stationary_init( ent, _ent_type );

	return ent;
}


void tm_entity_stationary_destroy( tm_entity_stationary_t* ent )
{
	assert( ent );
	free( ent );
}

void tm_entity_stationary_generate_attrs( tm_entity_stationary_t* ent )
{
	int i;
	entity_type_t* et = entity_types[ent->ent_type];
	assert( et->n_attr > 0 );
	for( i = 0; i < et->n_attr; ++i )
		ent->attrs[i] = attribute_type_rand( &et->attr_types[i] );
}

void tm_entity_stationary_generate_position( tm_entity_stationary_t* ent, rect_t* map_r )
{
	rect_generate_position( &ent->r, &ent->size, map_r );
}


int tm_entity_stationary_position_is_valid( tm_entity_stationary_t* ent, vect_t* map_sz )
{
	vect_t aux;
	if( !vect_is_positive( &ent->size ) )		return 0;
	if( !rect_is_positive( &ent->r ) )			return 0;
	if( !rect_is_valid( &ent->r ) )				return 0;

	aux.x = map_sz->x - ent->r.v2.x;
	aux.y = map_sz->y - ent->r.v2.y;
	if( !vect_is_positive( &aux ) )					return 0;

	aux.x = ent->r.v2.x - ent->r.v1.x;
	aux.y = ent->r.v2.y - ent->r.v1.y;
	if( !vect_is_eq( &ent->size, &aux ) )		return 0;

	return 1;
}

int tm_entity_stationary_attrs_is_valid( tm_entity_stationary_t* ent )
{
	int et_id = ent->ent_type;
	int i, n_attr = entity_types[et_id]->n_attr;
	for( i = 0; i < n_attr; i++ )
	{
		if( entity_types[et_id]->attr_types[i].min > ent->attrs[i] )		return 0;
		if( entity_types[et_id]->attr_types[i].max < ent->attrs[i] )		return 0;
	}
	return 1;
}

int tm_entity_stationary_is_valid( tm_entity_stationary_t* ent, vect_t* map_sz )
{
	if( !tm_entity_stationary_position_is_valid( ent, map_sz ) )		return 0;
	if( !tm_entity_stationary_attrs_is_valid( ent ) )					return 0;
	return 1;
}

void tm_entity_stationary_set_attr( tm_entity_stationary_t* ent, int attr_id, value_t attr_val )
{
	assert( ent && attr_id < entity_types[ent->ent_type]->n_attr );
	attribute_type_t* attr_type = &entity_types[ent->ent_type]->attr_types[attr_id];

	if( attr_type->min <= attr_val && attr_val <= attr_type->max )
		ent->attrs[attr_id] = attr_val;
	if( attr_type->max < attr_val )
		ent->attrs[attr_id] = attr_type->max;
	if( attr_val < attr_type->min )
		ent->attrs[attr_id] = attr_type->min;
}

void tm_entity_stationary_pack( tm_entity_stationary_t* ent, streamer_t* st )
{
	streamer_wrint( st, ent->ent_type );
	streamer_wrint( st, ent->ent_id );
	rect_pack( &ent->r, st );

	assert( sizeof(value_t) == sizeof(int) );
	int i, n_attr = entity_types[ent->ent_type]->n_attr;
	for( i = 0; i < n_attr; i++ )
		streamer_wrint( st, ent->attrs[i] );
}

void tm_entity_stationary_unpack( tm_entity_stationary_t* ent, streamer_t* st )
{
	rect_unpack( &ent->r, st );
	vect_substract( &ent->r.v2, &ent->r.v1, &ent->size );

	assert( sizeof(value_t) == sizeof(int));
	int i, n_attr = entity_types[ent->ent_type]->n_attr;
	for( i = 0; i < n_attr; i++ )
		ent->attrs[i] = streamer_rdint( st );
}

void tm_entity_stationary_pack_fixed( tm_entity_stationary_t* ent, streamer_t* st )
{
	rect_pack( &ent->r, st );
}

void tm_entity_stationary_unpack_fixed( tm_entity_stationary_t* ent, streamer_t* st )
{
	rect_unpack( &ent->r, st );
	vect_substract( &ent->r.v2, &ent->r.v1, &ent->size );
}


void tm_entity_copy10( tm_entity_stationary_t* ent1, entity_t* ent )
{
	ent1->r.v1.x = ent->r.v1.x;	ent1->r.v1.y = ent->r.v1.y;
	ent1->r.v2.x = ent->r.v2.x;	ent1->r.v2.y = ent->r.v2.y;
	
	ent1->size.x = ent->size.x;	ent1->size.y = ent->size.y;
	
	int i;
	for (i = 0; i < entity_types[ ent->ent_type ]->n_attr; ++i)
		ent1->attrs[i] = ent->attrs[i];
}






/*************************************
			TM_ENTITY Movable
*************************************/


void tm_entity_movable_init( tm_entity_movable_t* ent, int _ent_type )
{
	assert( ent && _ent_type >= 0 && _ent_type < n_entity_types );
	ent->ent_id = -1;
	ent->ent_type = _ent_type;

	tm_elem_init( &ent->e, 0 );
}

tm_entity_movable_t* tm_entity_movable_create( int _ent_type )
{
	assert( _ent_type >= 0 && _ent_type < n_entity_types );
	tm_entity_movable_t* ent = (tm_entity_movable_t*) mgr_on_new( sizeof(tm_entity_movable_t) + entity_types[_ent_type]->n_attr
			* sizeof(tm_value_t) );

	tm_entity_movable_init( ent, _ent_type );

	return ent;
}

void tm_entity_movable_destroy( tm_entity_movable_t* ent )
{
	assert( ent );
	free( ent );
}

void tm_entity_movable_generate_attrs( tm_entity_movable_t* ent )
{
	int i;
	entity_type_t* et = entity_types[ent->ent_type];
	assert( et->n_attr > 0 );
	for( i = 0; i < et->n_attr; ++i )
	{
		ent->attrs[i] = attribute_type_rand( &et->attr_types[i] );
	}
}

void tm_entity_movable_generate_position( tm_entity_movable_t* ent, rect_t* map_r )
{
	tm_rect_generate_position( &ent->r, &ent->size, map_r );
}


int tm_entity_movable_position_is_valid( tm_entity_movable_t* ent, vect_t* map_sz )
{
	vect_t aux;
	if( !vect_is_positive( &ent->size ) )			return 0;
	if( !tm_rect_is_positive( &ent->r ) )			return 0;
	if( !tm_rect_is_valid( &ent->r ) )				return 0;

	aux.x = map_sz->x - ent->r.v2.x;
	aux.y = map_sz->y - ent->r.v2.y;
	if( !vect_is_positive( &aux ) )					return 0;

	aux.x = ent->r.v2.x - ent->r.v1.x;
	aux.y = ent->r.v2.y - ent->r.v1.y;
	if( !tm_vect_is_eq( &ent->size, &aux ) )		return 0;

	return 1;
}

int tm_entity_movable_attrs_is_valid( tm_entity_movable_t* ent )
{
	int et_id = ent->ent_type;
	int i, n_attr = entity_types[et_id]->n_attr;
	for( i = 0; i < n_attr; i++ )
	{
		if( entity_types[et_id]->attr_types[i].min > ent->attrs[i] )		return 0;
		if( entity_types[et_id]->attr_types[i].max < ent->attrs[i] )		return 0;
	}
	return 1;
}

int tm_entity_movable_is_valid( tm_entity_movable_t* ent, vect_t* map_sz )
{
	if( !tm_entity_movable_position_is_valid( ent, map_sz ) )		return 0;
	if( !tm_entity_movable_attrs_is_valid( ent ) )					return 0;
	return 1;
}

void tm_entity_movable_set_attr( tm_entity_movable_t* ent, int attr_id, value_t attr_val )
{
	assert( ent && attr_id < entity_types[ent->ent_type]->n_attr );
	attribute_type_t* attr_type = &entity_types[ent->ent_type]->attr_types[attr_id];

	if( attr_type->min <= attr_val && attr_val <= attr_type->max )
		ent->attrs[attr_id] = attr_val;
	if( attr_type->max < attr_val )
		ent->attrs[attr_id] = attr_type->max;
	if( attr_val < attr_type->min )
		ent->attrs[attr_id] = attr_type->min;
}

void tm_entity_movable_pack( tm_entity_movable_t* ent, streamer_t* st )
{
	streamer_wrint( st, ent->ent_type );
	streamer_wrint( st, ent->ent_id );
	tm_rect_pack( &ent->r, st );

	assert( sizeof(value_t) == sizeof(int));
	int i, n_attr = entity_types[ent->ent_type]->n_attr;
	for( i = 0; i < n_attr; i++ )
		streamer_wrint( st, ent->attrs[i] );
}

void tm_entity_movable_unpack( tm_entity_movable_t* ent, streamer_t* st )
{
	tm_rect_unpack( &ent->r, st );
	ent->size.x = ent->r.v2.x - ent->r.v1.x;
	ent->size.y = ent->r.v2.y - ent->r.v1.y;

	assert( sizeof(value_t) == sizeof(int));
	int i, n_attr = entity_types[ent->ent_type]->n_attr;
	for( i = 0; i < n_attr; i++ )
		ent->attrs[i] = streamer_rdint( st );
}

void tm_entity_movable_pack_fixed( tm_entity_movable_t* ent, streamer_t* st )
{
	tm_rect_pack( &ent->r, st );
}

void tm_entity_movable_unpack_fixed( tm_entity_movable_t* ent, streamer_t* st )
{
	tm_rect_unpack( &ent->r, st );
	ent->size.x = ent->r.v2.x - ent->r.v1.x;
	ent->size.y = ent->r.v2.y - ent->r.v1.y;
}


void tm_entity_copy20( tm_entity_movable_t* ent2, entity_t* ent )
{
	ent2->r.v1.x = ent->r.v1.x;	ent2->r.v1.y = ent->r.v1.y;
	ent2->r.v2.x = ent->r.v2.x;	ent2->r.v2.y = ent->r.v2.y;
	
	ent2->size.x = ent->size.x;	ent2->size.y = ent->size.y;
	
	int i;
	for (i = 0; i < entity_types[ ent->ent_type ]->n_attr; ++i)
		ent2->attrs[i] = ent->attrs[i];
}



