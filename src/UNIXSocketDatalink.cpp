
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

}

typedef struct { uint8_t value[5] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; } ThunkMessage;

ThunkMessage thunk;

// Called by upper layers to send a frame.

int UNIXSocketDatalink::sendFrame( uint8_t *inBuffer, uint8_t len )
{

	if (send(sock, &len, 1, 0) == -1) {
		perror("send");
		exit(1);
	}

	if (send(sock, inBuffer, len, 0) == -1) {
		perror("send");
		exit(1);
	}

	DPR( dDATALINK, "<<<< (" );
	DPR( dDATALINK, len );
	DPR( dDATALINK, ") ");
	hexPrint( dDATALINK, inBuffer, len );
	DLN( dDATALINK );

	// Emit the "thunk". If we are a master, we need to emulate the idea of
	// opening the bus for responses the same way that I2C works when the
	// master holds the clock line after its finished sending. This happens by
	// emitting a 5 byte FF signature.

	if ( Makernet.network.role == Network::master ) {
		int len = sizeof( thunk );

		if (send(sock, &len, 1, 0) == -1) {
			perror("send");
			exit(1);
		}
		if ( send(sock, (uint8_t *)&thunk, 5, 0) == -1 ) {
			perror("send");
			exit(1);
		}
	}

	return 0;
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
		// network and the thunk triggers the poll.

		if ( t == 5 and Makernet.network.role == Network::slave )
			if ( memcmp( &thunk, frameBuffer, 5) == 0 )
			{
				for ( int i = 0 ; i < MAX_MAKERNET_FRAME_LENGTH ; i++ )
					frameBuffer[i] = 0;

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

int UNIXSocketDatalink::loop()
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
	return 0;
}


#endif
