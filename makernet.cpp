// Makernet revised core library rebuilt with improved hygene and ability to
// be tested and debugged on OSX

//  c++ -DMASTER makernet.cpp -o master
//  c++ -DSLAVE makernet.cpp -o slave


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// util.h

#define UI8(x) static_cast<uint8_t>(x)
#define DPR( X... )			   printDebug( X )
#define DLN( X... )			   printDebugln( X )
#define MAX(x,y) x > y ? x : y;
#define MIN(x,y) x > y ? y : x;


#define HEX 0x10


void hexPrint( uint8_t *buffer, int size );

void printDebug( const char *s );
void printDebug( int i);
void printDebug( int i, int format);
void printDebugln( char *s );

// util.cpp


void printDebug( const char *s )
{
	printf( "%s", s );
}

void printDebug( int i)
{
	printf( "%d", i );
}

void printDebug( int i, int format)
{
	if ( format == HEX )
	{
		printf( "%x", i );
	}
}


void printDebugln( char *s )
{
	printf( "%s\n", s );
}

void printDebugln( )
{
	printf( "\n" );
}

void hexPrint( uint8_t *buffer, int size )
{
	for ( int i = 0 ; i < size ; i++ )
	{
		uint8_t value = buffer[i];
		if (value < 0x10)
			DPR('0');
		DPR(value, HEX);
		DPR( " " );
	}
}


// The datalink layer handles putting bytes on wires with no
// knowledge of what those bytes mean. Each collection of bytes
// is called a "frame". The datalink interface is made interchangable so that multiple datalinks
// such as I2C, RFM, and even ethernet could all be makernet enabled.

#define MAX_MAKERNET_FRAME_LENGTH 255

typedef void (*frameReceiveCallback_t)( uint8_t *buffer, uint8_t readSize );

class Datalink {

public:
	// Start the datalink
	virtual void initialize() = 0;
	// Send a single frame
	virtual int sendFrame( uint8_t *inBuffer, uint8_t len ) = 0;
	// Register a callback when new frames arrive
	void onReceive( frameReceiveCallback_t t );

private:
	frameReceiveCallback_t frameReceiveCallback;

};

// Our callers can use this to set a callback when a new frame is received

void Datalink::onReceive( frameReceiveCallback_t t )
{
	frameReceiveCallback = t;
}

// An implementation of UNIX domain socket master

class UnixMaster : public Datalink {
public:
	virtual void initialize();
	virtual int sendFrame( uint8_t *inBuffer, uint8_t len );
private:
	struct sockaddr_un remote;
	int sock;
	uint8_t receiveBuffer[MAX_MAKERNET_FRAME_LENGTH];

};


#define SOCK_PATH "echo_socket"


void UnixMaster::initialize()
{

	int t, len;

	char str[100];

	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	printf("Trying to connect...\n");

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(sock, (struct sockaddr *)&remote, len) == -1) {
		perror("connect");
		exit(1);
	}

	printf("Connected.\n");

}

int UnixMaster::sendFrame( uint8_t *inBuffer, uint8_t len )
{
	if (send(sock, inBuffer, len, 0) == -1) {
		perror("send");
		exit(1);
	}

	DPR( "<<<< (" );
	DPR( len );
	DPR( ") ");
	hexPrint( inBuffer, len );
	DLN();


	int t;

	if ((t = recv(sock, receiveBuffer, MAX_MAKERNET_FRAME_LENGTH, 0)) > 0) {
		receiveBuffer[t] = '\0';

		DPR( ">>>> (" );
		DPR( t );
		DPR( ") ");
		hexPrint( receiveBuffer, t );
		DLN();
		// printf("echo> %s", receiveBuffer);
	} else {
		if (t < 0) perror("recv");
		else printf("Server closed connection\n");
		exit(1);
	}

}


class UnixSlave : public Datalink {
public:
	virtual void initialize();
	virtual int sendFrame( uint8_t *inBuffer, uint8_t len );
	void loop();
private:
	struct sockaddr_un local, remote;
	int sock;
	uint8_t receiveBuffer[MAX_MAKERNET_FRAME_LENGTH];

};


void UnixSlave::initialize()
{

	int len;

	char str[100];

	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, SOCK_PATH);
	unlink(local.sun_path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(sock, (struct sockaddr *)&local, len) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(sock, 5) == -1) {
		perror("listen");
		exit(1);
	}
}

int UnixSlave::sendFrame( uint8_t *inBuffer, uint8_t len )
{

}

void UnixSlave::loop()
{
	int t, s2;
	int done, n;
	printf("Waiting for a connection...\n");
	t = sizeof(remote);
	if ((s2 = accept(sock, (struct sockaddr *)&remote, (socklen_t *)&t)) == -1) {
		perror("accept");
		exit(1);
	}

	printf("Connected.\n");

	done = 0;
	do {
		n = recv(s2, receiveBuffer, MAX_MAKERNET_FRAME_LENGTH, 0);
		if (n <= 0) {
			if (n < 0) perror("recv");
			done = 1;
		}

		if (!done)
			if (send(s2, receiveBuffer, n, 0) < 0) {
				perror("send");
				done = 1;
			}
	} while (!done);

	close(s2);
}


#ifdef MASTER

int main(void)
{
	UnixMaster um;
	um.initialize();

	um.sendFrame( (uint8_t *)"hello", 5 );


}

#else

int main(void)
{
	UnixSlave us;
	us.initialize();

	while (1)
		us.loop();


}

#endif
