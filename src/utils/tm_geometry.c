#include "tm_geometry.h"

tm_vect_t* tm_vect_init( tm_vect_t* v, coord_t _x, coord_t _y )
{
	v->x = _x;
	v->y = _y;
	return v;
}

void tm_vect_add( tm_vect_t* v1, tm_vect_t* v2, tm_vect_t* rez )
{
	rez->x = v1->x + v2->x;
	rez->y = v1->y + v2->y;
}

void tm_vect_substract( tm_vect_t* v1, tm_vect_t* v2, tm_vect_t* rez )
{
	rez->x = v1->x - v2->x;
	rez->y = v1->y - v2->y;
}

void tm_vect_scale( tm_vect_t* v, double fx, tm_vect_t* rez )
{
	rez->x = (coord_t) (((double) v->x) * fx);
	rez->y = (coord_t) (((double) v->y) * fx);
}

int tm_vect_distance( tm_vect_t* v1, tm_vect_t* v2 )
{
	return abs( v2->x - v1->x ) + abs( v2->y - v1->y );
}

int tm_vect_distance_x( tm_vect_t* v1, tm_vect_t* v2 )
{
	return abs( v2->x - v1->x );
}

int tm_vect_distance_y( tm_vect_t* v1, tm_vect_t* v2 )
{
	return abs( v2->y - v1->y );
}

int tm_vect_direction( tm_vect_t* src_v, tm_vect_t* dest_v )
{
	vect_t dir_v;
	dir_v.x = dest_v->x - src_v->x;
	dir_v.y = dest_v->y - src_v->y;

	assert( dir_v.x * dir_v.y == 0);
	if( dir_v.x > 0 )		dir_v.x = 1;
	if( dir_v.x < 0 )		dir_v.x = -1;
	if( dir_v.y > 0 )		dir_v.y = 1;
	if( dir_v.y < 0 )		dir_v.y = -1;

	int i;
	for( i = 0; i < N_DIRS; i++ )
		if( vect_is_eq( &dir_v, &dirs[i] ) )
			break;
	assert( i < N_DIRS );

	return i;
}

void tm_vect_pack( tm_vect_t* v, streamer_t* st )
{
	coord_pack( v->x, st );
	coord_pack( v->y, st );
}

void tm_vect_unpack( tm_vect_t* v, streamer_t* st )
{
	v->x = coord_unpack( st );
	v->y = coord_unpack( st );
}




tm_rect_t* tm_rect_init( tm_rect_t* r, vect_t _v1, vect_t _v2 )
{
	assert( _v1.x <= _v2.x && _v1.y <= _v2.y );
	return tm_rect_init4( r, _v1.x, _v1.y, _v2.x, _v2.y );
}

tm_rect_t* tm_rect_init4( tm_rect_t* r, coord_t _x1, coord_t _y1, coord_t _x2, coord_t _y2 )
{
	assert( _x1 <= _x2 && _y1 <= _y2 );
	tm_vect_init( &r->v1, _x1, _y1 );
	tm_vect_init( &r->v2, _x2, _y2 );
	return r;
}

void tm_rect_generate_position( tm_rect_t* r, vect_t* sz, rect_t* map_r )
{
	assert( map_r->v2.x - map_r->v1.x >= sz->x );
	assert( map_r->v2.y - map_r->v1.y >= sz->y );
	r->v1.x = rand_range( map_r->v1.x, (map_r->v2.x - sz->x) );
	r->v1.y = rand_range( map_r->v1.y, (map_r->v2.y - sz->y) );
	r->v2.x = r->v1.x + sz->x;
	r->v2.y = r->v1.y + sz->y;
}

void tm_rect_generate_overlapping( tm_rect_t* r, tm_rect_t* overlap_r, tm_vect_t* sz, vect_t* map_sz )
{
	tm_vect_t* unit = tm_vect_create(1, 1);
	tm_rect_t aux_r;

	tm_vect_substract( &overlap_r->v1, sz, &aux_r.v1 );
	tm_vect_add( &aux_r.v1, unit, &aux_r.v1 );
	if( (int) aux_r.v1.x < 0 )
		aux_r.v1.x = 0;
	if( (int) aux_r.v1.y < 0 )
		aux_r.v1.y = 0;

	tm_vect_substract( &overlap_r->v2, unit, &aux_r.v2 );
	tm_vect_add( &aux_r.v2, sz, &aux_r.v2 );
	if( (int) aux_r.v2.x > map_sz->x )
		aux_r.v2.x = map_sz->x;
	if( (int) aux_r.v2.y > map_sz->y )
		aux_r.v2.y = map_sz->y;

	assert( rect_is_valid(&aux_r) );

	tm_vect_init( &r->v1, rand_range((int)aux_r.v1.x, (int)aux_r.v2.x), rand_range((int)aux_r.v1.y, (int)aux_r.v2.y) );
	tm_vect_add( &r->v1, sz, &r->v2 );

	tm_vect_destroy( unit );
}

