// Useful things to go along with test, but that are separate implementation-wise

#ifndef TESTER_UTIL_H
#define TESTER_UTIL_H

#include "tester.h"

// Put around C string and it puts it in memory for the duration of the statement,
// allowing detection of accesses outside string.
#define STR( s ) (tester::Checked_Str( s ).operator const char*())

// Implementation
namespace tester
{
	class Checked_Str : noncopyable {
		char* str;
	public:
		Checked_Str( const char [] );
		~Checked_Str();
		
		operator const char* () const { return str; }
	};
	
	class Temp_Nofail : noncopyable {
		int const old_count;
	public:
		Temp_Nofail() : old_count( alloc_fail_delay() ) { alloc_fail_delay() = 0; }
		~Temp_Nofail() { alloc_fail_delay() = old_count; }
	};
	
	class Alloc_Failer : noncopyable {
		int iter;
		int const old_delay;
	public:
		Alloc_Failer()          : old_delay( alloc_fail_delay() ) { iter = 0; }
		~Alloc_Failer()         { alloc_fail_delay() = old_delay; }
		
		bool done();
		bool failed() const     { return alloc_fail_delay() == 0; }
	};
	
	template<typename T>
	class Array_Deleter {
		T const t;
	public:
		Array_Deleter( T t ) : t( t ) { }
		~Array_Deleter() { delete [] t; }
	};
	
	class Logger : noncopyable {
		size_t const begin;
	public:
		Logger();
		operator const char* () const;
		bool equals( const char* s ) const;
	};
	
	template<typename T>
	class Array {
		T* const t;
		size_t const size_;
	public:
		Array( size_t size, const T& fill = T() ) :
			t( new T [size] ),
			size_( size )
		{
			for ( size_t i = 0; i < size; ++i )
				t [i] = fill;
		}
		
		~Array() { delete [] t; }
		
		operator T* () const { return t; }
		size_t size() const { return size_; }
		T& operator [] ( size_t i ) const { assert( i < size_ ); return t [i]; }
	};
	
	template<typename T>
	bool operator == ( const Array<T>& x, const Array<T>& y )
	{
		if ( x.size() != y.size() )
			return false;
		
		for ( size_t i = 0; i < x.size(); ++i )
			if ( x [i] != y [i] )
				return false;
		
		return true;
	}
	
	template<typename T>
	inline bool operator != ( const Array<T>& x, const Array<T>& y )
	{
		return !(x == y );
	}
	
	class string {
		const char* const s;
	public:
		string( const char* s ) : s( s ) { }
		const char* c_str() const { return s; }
	};
	
	bool operator == ( string const& lhs, string const& rhs );
	
	inline
	bool operator != ( string const& lhs, string const& rhs ) { return !(lhs == rhs); }
}

#endif
