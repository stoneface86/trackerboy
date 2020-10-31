#include "tester.h"
#include <string.h>
#include <assert.h> // wipe out our custom one

//// Checked_Str

using tester::Checked_Str;

Checked_Str::Checked_Str( const char s [] )
{
	str = NULL;
	if ( s )
	{
		size_t size = strlen( s ) + 1;
		str = new char [size];
		memcpy( str, s, size );
	}
}

Checked_Str::~Checked_Str()
{
	delete [] str;
}

void tester::error( const char* str )
{
	if ( str )
	{
		tprintf( "Error: %s\n", str );
		fail();
	}
}

bool tester::Alloc_Failer::done()
{
	if ( iter && alloc_fail_delay() > 0 )
		return true;
	
	alloc_fail_delay() = ++iter;
	return false;
}

bool tester::operator == ( string const& lhs, string const& rhs )
{
	return !strcmp( lhs.c_str(), rhs.c_str() );
}
