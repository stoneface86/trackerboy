#ifndef TESTER_H
#define TESTER_H

#include <stddef.h>
#include "blargg_test.h"

void tprintf( const char fmt [] ... );

namespace tester
{
	typedef void void_f();
	
	void test_func( void_f*, const char name [] );
	void test_func( void_f*, const char name [], const char file [], int line );
	
	struct Failure { };
	
	// Same as T name dimensions, but put on freestore to allow memory protection
	#define ARRAY( T, name, dimensions ) \
		T (&name) dimensions = *(new T [1] dimensions);\
		tester::Array_Deleter<T (*) dimensions> name##deleter( &name )
	
	// Fails if expr doesn't
	#define SHOULD_FAIL( expr ) \
		do {\
			try { expr; }\
			catch ( ... ) { break; }\
			assert( false );\
		}\
		while ( 0 )
	
	void checkpoint( const char file [], int line );
	
	// Test use
	void fail();
	void fail( const char str [] );
	
	// If str != NULL, prints it and fails
	void error( const char* str );
	
	// Same as error(), but records line number of failure
	#define ERROR( str ) \
		(TEST_CHECKPOINT(), tester::error_( str ))
	
	// Ignores allocations within its lifetime
	class Leak_Ignorer;
	
	int bytes_allocated();
	
	int& alloc_fail_delay();
	
	#define TEST_CHECKPOINT() \
		(tester::checkpoint( __FILE__, __LINE__ ))
	
	void clear_crc();
	unsigned calc_crc32( void const*, size_t, unsigned old_crc = 0 );
	void update_crc( void const*, size_t );
	#define CHECK_CRC( crc ) \
		(TEST_CHECKPOINT(), tester::check_crc_( crc ))
	
	#undef assert
	#define assert( expr ) \
		((TEST_CHECKPOINT(), (expr)) ? (void)0 : tester::assertion_failed( #expr ))
	
	#undef check
	#define check( expr ) \
		((TEST_CHECKPOINT(), (expr)) ? (void)0 : tester::check_failed( #expr ))
	
	#define DECL_SUITE \
		extern tester::Suite tester_suite_;\
		void run_all();\
	
	// Test suite definition
	#define DEF_SUITE \
		DECL_SUITE\
		tester::Suite tester_suite_;\
		void run_all() { tester_suite_.run(); }
	
	#define DEF_TEST2_( Struct, wrapper, obj, str ) \
		struct Struct : Suite_Fixture { Struct(); };\
		\
		static void wrapper() { Struct tester; }\
		\
		static tester::Test obj = {\
			tester::append_test_( &tester_suite_, &obj ),\
			&tester_suite_,\
			wrapper,\
			str,\
			__FILE__,\
			__LINE__\
		};
		
	#define DEF_TEST_( name, str ) \
		DEF_TEST2_( TESTER_SEMIUNIQUE(struct_,name),\
				TESTER_SEMIUNIQUE(wrap_,name),\
				 TESTER_SEMIUNIQUE(obj_,name),\
				 str )
	
	#define DEF_TEST( str ) \
		DEF_TEST_( unnamed, str )\
		TESTER_SEMIUNIQUE(struct_,unnamed)::TESTER_SEMIUNIQUE(struct_,unnamed)()
	
	#define DEF_NAMED_TEST( name ) \
		DEF_TEST_( name, #name )\
		\
		void name();\
		void name() { TESTER_SEMIUNIQUE(obj_,name).run(); }\
		\
		TESTER_SEMIUNIQUE(struct_,name)::TESTER_SEMIUNIQUE(struct_,name)()
	
	#define DEF_FILE( name ) \
		namespace tester_##name { DEF_SUITE }\
		tester::Suite& tester_file_##name();\
		tester::Suite& tester_file_##name() { return tester_##name::tester_suite_; }\
		namespace tester_##name
	
	// Master file
	#define USE_FILE( name ) \
		do {\
			extern tester::Suite& tester_file_##name();\
			tester::use_suite_( &tester_file_##name(), #name );\
		} while ( 0 )
	
	int run( const char* only_file = NULL, const char* only_test = NULL );
	void set_verbosity( int = 5 );
	void set_failure_limit( int = 1 );
}

// Implementation

// Only things needed by user or macros
namespace tester
{
	class noncopyable {
	protected:
		noncopyable() { }
	private:
		noncopyable( const noncopyable& );
		noncopyable& operator = ( const noncopyable& );
	};
	
	class Leak_Ignorer : noncopyable {
		int old;
	public:
		Leak_Ignorer();
		~Leak_Ignorer();
	};
	
	struct Suite;
	
	struct Test
	{
		Test*  next;
		Suite*      const suite;
		void_f*     const func;
		const char* const name;
		const char* const file;
		int         const line;
		
		void run() const;
	};
	
	struct Suite
	{
		Suite* next;
		const char* name;
		Test* tests;
		
		void run() const;
	};
	
	Test* append_test_( Suite*, Test* );
	int use_suite_( Suite*, const char name [] );
	void assertion_failed( const char expr [] );
	void check_crc_( unsigned );
	void error_( const char* str, const char file [], int line );
	void check_failed( const char expr [] );
	
	#define TESTER_SEMIUNIQUE2_( prefix, name, line ) tester_##prefix##name##line
	#define TESTER_SEMIUNIQUE_( prefix, name, line ) TESTER_SEMIUNIQUE2_( prefix, name, line )
	#define TESTER_SEMIUNIQUE( prefix, name ) TESTER_SEMIUNIQUE_( prefix, name, __LINE__ )
}

extern tester::Suite tester_suite_;

struct Suite_Fixture { };

#include "tester_util.h"
#include "tester_assert.h"

#endif
