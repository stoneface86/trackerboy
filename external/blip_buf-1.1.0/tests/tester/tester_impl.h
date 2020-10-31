#ifndef TESTER_IMPL_H
#define TESTER_IMPL_H

#include <stdio.h>
#include "tester.h"

namespace tester
{
	Suite* suites();
	const char* context();
	void clear_crc_();
	
	extern FILE* log_file;
	
	struct Excessive_Failures { };
	
	void suspend_context( bool );
	
	int blocks_allocated();
	
	typedef void (*assertion_handler)( const char file [], int line, const char expr [] );
	
	void set_assertion_handler( assertion_handler );
	
	class Context : noncopyable {
		size_t const old_size;
		size_t const old_log_size;
		int const old_indention;
		int const old_suspended;
	public:
		Context( const char str [] );
		~Context();
	};
}

#endif
