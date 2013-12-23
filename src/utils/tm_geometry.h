#ifndef TM_GEOMETRY_H_
#define TM_GEOMETRY_H_

#include "geometry.h"

#include "tm.h"

typedef tm_type<coord_t> tm_coord_t;

typedef struct
{
  tm_coord_t x, y;
} tm_vect_t;

tm_vect_t* tm_vect_init( tm_vect_t* v, coord_t _x, coord_t _y );

#define tm_vect_create( _x, _y )    tm_generic_create2( tm_vect_t, tm_vect_init, _x, _y )
#define tm_vect_destroy( v )        tm_generic_free( v )


void tm_vect_add( tm_vect_t* v1, tm_vect_t* v2, tm_vect_t* rez );
void tm_vect_substract( tm_vect_t* v1, tm_vect_t* v2, tm_vect_t* rez );
void tm_vect_scale( tm_vect_t* v, double fx, tm_vect_t* rez );
int tm_vect_distance( tm_vect_t* v1, tm_vect_t* v2 );
int tm_vect_distance_x( tm_vect_t* v1, tm_vect_t* v2 );
int tm_vect_distance_y( tm_vect_t* v1, tm_vect_t* v2 );
int tm_vect_direction( tm_vect_t* src_v, tm_vect_t* dest_v );


#define tm_vect_is_positive( v )	( (coord_t)(v)->x >= 0 && (coord_t)(v)->y >= 0 )
#define tm_vect_is_less( v1, v2 )	( (coord_t)(v1)->x <= (coord_t)(v2)->x && (coord_t)(v1)->y <= (coord_t)(v2)->y )
#define tm_vect_is_eq( v1, v2 )		( (coord_t)(v1)->x == (coord_t)(v2)->x && (coord_t)(v1)->y == (coord_t)(v2)->y )


void tm_vect_pack( tm_vect_t* v, streamer_t* st );
void tm_vect_unpack( tm_vect_t* v, streamer_t* st );



typedef struct
{
  tm_vect_t v1, v2;
} tm_rect_t;

tm_rect_t* tm_rect_init( tm_rect_t* r, vect_t _v1, vect_t _v2 );
tm_rect_t* tm_rect_init4( tm_rect_t* r, coord_t _x1, coord_t _y1, coord_t _x2, coord_t _y2 );

#define tm_rect_create( _v1, _v2 )              tm_generic_create2( tm_rect_t, tm_rect_init, _v1, _v2)
#define tm_rect_create4( _x1, _y1, _x2, _y2 )   tm_generic_create4( tm_rect_t, tm_rect_init4, _x1, _y1, _x2, _y2 )


#define tm_rect_is_positive( r )				( tm_vect_is_positive( &(r)->v1 ) && tm_vect_is_positive( &(r)->v2 ) )
#define tm_rect_is_valid( r )					tm_vect_is_less( &(r)->v1, &(r)->v2 )


void tm_rect_generate_position( tm_rect_t* r, vect_t* sz, rect_t* map_r );
void tm_rect_generate_overlapping( tm_rect_t* r, tm_rect_t* overlap_r, vect_t* map_sz );
void tm_rect_generate_nextto( tm_rect_t* r, tm_rect_t* nextto_r, tm_vect_t* sz, vect_t* map_sz );

unsigned int tm_rect_perimeter( tm_rect_t* r );
unsigned int tm_rect_area( tm_rect_t* r );
int tm_rect_distance( tm_rect_t* r1, tm_rect_t* r2 );
int tm_rect_distance_10( tm_rect_t* r1, rect_t* r2 );

void tm_rect_normalize( tm_rect_t* r );
int tm_rect_crop( tm_rect_t* r, tm_rect_t* constraint_r, tm_rect_t* crop_r );
void tm_rect_split( tm_rect_t* r, int split_dir, tm_rect_t* r_left, tm_rect_t* r_right, tm_rect_t* r_split);
int tm_rect_cmp( tm_rect_t* r1, tm_rect_t* r2 );
int tm_rect_cmp_10( tm_rect_t* r1, rect_t* r2 );
int tm_rect_quadrant( tm_rect_t* r1, tm_rect_t* r2 );
int tm_rect_quadrant_10( tm_rect_t* r1, rect_t* r2 );

int tm_rect_is_overlapping( tm_rect_t* r1, tm_rect_t* r2 );
int tm_rect_is_overlapping_01( rect_t* r1, tm_rect_t* r2 );
int tm_rect_is_nextto( tm_rect_t* r1, tm_rect_t* r2 );
int tm_rect_is_contained( tm_rect_t* r, tm_rect_t* cont_r );
int tm_rect_is_contained_10( tm_rect_t* r, rect_t* cont_r );
int tm_rect_can_fit_inside( tm_rect_t* r, tm_rect_t* out );
int tm_rect_is_outside(tm_rect_t* in, tm_rect_t* out );

void tm_rect_pack( tm_rect_t* r, streamer_t* st );
void tm_rect_unpack( tm_rect_t* r, streamer_t* st );



#endif /*TM_GEOMETRY_H_*/
