// Implements assert_test()

#ifndef TESTER_ASSERT_H
#define TESTER_ASSERT_H

#include "tester.h"

namespace assert_test {

void print_unknown();

template<typename T>
inline void print_t( const T& )
{
	print_unknown();
}

void print_t( char );
void print_t( long );
void print_t( unsigned long );
void print_t( long double );
void print_null();
void print_t( const char* );
void print_t( const void* );
void print_t( tester::string const& );

#define DEF_PRINT_T( U, T ) \
	inline void print_t( T t )\
	{\
		print_t( static_cast<U> (t) );\
	}

DEF_PRINT_T( long, signed char )
DEF_PRINT_T( long, short )
DEF_PRINT_T( long, int )

DEF_PRINT_T( unsigned long, unsigned char )
DEF_PRINT_T( unsigned long, unsigned short )
DEF_PRINT_T( unsigned long, unsigned int )

DEF_PRINT_T( long double, float )
DEF_PRINT_T( long double, double )

#undef DEF_PRINT_T

template<typename T>
struct LHS
{
	const T& t;
	
	LHS( const T& t ) : t( t ) { }
	operator T () const { return t; }
};

template<typename T>
struct LHSP
{
	const T& t;
	
	LHSP( const T& t ) : t( t ) { }
	operator T () const { return t; }
};

template<typename T>
bool test_( const T& t, bool b )
{
	if ( !b )
	{
		tprintf( "LHS: " );
		print_t( t );
		tprintf( "\nRHS: " );
	}
	return b;
}

template<typename T,typename U>
bool test( const T& t, bool b, const U& u )
{
	b = test_( t, b );
	if ( !b )
	{
		print_t( u );
		tprintf( "\n" );
	}
	return b;
}

#define DEF_OP( op ) \
	template<typename T,typename U>\
	inline bool operator op ( const LHS<T>& lhs, const U& rhs )\
	{\
		return test( lhs.t, lhs.t op rhs, rhs );\
	}\
	\
	template<typename T,typename U>\
	inline bool operator op ( const LHSP<T>& lhs, const U* rhs )\
	{\
		return test( lhs.t, lhs.t op rhs, rhs );\
	}\
	\
	template<typename T>\
	inline bool operator op ( const LHSP<T>& lhs, const void* rhs )\
	{\
		return test( lhs.t, lhs.t op rhs, rhs );\
	}

DEF_OP( == )
DEF_OP( != )
DEF_OP( <  )
DEF_OP( <= )
DEF_OP( >  )
DEF_OP( >= )

#undef DEF_OP

enum Args { args = 1 };

template<typename T>
struct LHS_type {
	typedef LHS<T> type;
};

template<typename T>
struct LHS_type<T*> {
	typedef LHSP<T*> type;
};

template<typename T>
struct LHS_type<const T*> {
	typedef LHSP<const T*> type;
};

template<typename T>
inline typename LHS_type<T>::type operator * ( Args, const T& t )
{
	return t;
}

#define assert_test( expr ) \
	(tester::checkpoint( __FILE__, __LINE__ ),\
	(assert_test::args * expr) ? (void) 0 : tester::assertion_failed( #expr ))

#define check_test( expr ) \
	(tester::checkpoint( __FILE__, __LINE__ ),\
	(assert_test::args * expr) ? (void) 0 : tester::check_failed( #expr ))

} // namespace

#endif
