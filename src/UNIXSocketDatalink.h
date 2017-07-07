/********************************************************
 ** 
 **  UNIXSocketDatalink.h
 ** 
 **  Part of the Makernet framework by Jeremy Gilbert
 ** 
 **  License: GPL 3
 **  See footer for copyright and license details.
 ** 
 ********************************************************/


#if !defined(ARDUINO) && !defined(UNIXSOCKETDATALINK_H)
#define UNIXSOCKETDATALINK_H


#include <Types.h>
#include <Datalink.h>

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

// UNIXSocketDatalink implements a Datalink layer for the UNIX testbed. This code is
// not intended to run on MCUs so needs an #if statement surround it.

class UNIXSocketDatalink : public Datalink {
public:
	virtual void initialize();
	virtual int sendFrame( uint8_t *inBuffer, uint8_t len );
	int loop();
	void processIncomingFrame();
	bool handleSTDIN( char *b, int s );
	typedef void (*UserCommandHandler)(char *cmd, int len );
	UserCommandHandler handleCommand = NULL;

private:
	struct sockaddr_un remote;
	int sock;

	// Buffer for the user command line
	char userCommandBuffer[1000];
	char *bpos = userCommandBuffer;
};


#define SOCK_PATH "/tmp/echo_socket"

#endif
