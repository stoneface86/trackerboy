#include <stddef.h>
#include "tester_impl.h"
#include <assert.h> // wipe out our custom one

using tester::Test;
using tester::Suite;

tester::Suite tester_suite_;

namespace
{
	Suite* suites_;
	
	template<typename T>
	void append( T*& list, T* item )
	{
		if ( !list )
		{
			list = item;
		}
		else
		{
			T* f = list;
			while ( f->next )
				f = f->next;
		
			f->next = item;
		}
	}
}

Suite* tester::suites()
{
	return suites_;
}

Test* tester::append_test_( Suite* g, Test* t )
{
	append( g->tests, t );
	return NULL;
}

int tester::use_suite_( Suite* g, const char name [] )
{
	assert( !g->name );
	g->name = name;
	append( suites_, g );
	return 0;
}