void tm_rect_generate_nextto( tm_rect_t* r, tm_rect_t* nextto_r, tm_vect_t* sz, tm_vect_t* map_sz )
{
	tm_rect_t aux_r;
	int i, d, start_dir = rand_n(N_DIRS);

	for( i = 0, d = start_dir; i < N_DIRS; i++, d = (d + 1) % N_DIRS )
	{
		if( d == DIR_UP )
			tm_rect_init4( &aux_r, nextto_r->v1.x - sz->x + 1, nextto_r->v2.y, nextto_r->v2.x + sz->x - 1,
					nextto_r->v2.y + sz->y );
		if( d == DIR_DOWN )
			tm_rect_init4( &aux_r, nextto_r->v1.x - sz->x + 1, nextto_r->v1.y - sz->y, nextto_r->v2.x + sz->x - 1,
					nextto_r->v1.y );
		if( d == DIR_RIGHT )
			tm_rect_init4( &aux_r, nextto_r->v2.x, nextto_r->v1.y - sz->y + 1, nextto_r->v2.x + sz->x, nextto_r->v2.y
					+ sz->y - 1 );
		if( d == DIR_LEFT )
			tm_rect_init4( &aux_r, nextto_r->v1.x - sz->x, nextto_r->v1.y - sz->y + 1, nextto_r->v1.x, nextto_r->v2.y
					+ sz->y - 1 );

		if( (int) aux_r.v1.x < 0 )
			aux_r.v1.x = 0;
		if( (int) aux_r.v1.y < 0 )
			aux_r.v1.y = 0;
		if( (int) aux_r.v2.x > map_sz->x )
			aux_r.v2.x = map_sz->x;
		if( (int) aux_r.v2.y > map_sz->y )
			aux_r.v2.y = map_sz->y;

		if( (int) aux_r.v2.x - (int) aux_r.v1.x < (int) sz->x )
			continue;
		if( (int) aux_r.v2.y - (int) aux_r.v1.y < (int) sz->y )
			continue;

		tm_vect_substract( &aux_r.v2, sz, &aux_r.v2 );
		tm_vect_init( &r->v1, rand_range((int)aux_r.v1.x, (int)aux_r.v2.x),
				rand_range((int)aux_r.v1.y, (int)aux_r.v2.y) );
		tm_vect_add( &r->v1, sz, &r->v2 );
		break;
	}
}

unsigned int tm_rect_perimeter( tm_rect_t* r )
{
	return (unsigned int) (r->v2.x - r->v1.x + r->v2.y - r->v1.y) * 2;
}

unsigned int tm_rect_area( tm_rect_t* r )
{
	return (unsigned int) (r->v2.x - r->v1.x) * (r->v2.y - r->v1.y);
}

int tm_rect_distance( tm_rect_t* r1, tm_rect_t* r2 )
{
	int dist = 0;
	if( tm_rect_is_overlapping( r1, r2 ) )		return -1;

	if( r1->v2.x <= r2->v1.x )		dist += r2->v1.x - r1->v2.x;
	if( r1->v1.x >= r2->v2.x )		dist += r1->v1.x - r2->v2.x;
	if( r1->v2.y <= r2->v1.y )		dist += r2->v1.y - r1->v2.y;
	if( r1->v1.y >= r2->v2.y )		dist += r1->v1.y - r2->v2.y;
	return dist;
}

int tm_rect_distance_10( tm_rect_t* r1, rect_t* r2 )
{
	int dist = 0;
	if( tm_rect_is_overlapping_01( r2, r1 ) )		return -1;

	if( r1->v2.x <= r2->v1.x )		dist += r2->v1.x - r1->v2.x;
	if( r1->v1.x >= r2->v2.x )		dist += r1->v1.x - r2->v2.x;
	if( r1->v2.y <= r2->v1.y )		dist += r2->v1.y - r1->v2.y;
	if( r1->v1.y >= r2->v2.y )		dist += r1->v1.y - r2->v2.y;
	return dist;
}


void tm_rect_normalize( tm_rect_t* r )
{
	tm_rect_t out_r;
	tm_vect_init( &out_r.v1, min(r->v1.x,r->v2.x), min(r->v1.y,r->v2.y) );
	tm_vect_init( &out_r.v2, max(r->v1.x,r->v2.x), max(r->v1.y,r->v2.y) );
	*r = out_r;
}

