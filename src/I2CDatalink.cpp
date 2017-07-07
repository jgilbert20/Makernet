/********************************************************
 **
 **  I2CDatalink.cpp
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/

#ifdef ARDUINO

#include <I2CDatalink.h>
#include <Debug.h>
#include <Network.h>
#include <Makernet.h>
#include <Types.h>

#define MAKERNET_BROADCAST_I2C 0x09

// Stash a copy of ourselves for the callback handlers
I2CDatalink *_datalink;

#include <Wire.h>



// Next two functions are declared static and not formally part of the class
// because I _THINK_ they have to have regular C bindings. They are called by
// the Wire library, possibly at interrupt time.

// receiveEvent() is a callback function from Wire that executes whenever data
// is received from master. This function is registered as an event, see
// setup() In I2C parlance, this is a "write" operation. (Master WRITING to
// slave). Only called in cases where we are a I2C slave.

static void I2CDatalink_receiveEvent(int howMany) {
	uint8_t p = 0;

	DPR( dDATALINK, ">>>> (");
	DPR( dDATALINK, howMany );
	DPR( dDATALINK, ") ");
	DFL( dDATALINK );

	while (Wire.available() > 0 and p < MAX_MAKERNET_FRAME_LENGTH) {
		uint8_t c = Wire.read(); // receive byte as a character
		_datalink->frameBuffer[p++] = c;
		if ( c < 0x10 )
			DPR( dDATALINK, "0" );
		DPR( dDATALINK, c, HEX );
		DPR( dDATALINK, " ");
	}

	DPR( dDATALINK,  " Actual Size:" );
	DPR( dDATALINK, p);
	DLN( dDATALINK );
	DFL( dDATALINK );

	DLN( dDATALINK, "Sending frame up to network layer" );

	_datalink->returnFrameSize = 0;
	Makernet.network.handleFrame( _datalink->frameBuffer, p );

	DLN( dDATALINK, "Frame handled" );

	if ( _datalink->returnFrameSize > 0 )
	{
		DLN( dDATALINK, "handleFrame generated a return packet!");
	}
	else
	{
		DLN( dDATALINK, "handleFrame did NOT generate a return packet, prompting framework!");
		_datalink->returnFrameSize = Makernet.network.pollFrame( _datalink->frameBuffer, MAX_MAKERNET_FRAME_LENGTH );
		if ( _datalink->returnFrameSize > 0 ) {
			DLN( dDATALINK, "No packet to send back after poll." );
		} else {
			DLN( dWARNING | dDATALINK, "WARNING: Unusual codepath: As slave, handleFrame did not generate a packet but I got one on poll");
		}
	}
}

// function that executes whenever data is requested by master this function
// is registered as an event, see setup(). In Arduino semantics, we are
// expected to fill the tx buffer which will be drained when the master clocks
// the read operation from the slave. This is called a "read" operation in I2C
// parlance (master READs from slave).
//
// This function is never used when we operate in Master mode

static void I2CDatalink_requestEvent() {

	if ( _datalink->returnFrameSize <= 0 ) {
		DLN( dDATALINK, "|||| I2C requesed a frame but none has been readied. Going silent.")
		return;
	}

	DLN( dDATALINK, "I2C read request from slave.." );

	Wire.write( _datalink->frameBuffer, _datalink->returnFrameSize );

	DPR( dDATALINK, "<<<< " );
	hexPrint( dDATALINK, _datalink->frameBuffer, _datalink->returnFrameSize );
	DLN( dDATALINK );

	// Make sure we don't send anything again if called again before next receiveEvent
	_datalink->returnFrameSize = 0;
}

// Called by upper layers to send a frame. Zero means everything OK. Negative is errors

int I2CDatalink::sendFrame( uint8_t *inBuffer, uint8_t len )
{


	if ( Makernet.network.role == Network::slave ) {
		// Slave case: store packet and send later
		if ( returnFrameSize > 0 )
			DLN( dDATALINK | dWARNING, "WARNING: Framework provided a new packet before the old one was sent!");

		if ( inBuffer != frameBuffer )
			DLN( dDATALINK | dERROR, "ERROR: Unimplemented case of an external buffer being sent in slave mode");

		DLN( dDATALINK, "Packet queued for next i2c 'read'" );

		returnFrameSize = len;

		return 0;
	} else {
		// Master sending case

		DPR( dDATALINK, "<<<< (" );
		DPR( dDATALINK, len );
		DPR( dDATALINK, ") ");
		hexPrint( dDATALINK, inBuffer, len );
		DLN( dDATALINK );

		Wire.beginTransmission(MAKERNET_BROADCAST_I2C); // transmit to device

		for ( int i = 0 ; i < len ; i++ ) {
			uint8_t c = frameBuffer[i];
			Wire.write(c);

			if ( c < 0x10 )
				DPR( dDATALINK, "0" );
			DPR( dDATALINK, c, HEX );
			DPR( dDATALINK, " ");
		}

		DLN( dDATALINK );
		DFL( dDATALINK );

		Wire.endTransmission(true);    // stop transmitting

		// Now read from the remote side...

//		DLN( dDATALINK, "Starting read...");

		uint8_t recvSize = Wire.requestFrom(9, MAX_MAKERNET_FRAME_LENGTH);    // request 6 bytes from slave device #8

		DPR( dDATALINK, ">>>> ");
		DPR( dDATALINK, " (" );
		DPR( dDATALINK, recvSize );
		DPR( dDATALINK, ") ");
		DFL( dDATALINK );

		int count = 0;

		while (Wire.available()) { // slave may send less than requested
			char c = Wire.read(); // receive a byte as character

			if ( c < 0x10 )
				DPR( dDATALINK, "0" );
			DPR( dDATALINK, c, HEX);         // print the character
			DPR( dDATALINK, " " );

			if ( count < MAX_MAKERNET_FRAME_LENGTH )
				frameBuffer[count] = c;
			count++;
		}

		DPR( dDATALINK, "READ DONE, actual" );
		DPR( dDATALINK, count );
		DLN( dDATALINK );

		// Todo better error handling?

		return 0;
	}
	return 0;
}

void I2CDatalink::loop()
{
}


void I2CDatalink::initialize()
{
	_datalink = this;

	if ( Makernet.network.role == Network::slave ) {
		Wire.begin(MAKERNET_BROADCAST_I2C);                // join i2c bus with address
		Wire.onReceive(I2CDatalink_receiveEvent); // register event
		Wire.onRequest(I2CDatalink_requestEvent); // register event
	}
	else
	{
		Wire.begin();
	}

}




#endif
