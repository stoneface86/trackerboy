#include "blip_tester.h"

DEF_FILE( synthesis ) {

int const blip_size = 32;

struct Suite_Fixture
{
	Blip b;
	tester::Array<short> buf;
	tester::Array<short> stereo_buf;
	
	Suite_Fixture() :
		b( blip_size ),
		buf( blip_size ),
		stereo_buf( blip_size * 2 )
	{ }
	
	void end_frame_and_dump_buf_deltas()
	{
		blip_end_frame( b, blip_size * oversample );
		blip_read_samples( b, buf, blip_size, 0 );
		for ( int i = 1; i < blip_size; ++i )
			tprintf( "%d ", buf [i] - buf [i-1] );
		tprintf( "\n" );
		blip_clear( b );
	}
};

DEF_TEST( "blip_add_delta_fast, blip_read_samples" )
{
	blip_add_delta_fast( b, 2 * oversample, +16384 );
	end_frame_and_dump_buf_deltas();
	CHECK_CRC( 0x4FF00AB2 );
	
	blip_add_delta_fast( b, int (2.5 * oversample), +16384 );
	end_frame_and_dump_buf_deltas();
	CHECK_CRC( 0x7B871E92 );
}

DEF_TEST( "blip_add_delta tails" )
{
	blip_add_delta( b, 0, +16384 );
	end_frame_and_dump_buf_deltas();
	CHECK_CRC( 0x8DE789B2 );
	
	blip_add_delta( b, oversample/2, +16384 );
	end_frame_and_dump_buf_deltas();
	CHECK_CRC( 0x3BD3F8BF );
}

DEF_TEST( "blip_add_delta interpolation" )
{
	blip_add_delta( b, oversample/2, +32768 );
	end_frame_and_dump_buf_deltas();
	
	// Values should be half-way between values for above and below
	blip_add_delta( b, oversample/2 + oversample/64, +32768 );
	end_frame_and_dump_buf_deltas();
	
	blip_add_delta( b, oversample/2 + oversample/32, +32768 );
	end_frame_and_dump_buf_deltas();
	
	CHECK_CRC( 0x2593B066 );
}

static int test_saturation( int delta )
{
	Suite_Fixture f;
	blip_add_delta_fast( f.b, 0, delta );
	blip_end_frame( f.b, oversample * blip_size );
	blip_read_samples( f.b, f.buf, blip_size, 0 );
	return f.buf [20];
}

DEF_TEST( "saturation" )
{
	check_test( test_saturation( +35000 ) == +32767 );
	check_test( test_saturation( -35000 ) == -32768 );
}

DEF_TEST( "stereo interleave" )
{
	blip_add_delta( b, 0, +16384 );
	blip_end_frame( b, blip_size * oversample );
	blip_read_samples( b,        buf, blip_size, 0 );
	
	blip_clear( b );
	blip_add_delta( b, 0, +16384 );
	blip_end_frame( b, blip_size * oversample );
	blip_read_samples( b, stereo_buf, blip_size, 1 );
	
	for ( int i = 0; i < blip_size; ++i )
		assert( stereo_buf [i*2] == buf [i] );
}

DEF_TEST( "blip_clear" )
{
	// Make first and last internal samples non-zero
	blip_add_delta( b, 0, 32768 );
	blip_add_delta( b, (blip_size+2)*oversample+oversample/2, 32768 );
	
	blip_clear( b );
	
	for ( int n = 2; n--; )
	{
		blip_end_frame( b, blip_size * oversample );
		assert_test( blip_read_samples( b, buf, blip_size, 0 ) == blip_size );
		for ( int i = 0; i < blip_size; ++i )
			assert_test( buf [i] == 0 );
	}
}

} // DEF_FILE
