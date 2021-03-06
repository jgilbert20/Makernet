/********************************************************
 **
 **  UNIXSocketDatalink.cpp
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/


#ifndef ARDUINO

#include <UNIXSocketDatalink.h>
#include <Debug.h>
#include <Network.h>
#include <Makernet.h>
#include <Types.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/time.h>
#include <poll.h>


void UNIXSocketDatalink::initialize()
{
	int t, len;

	char str[100];

	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	DLN( dDATALINK, "Trying to connect...");

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(sock, (struct sockaddr *)&remote, len) == -1) {
		perror("connect");
		exit(1);
	}

	DLN( dDATALINK, "Connected.");

	returnFrameSize = 0;

}

typedef struct { uint8_t value[5] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; } ThunkMessage;

ThunkMessage thunk;

// Called by upper layers to send a frame. Zero means everything OK. Negative
// is errors

int UNIXSocketDatalink::sendFrame( uint8_t *inBuffer, uint8_t len )
{
	if ( Makernet.network.role == Network::slave ) {
		// Slave case: store packet and send later
		if ( returnFrameSize > 0 )
			DLN( dDATALINK | dWARNING, "WARNING: Framework provided a new packet before the old one was sent!");

		if ( inBuffer != frameBuffer )
			DLN( dDATALINK | dERROR, "ERROR: Unimplemented case of an external buffer being sent in slave mode");

		DLN( dDATALINK, "Packet queued for next 'read'" );

		returnFrameSize = len;
		memcpy( returnFrameBuffer, inBuffer, len );
		return 0;
	}

	// Else we are the master

	sendToWire( inBuffer, len );


	// Emit the "thunk". If we are a master, we need to emulate the idea of
	// opening the bus for responses the same way that I2C works when the
	// master holds the clock line after its finished sending. This happens by
	// emitting a 5 byte FF signature.

	if ( Makernet.network.role == Network::master ) {
		int len = sizeof( thunk );
		sendToWire( (uint8_t *)&thunk, len );
	}

	return 0;
}

void UNIXSocketDatalink::sendToWire( uint8_t *inBuffer, int len )
{
	// Send the length

	if (send(sock, &len, 1, 0) == -1) {
		perror("send");
		exit(1);
	}

	// Send the packet

	if (send(sock, inBuffer, len, 0) == -1) {
		perror("send");
		exit(1);
	}

	DPR( dDATALINK, "<<<< (" );
	DPR( dDATALINK, len );
	DPR( dDATALINK, ") ");
	hexPrint( dDATALINK, inBuffer, len );
	DLN( dDATALINK );

}

void UNIXSocketDatalink::processIncomingFrame()
{
	int t;

	uint8_t frameLen;

	t = recv(sock, &frameLen, 1, MSG_WAITALL);

	if ( t <= 0 ) {
		if (t < 0)
			perror("recv");
		else
			printf("Server closed connection\n");
		exit(1);
	}

//	DPF( "Frame len:[%d]\n", frameLen );

	if ((t = recv(sock, frameBuffer, frameLen, MSG_WAITALL)) > 0) {
		frameBuffer[t] = '\0';

		DPR( dDATALINK, ">>>> (" );
		DPR( dDATALINK, t );
		DPR( dDATALINK, ") ");
		hexPrint( dDATALINK, frameBuffer, t );
		DLN( dDATALINK );

		// Intercept a master broadcast "thunk". This emulation is only
		// handled in cases where we are pretending to be a master/slave
		// network and the thunk triggers the poll. In some cases this may
		// just trigger sending the reply packet queued up during the inbound
		// routing.

		if ( t == 5 and Makernet.network.role == Network::slave )
			if ( memcmp( &thunk, frameBuffer, 5) == 0 )
			{
				if ( returnFrameSize > 0 ) {
					DLN( dDATALINK , "THUNK IN: Sending queued buffer back on thunk");
					sendToWire( returnFrameBuffer, returnFrameSize );
					returnFrameSize = 0;
					return;
				}

				for ( int i = 0 ; i < MAX_MAKERNET_FRAME_LENGTH ; i++ )
					frameBuffer[i] = 0;

				DLN( dDATALINK , "THUNK IN: Polling framework");

				int n = Makernet.network.pollFrame( frameBuffer, n );
				if ( n > 0 )
					sendFrame( frameBuffer, n );

				return;
			}

		// Dispatch the frame up to the higher levels of the network

		if ( t > 0 )
			Makernet.network.handleFrame( frameBuffer, t );
	} else {
		if (t < 0)
			perror("recv");
		else
			printf("Server closed connection\n");
		exit(1);
	}
}

bool UNIXSocketDatalink::handleSTDIN( char *b, int s )
{
	for ( int i = 0 ; i < s ; i++ )
	{
		if ( b[i] == '\n') {
			b[i] = '\0';
			printf( "*** Command from STDIN: [%s]\n", b );
			if ( handleCommand != NULL )
				handleCommand( b, s );
			return true;
		}
	}

	return false;

}

void UNIXSocketDatalink::loop()
{
	struct pollfd fds[2];

	const int stdin = 0;

	fds[0].fd = stdin; /* this is STDIN */
	fds[0].events = POLLIN;
	fds[1].fd = sock; /* this is our socket */
	fds[1].events = POLLIN;

	if ( poll( fds, 2, 1000 ) > 0 ) {
		// Handle stdin
		if ( fds[0].revents & POLLIN ) {

			int r = read(stdin, bpos, 1000 + userCommandBuffer - bpos);
			// printf( "**** read from STDIN: [%d] bytes\n", r );
			bpos += r;
			if ( handleSTDIN( userCommandBuffer, bpos - userCommandBuffer )) {
				// DLN( dDATALINK, "reset" );
				bpos = userCommandBuffer;
			}
		}

		// Handle network input
		if ( fds[1].revents & POLLIN )
			processIncomingFrame();
	}
}


#endif
