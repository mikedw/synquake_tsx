#ifndef __TM_H__
#define __TM_H__

#include "tm_general.h"
#include "tm_scope.h"
#include "tm_threads.h"

#define end_tx() do {} while(0)

#ifdef INTEL_TM
# define BEGIN_TRANSACTION();		__transaction[[relaxed]]{
# define END_TRANSACTION();		}end_tx()
#else
# define TM_INIT();                    \
  static Mutex tm_mutex;

# define BEGIN_TRANSACTION();     \
   Transaction tm_guard(tm_mutex, 5); \
   tm_guard.TransactionStart();

# define END_TRANSACTION();		tm_guard.TransactionEnd();

#endif

TM_INIT();

static void* mgr_on_new( size_t size )
{
	void* ptr = malloc(size);
	memset( ptr, 0, size );

	return ptr;
}

class tm_obj
{
	public:
		//[[transaction_callable]]
		void* operator new(size_t size)		{	return mgr_on_new( size );	}

		//[[transaction_callable]]
		void* operator new[](size_t size)	{	return mgr_on_new( size );	}

		//[[transaction_callable]]
		void  operator delete(void* ptr)	{	free( ptr );		}

		//[[transaction_callable]]
		void  operator delete[](void* ptr)	{	free( ptr );		}
};



/**************************************************************************
 *		TM_TYPE
 **************************************************************************/

#define TX_WRITE(x,y) (x) = (y)
#define TX_READ(x) (x)

template <typename T>
class tm_type : public tm_obj
{
	typedef tm_type<T> tm_T;
public:
    T	_t;

    tm_type() 				{}

    tm_type( T const& r) : _t( r )	{}

    //[[transaction_callable]]
    operator volatile T () const volatile	{return TX_READ(_t);}

    //[[transaction_callable]]
    T& operator = (    T const&    r )	{return (TX_WRITE(_t, r));			}

    //[[transaction_callable]]
    T& operator = ( tm_T const& tm_r )	{return ( TX_WRITE(_t, TX_READ(tm_r._t)));	}

    //[[transaction_callable]]
    T& operator ++ ()			{T tt = TX_READ(_t);return ( TX_WRITE(_t, tt + 1 ));}

    //[[transaction_callable]]
    T& operator -- ()			{T tt = TX_READ(_t);return ( TX_WRITE(_t, tt - 1 ));}

    //[[transaction_callable]]
    T operator ++ (int)			{T tt = TX_READ(_t);return ( TX_WRITE(_t, tt + 1 )); return tt;}

    //[[transaction_callable]]
    T operator -- (int)			{T tt = TX_READ(_t);return ( TX_WRITE(_t, tt - 1 )); return tt;}

	/**********/
    //[[transaction_callable]]
	T&  operator += (   T const&    r ){	T tt = TX_READ(_t);return ( TX_WRITE(_t, tt + r ));}

    //[[transaction_callable]]
	T&  operator -= (   T const&    r ){	T tt = TX_READ(_t);return ( TX_WRITE(_t, tt - r ));}

    //[[transaction_callable]]
	T&  operator *= (   T const&    r ){	T tt = TX_READ(_t);return ( TX_WRITE(_t, tt * r ));}

    //[[transaction_callable]]
	T&  operator /= (   T const&    r ){	T tt = TX_READ(_t);return ( TX_WRITE(_t, tt / r ));}

	/**********/
    //[[transaction_callable]]
	T&   operator += (tm_T const& tm_r ){	T tt = TX_READ(_t);return ( TX_WRITE(_t, tt + TX_READ(tm_r._t) ));}

    //[[transaction_callable]]
	T&   operator -= (tm_T const& tm_r ){	T tt = TX_READ(_t);return ( TX_WRITE(_t, tt - TX_READ(tm_r._t) ));}

    //[[transaction_callable]]
	T&   operator *= (tm_T const& tm_r ){	T tt = TX_READ(_t);return ( TX_WRITE(_t, tt * TX_READ(tm_r._t) ));}

    //[[transaction_callable]]
	T&   operator /= (tm_T const& tm_r ){	T tt = TX_READ(_t);return ( TX_WRITE(_t, tt / TX_READ(tm_r._t) ));}

};


/**************************************************************************
 *		TYPE REDEFINITIONS
 **************************************************************************/


typedef	tm_type<char>			tm_char;
typedef	tm_type<short>			tm_short;
typedef	tm_type<int>			tm_int;
typedef	tm_type<long>			tm_long;
typedef	tm_type<long long>		tm_llong;

typedef	tm_type<unsigned char>		tm_uchar;
typedef	tm_type<unsigned short>		tm_ushort;
typedef	tm_type<unsigned int>		tm_uint;
typedef	tm_type<unsigned long>		tm_ulong;
typedef	tm_type<unsigned long long>	tm_ullong;

typedef	tm_type<float>			tm_float;
typedef	tm_type<double>			tm_double;


#endif	// __TM_H__
