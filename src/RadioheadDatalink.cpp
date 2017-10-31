/********************************************************
 **
 **  RadioheadDatalink.cpp
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/

#if( defined(ARDUINO) && defined(RADIO_SUPPORT) )

#include <RadioheadDatalink.h>
#include <Debug.h>
#include <Network.h>
#include <Types.h>

#include <Wire.h>


#include <SPI.h>
#include <RH_RF69.h>

void RadioheadDatalink::initialize()
{

	if (!rf69.init())
		DLN( dDATALINK | dERROR, "radiohead init failed");
	// Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
	// No encryption
	if (!rf69.setFrequency(433.0))
		DLN( dDATALINK | dERROR, "set frequency failed");

	// If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
	// ishighpowermodule flag set like this:
	rf69.setTxPower(14, true);

	// The encryption key has to be the same as the one in the client
	uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
	                  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
	                };
	rf69.setEncryptionKey(key);
}

void RadioheadDatalink::loop()
{
	if (rf69.available())
	{
		uint8_t len = MAX_MAKERNET_FRAME_LENGTH;

		if ( rf69.recv( frameBuffer, &len) ) {
			DST( dDATALINK );
			DPR( dDATALINK, ":  ");
			DPR( dDATALINK, ">>>> RFM (" );
			DPR( dDATALINK, len );
			DPR( dDATALINK, ") ");
			hexPrint( dDATALINK, frameBuffer, len );
			DLN( dDATALINK );

			Makernet.network.handleFrame( frameBuffer, len );
		} else {
			DST( dDATALINK );
			DPR( dDATALINK | dWARNING, ": recv failed" );
		}
	}
}

// Called by upper layers to send a frame. Zero means everything OK. Negative is errors

int RadioheadDatalink::sendFrame( uint8_t *inBuffer, uint8_t len )
{
	DST( dDATALINK );
	DPR( dDATALINK, ":  ");
	DPR( dDATALINK, "<<<< RFM (" );
	DPR( dDATALINK, len );
	DPR( dDATALINK, ") ");
	hexPrint( dDATALINK, inBuffer, len );
	DLN( dDATALINK );

	bool r = rf69.send(inBuffer, len);

	DST( dDATALINK );
	DPR( dDATALINK, "send() called, return=" );
	DPR( dDATALINK, r ? "TRUE" : "FALSE" );
	DLN( dDATALINK );

	rf69.waitPacketSent();

	DST( dDATALINK );
	DLN( dDATALINK, "Sent!")
}







#endif
