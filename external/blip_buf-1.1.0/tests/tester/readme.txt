Simple test framework
---------------------
This is a simple test framework for writing unit tests of my libraries.
It's not polished or meant for general use by others. I looked at others
for C and C++, but they were too verbose for defining tests. This
framework makes it very minimal and automatic.

Tests are arranged in files, with a test_all.cpp that runs tests from
each file. In each file, tests are defined with DEF_TEST() and
automatically run. For example:

	// test_foo.cpp
	DEF_FILE( foo )
	{
		DEF_TEST( "something should do this" )
		{
			something();
			assert( it_did_this );
		}
		
		DEF_TEST( "else() should return 2" )
		{
			// If this fails, prints what else() returned
			assert_test( else() == 2 );
		}
	}
	
	// test_bar.cpp
	DEF_FILE( bar )
	{
		DEF_TEST( "bar() leaks" )
		{
			// If bar() leaks any memory, test framework catches it
			bar();
		}
	}
	
	// test_all.cpp
	int main()
	{
		USE_FILE( foo );
		USE_FILE( bar );
		
		// Runs all tests in foo and bar
		tester::run();
	}

When run, the file names and test names are printed.
tester::set_verbosity() will show any messages tests print (using
tprintf()).

A test generally uses assert() to verify things. assert_test() is the
same, except it prints the values of the left-hand and right-hand sides
of a comparison if it fails. SHOULD_FAIL() ensures that the expression
causes an exception or failed assertion.

A set of tests can have a fixture, an object which is created and
destroyed for each test:

	DEF_FILE( foo )
	{
		struct Suite_Fixture
		{
			int* p;
			
			Suite_Fixture() { p = new int [100]; }
			~Suite_Fixture() { delete [] p; }
		};
		
		DEF_TEST( "bla bla" )
		{
			p [99] = 123;
		}
	}

-- 
Shay Green <gblargg@gmail.com>
