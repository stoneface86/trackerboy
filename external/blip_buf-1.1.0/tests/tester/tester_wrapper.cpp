#include <stdio.h>
#include <string>
#include <stdlib.h>
#include "tester_impl.h"
#include <assert.h> // wipe out our custom one

namespace
{
	struct state_t {
		const char* file;
		int         line;
		bool        running;
		bool        check_failed;
	};
	
	state_t state;
	
	struct State_Saver {
		state_t const old;
		State_Saver();
		~State_Saver();
	};
	
	State_Saver::State_Saver() :
		old( state )
	{
		static state_t const zero = { };
		state = zero;
	}
	
	State_Saver::~State_Saver()
	{
		state = old;
	}
	
	void print_context()
	{
		printf( "%s\n", tester::context() );
	}
	
	void print_exception()
	{
		const char* file = state.file;
		int         line = state.line;
		
		try
		{
			throw;
		}
		catch ( tester::Failure const& e )
		{
			print_context();
			printf( "Failed\n" );
		}
		catch ( std::exception const& e )
		{
			print_context();
			printf( "std::exception: %s\n", e.what() );
		}
		catch ( const char* e )
		{
			print_context();
			printf( "exception: %s\n", e );
		}
		catch ( ... )
		{
			print_context();
			printf( "Unknown exception\n" );
		}
		
		printf( "%s (%d)\n", file, line );
	}
	
	int leak_delta;
	
	int leak_count()
	{
		return tester::blocks_allocated() + leak_delta;
	}
}

void tester::fail()
{
	alloc_fail_delay() = 0;
	throw Failure();
}

void tester::fail( const char expr [] )
{
	if ( expr )
		tprintf( "%s\n", expr );
	
	fail();
}

void tester::assertion_failed( const char expr [] )
{
	if ( state.running )
		fail( expr );
	
	fprintf( stderr, "Assertion failed %s (%d): %s\n",
			(state.file ? state.file : ""), state.line, expr );
	abort();
}

void tester::check_failed( const char expr [] )
{
	printf( "\n********\n" );
	print_context();
	printf( "Failed: check( %s )\n", expr );
	printf( "%s (%d)\n\n", state.file, state.line );
	printf( "********\n" );
	
	state.check_failed = true;
}

void tester::Test::run() const
{
	test_func( func, name, file, line );
}

tester::Leak_Ignorer::Leak_Ignorer() :
	old( leak_count() )
{ }

tester::Leak_Ignorer::~Leak_Ignorer()
{
	leak_delta -= leak_count() - old;
}

void tester::test_func( void_f* func, const char name [] )
{
	test_func( func, name, state.file, state.line );
}

static void std_assertion_failed( const char file [], int line, const char expr [] )
{
	tprintf( "assert failed in %s (%d)\n", file, line );
	tester::assertion_failed( NULL );
}

void tester::checkpoint( const char file [], int line )
{
	state.file = file;
	state.line = line;
}

void tester::test_func( void_f* func, const char name [], const char file [], int line )
{
	Leak_Ignorer li;
	Context c( name );
	State_Saver ss;
	
	state.file = file;
	state.line = line;
	
	set_assertion_handler( std_assertion_failed );
	
	try
	{
		for ( int phase = 2; phase--; )
		{
			int old_count = leak_count();
			
			clear_crc_();
			state.running = true;
			func();
			state.running = false;
			alloc_fail_delay() = 0;
			if ( state.check_failed )
				goto already_failed;
			
			int leaks = leak_count() - old_count;
			if ( leaks != 0 )
			{
				if ( phase )
				{
					tprintf( "Rerunning due to possible leaks\n" );
					suspend_context( true );
					continue;
				}
				
				suspend_context( false );
				tprintf( "Leaked %d blocks\n", leaks );
				fail();
			}
			break;
		}
	}
	catch ( ... )
	{
		printf( "\n********\n" );
		print_exception();
		printf( "********\n" );
		
		throw Failure();
	}
	
	return;
already_failed:
		throw Failure();
}
