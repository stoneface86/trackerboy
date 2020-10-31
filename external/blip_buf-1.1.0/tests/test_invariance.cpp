// Time frames and sample reads shouldn't affect generated samples

#include "blip_tester.h"

DEF_FILE( invariance ) {

int const frame_len  = 20 * oversample + oversample / 4;
int const blip_size  = (frame_len * 2) / oversample;

static void add_deltas( blip_t* b, int offset )
{
	blip_add_delta( b, frame_len/2 + offset, +1000 );
	blip_add_delta( b, frame_len   + offset + end_frame_extra * oversample, +1000 );
}

DEF_TEST( "blip_end_frame, blip_add_delta" )
{
	tester::Array<short> one( blip_size, +1 );
	tester::Array<short> two( blip_size, -1 );
	
	{
		Blip b( blip_size );
		add_deltas( b, 0 );
		add_deltas( b, frame_len );
		blip_end_frame( b, frame_len * 2 );
		assert_test( blip_read_samples( b, one, blip_size, 0 ) == blip_size );
	}
		
	{
		Blip b( blip_size );
		add_deltas( b, 0 );
		blip_end_frame( b, frame_len );
		add_deltas( b, 0 );
		blip_end_frame( b, frame_len );
		assert_test( blip_read_samples( b, two, blip_size, 0 ) == blip_size );
	}
	
	assert( one == two );
}

DEF_TEST( "blip_read_samples" )
{
	int const blip_size  = (frame_len * 3) / oversample;
	
	tester::Array<short> one( blip_size, +1 );
	tester::Array<short> two( blip_size, -1 );
	
	{
		Blip b( blip_size );
		
		add_deltas    ( b, 0 * frame_len );
		add_deltas    ( b, 1 * frame_len );
		add_deltas    ( b, 2 * frame_len );
		blip_end_frame( b, 3 * frame_len );
		
		assert_test( blip_read_samples( b, one, blip_size, 0 ) == blip_size );
	}
	
	{
		Blip b( blip_size / 3 );
		int count = 0;
		
		for ( int n = 3; n--; )
		{
			add_deltas( b, 0 );
			blip_end_frame( b, frame_len );
			count += blip_read_samples( b, two + count, blip_size - count, 0 );
		}
		
		assert_test( count == blip_size );
	}
		
	assert( one == two );
}

DEF_TEST( "blip_max_frame" )
{
	int const oversample = 32;
	int const frame_len = blip_max_frame * oversample;
	int const blip_size = frame_len / oversample * 3;
	
	tester::Array<short> one( blip_size, +1 );
	tester::Array<short> two( blip_size, -1 );
	
	{
		Blip b( blip_size );
		blip_set_rates( b, oversample, 1 );
		
		int count = 0;
		for ( int n = 3; n--; )
		{
			blip_end_frame( b, frame_len/2 );
			blip_add_delta( b, frame_len/2 + end_frame_extra * oversample, +1000 );
			blip_end_frame( b, frame_len/2 );
			count += blip_read_samples( b, one + count, blip_size - count, 0 );
		}
		assert_test( count == blip_size );
	}
	
	{
		Blip b( blip_size );
		blip_set_rates( b, oversample, 1 );
		
		int count = 0;
		for ( int n = 3; n--; )
		{
			blip_add_delta( b, frame_len + end_frame_extra * oversample, +1000 );
			blip_end_frame( b, frame_len );
		}
		count += blip_read_samples( b, two + count, blip_size - count, 0 );
		assert_test( count == blip_size );
	}
		
	assert( one == two );
}

} // DEF_FILE
