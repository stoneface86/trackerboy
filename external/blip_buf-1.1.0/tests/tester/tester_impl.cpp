#include "tester_impl.h"
#undef calloc
#undef malloc
#undef realloc
#undef free
#undef assert

#include <stdlib.h>
#include <stdio.h>

static int alloc_fail_delay_;
static int blocks_allocated_;

#define ALLOC \
	if ( alloc_fail_delay_ && --alloc_fail_delay_ == 0 )\
		return NULL;\
	\
	blocks_allocated_++;\

void* blargg_calloc( size_t x, size_t y )
{
	ALLOC
	return calloc( x, y );
}

void* blargg_malloc( size_t s )
{
	ALLOC
	return malloc( s );
}

void* blargg_realloc( void* p, size_t s )
{
	ALLOC
	if ( p )
		blocks_allocated_--;
	
	return realloc( p, s );
}

void  blargg_free( void* p )
{
	if ( p )
		blocks_allocated_--;
	
	free( p );
}

int& tester::alloc_fail_delay()
{
	return alloc_fail_delay_;
}

int tester::bytes_allocated()
{
	return 0;
}

int tester::blocks_allocated()
{
	return blocks_allocated_;
}

static tester::assertion_handler handler;

void blargg_assert_failed( const char* file, int line, const char* expr )
{
	if ( handler )
		handler( file, line, expr );
	
	fprintf( stderr, "Assertion failed, \"%s\" (%d): %s\n", file, line, expr );
	abort();
}

void tester::set_assertion_handler( assertion_handler h )
{
	handler = h;
}

unsigned tester::calc_crc32( void const* in, size_t count, unsigned crc )
{
	size_t i;
	crc = ~crc & 0xFFFFFFFF;
	for ( i = 0; i < count; i++ )
	{
		int n;
		crc ^= ((unsigned char const*) in) [i];
		for ( n = 8; n--; )
			crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
	}
	return ~crc & 0xFFFFFFFF;
}

