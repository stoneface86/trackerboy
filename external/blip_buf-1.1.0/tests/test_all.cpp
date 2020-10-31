#include "tester.h"

int main()
{
	USE_FILE( creation );
	USE_FILE( basics );
	USE_FILE( synthesis );
	USE_FILE( invariance );
	
	tester::run( "", "" );
}
