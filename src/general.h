#ifndef __GENERAL_H
#define __GENERAL_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/* standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include <arpa/inet.h>

#include "utils/hrtime.h"


#define	MAX_NAME_LEN				32
#define	MAX_FILE_READ_BUFFER		1024
#define DEFAULT_DISCONNECT_TIMEOUT	10000


extern unsigned int rand_seed;

static
#ifdef INTEL_TM
[[TRANSACTION_ANNOTATION]]
#endif
inline int my_rand()
{
	rand_seed = (214013*rand_seed+2531011);
	return (rand_seed>>16)&0x7FFF;
}

//#define my_rand rand

#define rand_range( min, max )			( min + ( my_rand() % ( max - min + 1 ) ) )
#define rand_n( n )				rand_range( 0, n-1 )


#ifndef min
#define min(x,y) (((x)<(y))?(x):(y))
#endif

#ifndef max
#define max(x,y) (((x)>(y))?(x):(y))
#endif

#define pexit( str )			do{	printf( str );		exit(-1);	}while(0)
#define pexit1( str, a1 )		do{	printf( str, a1 );	exit(-1);	}while(0)


#define generic_create( obj_type, obj_init )					\
	({	obj_type* o = (obj_type*)malloc( sizeof( obj_type ) );		obj_init( o );					o;	})
#define generic_create1( obj_type, obj_init, a1 )				\
	({	obj_type* o = (obj_type*)malloc( sizeof( obj_type ) );		obj_init( o, a1 );				o;	})
#define generic_create2( obj_type, obj_init, a1, a2 )			\
	({	obj_type* o = (obj_type*)malloc( sizeof( obj_type ) );		obj_init( o, a1, a2 );			o;	})
#define generic_create3( obj_type, obj_init, a1, a2, a3 )		\
	({	obj_type* o = (obj_type*)malloc( sizeof( obj_type ) );		obj_init( o, a1, a2, a3 );		o;	})
#define generic_create4( obj_type, obj_init, a1, a2, a3, a4 )	\
	({	obj_type* o = (obj_type*)malloc( sizeof( obj_type ) );		obj_init( o, a1, a2, a3, a4 );	o;	})


#define tm_generic_create( obj_type, obj_init )					\
	({	obj_type* o = (obj_type*)mgr_on_new( sizeof( obj_type ) );		obj_init( o );					o;	})
#define tm_generic_create1( obj_type, obj_init, a1 )				\
	({	obj_type* o = (obj_type*)mgr_on_new( sizeof( obj_type ) );		obj_init( o, a1 );				o;	})
#define tm_generic_create2( obj_type, obj_init, a1, a2 )			\
	({	obj_type* o = (obj_type*)mgr_on_new( sizeof( obj_type ) );		obj_init( o, a1, a2 );			o;	})
#define tm_generic_create3( obj_type, obj_init, a1, a2, a3 )		\
	({	obj_type* o = (obj_type*)mgr_on_new( sizeof( obj_type ) );		obj_init( o, a1, a2, a3 );		o;	})
#define tm_generic_create4( obj_type, obj_init, a1, a2, a3, a4 )	\
	({	obj_type* o = (obj_type*)mgr_on_new( sizeof( obj_type ) );		obj_init( o, a1, a2, a3, a4 );	o;	})


#define generic_free( o )				({assert(o);				free( (void*)o ); o = NULL;	})
#define generic_destroy( o, obj_deinit )		({assert(o);	obj_deinit( o );	free( o ); o = NULL;	})
#define generic_destroy1( o, obj_deinit, a1 )		({assert(o);	obj_deinit( o, a1 );	free( o ); o = NULL;	})
#define generic_destroy2( o, obj_deinit, a1, a2 )	({assert(o);	obj_deinit( o, a1, a2 );free( o ); o = NULL;	})

#define tm_generic_free( o )				({assert(o);				delete( o ); o = NULL;	})
#define tm_generic_destroy( o, obj_deinit )		({assert(o);	obj_deinit( o );	delete( o ); o = NULL;	})
#define tm_generic_destroy1( o, obj_deinit, a1 )	({assert(o);	obj_deinit( o, a1 );	delete( o ); o = NULL;	})
#define tm_generic_destroy2( o, obj_deinit, a1, a2 )	({assert(o);	obj_deinit( o, a1, a2 );delete( o ); o = NULL;	})


inline static
unsigned int str_hash( const char* str )
{
	unsigned int rez = 0;
	unsigned int i;
	assert( str );
	for( i = 0; i < strlen(str); i++ )	rez += str[i];
	return rez;
}

inline static
char* str_strip( char* str )
{
	char* last;

	while( isspace( *str ) ) 	str++;
	last = str + strlen( str ) - 1;
	while( isspace( *last ) ){ 	*last = 0;last--;	}

	return str;
}


#define spf1( out, fmt, a1 )		({	sprintf( out, fmt, a1 );	out;	})
#define spf2( out, fmt, a1, a2 )	({	sprintf( out, fmt, a1, a2 );	out;	})


#ifdef INFO

#define log_info( str )			({fprintf( stderr, str );		fflush(stderr);	})
#define log_info1( str, a1 )		({fprintf( stderr, str, a1 );		fflush(stderr);	})
#define log_info2( str, a1, a2 )	({fprintf( stderr, str, a1, a2 );	fflush(stderr);	})
#define log_info3( str, a1, a2, a3 )	({fprintf( stderr, str, a1, a2, a3 );	fflush(stderr);	})

#else

#define log_info( str )
#define log_info1( str, a1 )
#define log_info2( str, a1, a2 )
#define log_info3( str, a1, a2, a3 )

#endif

#endif

