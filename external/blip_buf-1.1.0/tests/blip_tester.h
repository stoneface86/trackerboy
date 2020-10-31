#include "blip_buf.h"
#include "tester.h"

int const oversample = blip_max_ratio;
int const blip_size  = blip_max_frame / 2;

enum { end_frame_extra = 2 }; /* allows deltas slightly after frame length */

class Blip {
	blip_t* const b;
public:
	Blip( int size = blip_size ) : b( blip_new( size ) ) { assert( b ); }
	~Blip() { blip_delete( b ); }
	
	operator blip_t* () const { return b; }
};