int tm_rect_crop( tm_rect_t* r, tm_rect_t* constraint_r, tm_rect_t* crop_r )
{
	if( !tm_rect_is_overlapping( r, constraint_r ) )
		return 0;

	crop_r->v1.x = max( r->v1.x, constraint_r->v1.x );
	crop_r->v1.y = max( r->v1.y, constraint_r->v1.y );
	crop_r->v2.x = min( r->v2.x, constraint_r->v2.x );
	crop_r->v2.y = min( r->v2.y, constraint_r->v2.y );
	return 1;
}

void tm_rect_split( tm_rect_t* r, int split_dir, tm_rect_t* r_left, tm_rect_t* r_right, tm_rect_t* r_split )
{
	assert( r && r_left && r_right );
	assert( split_dir == DIR_LEFT || split_dir == DIR_RIGHT );

	int mid;
	*r_left = *r;
	*r_right = *r;
	*r_split = *r;

	if( split_dir == DIR_UP )
	{
		mid = (r->v1.x + r->v2.x) / 2;
		r_left->v2.x = r_right->v1.x = mid;
		r_split->v1.x = r_split->v2.x = mid;
	}
	if( split_dir == DIR_RIGHT )
	{
		mid = (r->v1.y + r->v2.y) / 2;
		r_left->v2.y = r_right->v1.y = mid;
		r_split->v1.y = r_split->v2.y = mid;
	}
}

/* compares 2 rectangles, returns:
 * -  0 if they overlap,
 * - -1 if "r1" is positioned lower or more to the left than "r2",
 * - +1 if "r1" is positioned higher or more to the right than "r2" */
int tm_rect_cmp( tm_rect_t* r1, tm_rect_t* r2 )
{
	if( r1->v2.x <= r2->v1.x )		return -1;
	if( r1->v1.x >= r2->v2.x )		return +1;
	if( r1->v2.y <= r2->v1.y )		return -1;
	if( r1->v1.y >= r2->v2.y )		return +1;

	return 0;
}

int tm_rect_cmp_10( tm_rect_t* r1, rect_t* r2 )
{
	if( r1->v2.x <= r2->v1.x )		return -1;
	if( r1->v1.x >= r2->v2.x )		return +1;
	if( r1->v2.y <= r2->v1.y )		return -1;
	if( r1->v1.y >= r2->v2.y )		return +1;

	return 0;
}


int tm_rect_is_overlapping( tm_rect_t* r1, tm_rect_t* r2 )
{
	if( r1->v2.x <= r2->v1.x || r1->v1.x >= r2->v2.x || r1->v2.y <= r2->v1.y || r1->v1.y >= r2->v2.y )
		return 0;
	return 1;
}


int tm_rect_is_overlapping_01( rect_t* r1, tm_rect_t* r2 )
{
	if( r1->v2.x <= r2->v1.x || r1->v1.x >= r2->v2.x || r1->v2.y <= r2->v1.y || r1->v1.y >= r2->v2.y )
		return 0;
	return 1;
}

int tm_rect_is_overlapping_10( tm_rect_t* r1, rect_t* r2 )
{
	if( r1->v2.x <= r2->v1.x || r1->v1.x >= r2->v2.x || r1->v2.y <= r2->v1.y || r1->v1.y >= r2->v2.y )
		return 0;
	return 1;
}

int tm_rect_quadrant( tm_rect_t* r1, tm_rect_t* r2 )
{
	if( tm_rect_is_overlapping( r1, r2 ) )					return 0;

	if( r1->v2.x <= r2->v1.x && r1->v1.y >= r2->v2.y )		return 1;
	if( r1->v2.x >= r2->v2.x && r1->v1.y >= r2->v2.y )		return 2;
	if( r1->v2.x <= r2->v1.x && r1->v2.y <= r2->v1.y )		return 3;
	if( r1->v1.x >= r2->v2.x && r1->v2.y <= r2->v1.y )		return 4;

	if(((r1->v1.x <= r2->v1.x && r2->v1.x <= r1->v2.x) ||
	    (r1->v1.x <= r2->v2.x && r2->v2.x <= r1->v2.x))&&
	    (r1->v1.y >= r2->v2.y)) return 5; // go down
	if(((r1->v1.x <= r2->v1.x && r2->v1.x <= r1->v2.x) ||
	    (r1->v1.x <= r2->v2.x && r2->v2.x <= r1->v2.x))&&
	    (r1->v2.y <= r2->v1.y)) return 6; // go up
	if(((r1->v1.y <= r2->v1.y && r2->v1.y <= r1->v2.y) ||
	    (r1->v1.y <= r2->v2.y && r2->v2.y <= r1->v2.y))&&
	    (r1->v1.x >= r2->v2.x)) return 7; // go left
	if(((r1->v1.y <= r2->v1.y && r2->v1.y <= r1->v2.y) ||
	    (r1->v1.y <= r2->v2.y && r2->v2.y <= r1->v2.y))&&
	    (r1->v2.x <= r2->v1.x)) return 8; // go right

	assert(0);
	return 0;
}

