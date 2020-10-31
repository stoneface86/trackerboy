#include "tester.h"

void assert_test::print_unknown()
{
	tprintf( "-" );
}

void assert_test::print_t( char c )
{
	tprintf( "%c", c );
}

void assert_test::print_t( long i )
{
	tprintf( "%ld", i );
}

void assert_test::print_t( unsigned long i )
{
	tprintf( "%lu", i );
}

void assert_test::print_t( long double f )
{
	tprintf( "%Lf", f );
}

void assert_test::print_null()
{
	tprintf( "(null)" );
}

void assert_test::print_t( const char* s )
{
	if ( s )
	{
		for ( int i = 0; *s; ++i )
		{
			tprintf( "%c", *s++ );
			if ( i == 75 && *s )
			{
				tprintf( "..." );
				break;
			}
		}
	}
	else
	{
		print_null();
	}
}

void assert_test::print_t( tester::string const& s )
{
	print_t( s.c_str() );
}

void assert_test::print_t( const void* p )
{
	if ( p )
		tprintf( "%p", p );
	else
		print_null();
}
