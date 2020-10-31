/* http://www.slack.net/~ant/ */

#include <string.h>
#include <stdio.h>
#include "tester_impl.h"
#include <assert.h> // wipe out our custom one

/* Library Copyright (C) 2009 Shay Green. This library is free software;
you can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
library is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

using tester::Test;
using tester::Suite;

FILE* tester::log_file;

static int failure_limit = 0;

void tester::set_failure_limit( int n )
{
	failure_limit = n;
}

void Suite::run() const
{
	for ( Test const* t = tests; t; t = t->next )
		t->run();
}

int tester::run( const char* only_file, const char* only_test )
{
	if ( only_file && !*only_file )
		only_file = NULL;
	
	if ( only_test && !*only_test )
		only_test = NULL;
	
	const char path [] = "tester.txt";
	log_file = fopen( path, "w" );
	if ( !log_file )
		throw "Couldn't open log";
	
	int count = 0;
	int failed_count = 0;
	
	try
	{
		for ( Suite const* g = suites(); g; g = g->next )
		{
			if ( only_file && strcmp( g->name, only_file ) )
				continue;
			
			for ( Test const* t = g->tests; t; t = t->next )
			{
				if ( !only_test || !strcmp( t->name, only_test ) )
				{
					Context c( g->name );
					
					for ( Test const* t = g->tests; t; t = t->next )
					{
						if ( !only_test || !strcmp( t->name, only_test ) )
						{
							count++;
							try
							{
								t->run();
							}
							catch ( tester::Failure const& )
							{
								failed_count++;
								if ( failure_limit&& failed_count >= failure_limit )
									goto stop_tests;
							}
						}
					}
					break;
				}
			}
		}
		
	stop_tests:
		if ( fclose( log_file ) )
			throw "Error writing to log";
		log_file = NULL;
		
		if ( !count )
		{
			printf( "No matching tests\n" );
			return 1;
		}
		
		printf( "\n--------\n" );
		
		printf( "%d passed\n", count - failed_count );
		
		if ( failed_count > 0 )
		{
			printf( "%d failed\n", failed_count );
		}
		else if ( only_file || only_test )
		{
			printf( "Specified tests passed\n" );
		}
		else
		{
			remove( "passed.txt" );
			rename( path, "passed.txt" );
			
			printf( "All tests passed\n" );
		}
	}
	catch ( ... )
	{
		printf( "\n********\nUnhandled exception while running tests\n" );
	}
	
	return failed_count;
}