int tm_rect_quadrant_10( tm_rect_t* r1, rect_t* r2 )
{
	//if( tm_rect_is_overlapping_01( r2, r1 ) )				return 0;
	if( tm_rect_is_overlapping_10( r1, r2 ) )				return 0;

	if( r1->v2.x <= r2->v1.x && r1->v1.y >= r2->v2.y )		return 1;
	if( r1->v1.x >= r2->v2.x && r1->v1.y >= r2->v2.y )		return 2;
	if( r1->v2.x <= r2->v1.x && r1->v2.y <= r2->v1.y )		return 3;
	if( r1->v1.x >= r2->v2.x && r1->v2.y <= r2->v1.y )		return 4;

	if(((r1->v1.x <= r2->v1.x && r2->v1.x <= r1->v2.x) ||
	    (r1->v1.x <= r2->v2.x && r2->v2.x <= r1->v2.x))&&
	    (r1->v1.y >= r2->v2.y)) return 5; // go down
	if(((r1->v1.x <= r2->v1.x && r2->v1.x <= r1->v2.x) ||
	    (r1->v1.x <= r2->v2.x && r2->v2.x <= r1->v2.x))&&
	    (r1->v2.y <= r2->v1.y)) return 6; // go up
	if(((r1->v1.y <= r2->v1.y && r2->v1.y <= r1->v2.y) ||
	    (r1->v1.y <= r2->v2.y && r2->v2.y <= r1->v2.y))&&
	    (r1->v1.x >= r2->v2.x)) return 7; // go right
	if(((r1->v1.y <= r2->v1.y && r2->v1.y <= r1->v2.y) ||
	    (r1->v1.y <= r2->v2.y && r2->v2.y <= r1->v2.y))&&
	    (r1->v2.x <= r2->v1.x)) return 8; // go left


	assert(0);
	return 0;
}


int tm_rect_is_nextto( tm_rect_t* r1, tm_rect_t* r2 )
{
	if( tm_rect_distance( r1, r2 ) != 0 )				return 0;

	if( vect_is_eq( &r1->v2, &r2->v1 ) )				return 0;
	if( vect_is_eq( &r1->v1, &r2->v2 ) )				return 0;
	if( r1->v1.x == r2->v2.x && r1->v2.y == r2->v1.y )	return 0;
	if( r1->v2.x == r2->v1.x && r1->v1.y == r2->v2.y )	return 0;

	return 1;
}

int tm_rect_is_contained( tm_rect_t* r, tm_rect_t* cont_r )
{
	return (cont_r->v1.x <= r->v1.x && r->v2.x <= cont_r->v2.x && cont_r->v1.y <= r->v1.y && r->v2.y <= cont_r->v2.y);
}

int tm_rect_is_contained_10( tm_rect_t* r, rect_t* cont_r )
{
	return (cont_r->v1.x <= r->v1.x && r->v2.x <= cont_r->v2.x && cont_r->v1.y <= r->v1.y && r->v2.y <= cont_r->v2.y);
}


int tm_rect_can_fit_inside( tm_rect_t* r, tm_rect_t* cont_r )
{
	if( (r->v2.x - r->v1.x) > (cont_r->v2.x - cont_r->v1.x) )
		return 0;
	if( (r->v2.y - r->v1.y) > (cont_r->v2.y - cont_r->v1.y) )
		return 0;
	return 1;
}

int tm_rect_is_outside( tm_rect_t* in, tm_rect_t* out )
{
	//v1 : lower left
	//v2 : upper right

	//Out on left
	if( in->v1.x < out->v1.x )
		return 1;
	//Out on right
	if( in->v2.x > out->v2.x )
		return 1;
	//Out on bottom
	if( in->v1.y < out->v1.y )
		return 1;
	//Out on top
	if( in->v2.y > out->v2.y )
		return 1;
	//Inside
	return 0;
}

void tm_rect_pack( tm_rect_t* r, streamer_t* st )
{
	tm_vect_pack( &r->v1, st );
	tm_vect_pack( &r->v2, st );
}

void tm_rect_unpack( tm_rect_t* r, streamer_t* st )
{
	tm_vect_unpack( &r->v1, st );
	tm_vect_unpack( &r->v2, st );
}

