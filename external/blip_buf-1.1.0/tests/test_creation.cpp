// Buffer creation and initial state

#include <stdlib.h>
#include "blip_tester.h"

DEF_FILE( creation ) {

struct Suite_Fixture
{
	Blip b;
};

DEF_TEST( "blip_delete" )
{
	// fixture will create and delete buffer
}

DEF_TEST( "free() in place of blip_delete()" )
{
	free( blip_new( blip_size ) );
}
/*
// Requires that protected malloc be turned off
DEF_TEST( "blip_delete clears fields" )
{
	blip_t* b = blip_new( blip_size );
	blip_end_frame( b, blip_size * oversample );
	assert( blip_samples_avail( b ) );
	blip_delete( b );
	assert( !blip_samples_avail( b ) );
}
*/

DEF_TEST( "blip_new malloc failure" )
{
	for ( tester::Alloc_Failer af; !af.done(); )
	{
		blip_t* b = blip_new( blip_size );
		assert( !b == af.failed() );
		blip_delete( b );
	}
}

DEF_TEST( "blip_new limits" )
{
	blip_delete( blip_new( 0 ) );
	
	SHOULD_FAIL( blip_new( -1 ) );
}

DEF_TEST( "blip_new empty on creation" )
{
	assert( blip_samples_avail( b ) == 0 );
}

DEF_TEST( "default ratio" )
{
	assert_test( blip_clocks_needed( b, 1 ) == +blip_max_ratio );
}

DEF_TEST( "blip_new size" )
{
	// add as late as possible
	blip_add_delta_fast( b, (blip_size + 3) * blip_max_ratio - 1, 32768 );
}

DEF_TEST( "blip_new malloc size" )
{
	// modify last sample in internal buffer
	blip_add_delta( b, (blip_size + 2) * blip_max_ratio, 32768 );
}

} // DEF_FILE
