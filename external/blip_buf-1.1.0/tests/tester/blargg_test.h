#include <stddef.h>

#ifdef __cplusplus
	extern "C" {
#endif

void* blargg_calloc( size_t, size_t );
void* blargg_malloc( size_t );
void* blargg_realloc( void*, size_t );
void  blargg_free( void* );

#define calloc( x, y )          blargg_calloc( x, y )
#define malloc( size )          blargg_malloc( size )
#define realloc( old, size )    blargg_realloc( old, size )
#define free( p )               blargg_free( p )

void blargg_assert_failed( const char* file, int line, const char* expr );

#undef assert
#define assert( expr ) \
	((expr) ? (void) 0 : blargg_assert_failed( __FILE__, __LINE__, #expr ))

#ifdef __cplusplus
	}
#endif
