#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include "tester_impl.h"
#include <assert.h> // wipe out our custom one

namespace
{
	std::string tprintf_log;
	std::string context_str;
	size_t starting_size;
	int indention;
	int verbosity = 2;
	bool needs_newline;
	unsigned crc;
	int suspended;
	
	void append_str( const char s [] )
	{
		tprintf_log += s;
		
		while ( *s )
		{
			if ( context_str.size() == 0 || context_str [context_str.size() - 1] == '\n' )
			{
				for ( int n = indention; n--; )
					context_str += '\t';
			}
			
			do
			{
				int c = *s++;
				context_str += c;
				if ( c == '\n' )
					break;
			}
			while ( *s );
		}
	}
	
	void rollback()
	{
		assert( starting_size <= context_str.size() );
		context_str.resize( starting_size );
	}
}

void tester::clear_crc_()
{
	rollback();
	crc = 0;
}

void tester::clear_crc()
{
	tprintf( "\n" );
	clear_crc_();
}

void tester::update_crc( void const* p, size_t s )
{
	crc = calc_crc32( p, s, crc );
}

void tester::check_crc_( unsigned correct )
{
	if ( crc != correct )
	{
		tprintf( "\nCRC: 0x%08X\n", crc );
		fail( "CRC differed" );
	}
	
	clear_crc();
}

void tester::set_verbosity( int n )
{
	verbosity = n;
}

void tester::suspend_context( bool b )
{
	suspended += (b ? +1 : -1);
}

void tprintf( const char fmt [] ... )
{
	tester::Leak_Ignorer li;
	tester::Temp_Nofail nf;
	
	va_list ap;
	va_start( ap, fmt );
	char str [256];
	vsprintf( str, fmt, ap );
	va_end( ap );
	
	tester::update_crc( str, strlen( str ) );
	
	size_t old_size = context_str.size();
	append_str( str );
	
	if ( !suspended )
	{
		const char* s = context_str.c_str() + old_size;
		
		if ( tester::log_file )
			fprintf( tester::log_file, "%s", s );

		if ( indention < verbosity )
		{
			// Restore final newline
			if ( needs_newline )
			{
				needs_newline = false;
				putchar( '\n' );
			}
			
			// Withold final newline
			int len = strlen( s );
			if ( len && s [len - 1] == '\n' )
			{
				len--;
				needs_newline = true;
			}
			
			printf( "%.*s", len, s );
		}
		
		// Flush periodically
		clock_t present = clock();
		static clock_t next_time = present;
		if ( present > next_time )
		{
			next_time = present + CLOCKS_PER_SEC;
			
			if ( indention >= verbosity )
				putchar( '.' );
			
			fflush( stdout );
		}
	}
}

tester::Context::Context( const char str [] ) :
	old_size( context_str.size() ),
	old_log_size( tprintf_log.size() ),
	old_indention( indention ),
	old_suspended( suspended )
{
	if ( str )
	{
		tprintf( "%s\n", str );
		indention++;
	}
	starting_size = context_str.size();
}

tester::Context::~Context()
{
	indention = old_indention;
	suspended  = old_suspended;
	
	starting_size = old_size;
	context_str.resize( old_size );
	tprintf_log.resize( old_log_size );
}

const char* tester::context()
{
	return context_str.c_str();
}

tester::Logger::Logger() :
	begin( tprintf_log.size() )
{ }

tester::Logger::operator const char* () const
{
	assert( tprintf_log.size() >= begin );
	return tprintf_log.c_str() + begin;
}

bool tester::Logger::equals( const char* s ) const
{
	return !strcmp( *this, s );
}
