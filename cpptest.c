#include <stdio.h>


#define DEBUGLEVEL 0x00

#define DD( mask, f )    if( (mask & DEBUGLEVEL) > 0 ) { printf( f ); }


int main()
{
	
	DD( 0x01, "sdfsdf\n" );
}