// Most behavior besides synthesis

#include "blip_tester.h"

DEF_FILE( basics ) {

struct Suite_Fixture
{
	Blip b;
};

DEF_TEST( "blip_end_frame, blip_samples_avail" )
{
	blip_end_frame( b, oversample );
	assert_test( blip_samples_avail( b ) == 1 );
	
	blip_end_frame( b, oversample*2 );
	assert_test( blip_samples_avail( b ) == 3 );
}

DEF_TEST( "blip_end_frame, blip_samples_avail fractional" )
{
	blip_end_frame( b, oversample*2-1 );
	assert_test( blip_samples_avail( b ) == 1 );
	
	blip_end_frame( b, 1 );
	assert_test( blip_samples_avail( b ) == 2 );
}

DEF_TEST( "blip_end_frame limits" )
{
	blip_end_frame( b, 0 );
	assert_test( blip_samples_avail( b ) == 0 );
	
	blip_end_frame( b, blip_size*oversample + oversample - 1 );
	
	SHOULD_FAIL( blip_end_frame( b, 1 ) );
}

DEF_TEST( "blip_clocks_needed" )
{
	check_test( blip_clocks_needed( b, 0 ) == 0 * oversample );
	check_test( blip_clocks_needed( b, 2 ) == 2 * oversample );
	
	blip_end_frame( b, 1 );
	check_test( blip_clocks_needed( b, 0 ) == 0 );
	check_test( blip_clocks_needed( b, 2 ) == 2 * oversample - 1 );
}

DEF_TEST( "blip_clocks_needed limits" )
{
	SHOULD_FAIL( blip_clocks_needed( b, -1 ) );
	
	blip_end_frame( b, oversample*2 - 1 );
	assert_test( blip_clocks_needed( b, blip_size-1 ) == (blip_size-2)*oversample + 1 );
	
	blip_end_frame( b, 1 );
	SHOULD_FAIL( blip_clocks_needed( b, blip_size-1 ) );
}

DEF_TEST( "blip_clear" )
{
	blip_end_frame( b, 2*oversample - 1 );
	
	blip_clear( b );
	assert_test( blip_samples_avail( b ) == 0 );
	assert_test( blip_clocks_needed( b, 1 ) == oversample );
}

DEF_TEST( "blip_read_samples" )
{
	ARRAY( short, buf, [2] );
	buf [0] = buf [1] = -1;
	
	blip_end_frame( b, 3*oversample + oversample - 1 );
	assert_test( blip_read_samples( b, buf, 2, 0 ) == 2 );
	assert_test( buf[0] == 0 );
	assert_test( buf[1] == 0 );
	
	assert_test( blip_samples_avail( b ) == 1 );
	assert_test( blip_clocks_needed( b, 1 ) == 1 );
}

DEF_TEST( "blip_read_samples stereo" )
{
	ARRAY( short, buf, [3] );
	buf [0] = buf [1] = buf [2] = -1;
	
	blip_end_frame( b, 2*oversample );
	assert_test( blip_read_samples( b, buf, 2, 1 ) == 2 );
	assert_test( buf[0] ==  0 );
	assert_test( buf[1] == -1 );
	assert_test( buf[2] ==  0 );
}

DEF_TEST( "blip_read_samples limits to avail" )
{
	blip_end_frame( b, oversample * 2 );
	
	ARRAY( short, buf, [2] );
	buf [0] = buf [1] = -1;
	assert_test( blip_read_samples( b, buf, 3, 0 ) == 2 );
	assert_test( blip_samples_avail( b ) == 0 );
	assert_test( buf[0] == 0 );
	assert_test( buf[1] == 0 );
}

DEF_TEST( "blip_read_samples limits" )
{
	assert_test( blip_read_samples( b, NULL, 1, 0 ) == 0 );
	
	SHOULD_FAIL( blip_read_samples( b, NULL, -1, 0 ) );
}

DEF_TEST( "blip_set_rates" )
{
	blip_set_rates( b, 2, 2 );
	assert_test( blip_clocks_needed( b, 10 ) == 10 );
	
	blip_set_rates( b, 2, 4 );
	assert_test( blip_clocks_needed( b, 10 ) == 5 );
	
	blip_set_rates( b, 4, 2 );
	assert_test( blip_clocks_needed( b, 10 ) == 20 );
}

DEF_TEST( "blip_set_rates rounds sample rate up" )
{
	for ( int r = 1; r < 10000; ++r )
	{
		blip_set_rates( b, r, 1 );
		assert_test( blip_clocks_needed( b, 1 ) <= r );
	}
}

DEF_TEST( "blip_set_rates accuracy" )
{
	int const max_error = 100; // 1%
	
	for ( int r = blip_size / 2; r < blip_size; ++r )
	{
		for ( int c = r / 2; c < 8000000; c += c / 32 )
		{
			blip_set_rates( b, c, r );
			int error = blip_clocks_needed( b, r ) - c;
			assert_test( (error < 0 ? -error : error) < (c / max_error) );
		}
	}
}

DEF_TEST( "blip_set_rates high accuracy" )
{
	blip_set_rates( b, 1000000, blip_size );
	if ( blip_clocks_needed( b, blip_size ) != 1000000 )
	{
		tprintf( "skipping because 64-bit int isn't available\n" );
		return;
	}
	
	for ( int r = blip_size / 2; r < blip_size; ++r )
	{
		for ( int c = r / 2; c < 200000000; c += c / 32 )
		{
			blip_set_rates( b, c, r );
			assert_test( blip_clocks_needed( b, r ) == c );
		}
	}
}

DEF_TEST( "blip_set_rates long-term accuracy" )
{
	blip_set_rates( b, 1000000, blip_size );
	if ( blip_clocks_needed( b, blip_size ) != 1000000 )
	{
		tprintf( "skipping because 64-bit int isn't available\n" );
		return;
	}
	
	// Generates secs seconds and ensures that exactly secs*sample_rate samples
	// are generated.
	int const clock_rate  = 1789773;
	int const sample_rate = 44100;
	double const secs     = 1000;
	
	blip_set_rates( b, clock_rate, sample_rate );
	
	// speeds test greatly when using protected malloc
	int const buf_size = blip_size / 2;
	
	int const clock_size = blip_clocks_needed( b, buf_size ) - 1;
	double total_samples = 0;
	for ( double remain = clock_rate * secs;
		int n = (remain < clock_size ? (int) remain : clock_size);
		remain -= n )
	{
		blip_end_frame( b, n );
		
		short buf [buf_size];
		total_samples += blip_read_samples( b, buf, buf_size, 0 );
	}
	
	assert_test( total_samples == sample_rate * secs );
}

DEF_TEST( "blip_add_delta limits" )
{
	blip_add_delta( b, 0, 1 );
	blip_add_delta( b, (blip_size + 3)*oversample - 1, 1 );
	
	SHOULD_FAIL( blip_add_delta( b, (blip_size + 3)*oversample, 1 ) );
	SHOULD_FAIL( blip_add_delta( b, -1, 1 ) );
}

DEF_TEST( "blip_add_delta_fast limits" )
{
	blip_add_delta_fast( b, 0, 1 );
	blip_add_delta_fast( b, (blip_size + 3)*oversample - 1, 1 );
	
	SHOULD_FAIL( blip_add_delta_fast( b, (blip_size + 3)*oversample, 1 ) );
	SHOULD_FAIL( blip_add_delta_fast( b, -1, 1 ) );
}

} // DEF_FILE
