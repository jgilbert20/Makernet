/*
** echos.c -- the echo server for echoc.c; demonstrates unix sockets
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <poll.h>

typedef int bool;
#define true 1
#define false 0

#define SOCK_PATH "/tmp/echo_socket"
	

	void hexPrint( uint8_t *buffer, int size )
{
	for ( int i = 0 ; i < size ; i++ ) {
		uint8_t value = buffer[i];
		if (value < 0x10)
			printf( "0" );
		printf( "%X", value );
		printf( " " );
	}
}


int main(void)
{
	int listenSocket, s2, t, len;
	struct sockaddr_un local, remote;
	char str[100];

	if ((listenSocket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, SOCK_PATH);
	unlink(local.sun_path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(listenSocket, (struct sockaddr *)&local, len) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(listenSocket, 5) == -1) {
		perror("listen");
		exit(1);
	}


	struct pollfd fds[100];
	fds[0].fd = listenSocket; /* this is STDIN */
	fds[0].events = POLLIN;


	int nextFreeSlot = 1;


	for (;;) {


		int p =		poll( fds, 10, 1000 );
		printf( "poll returned %d\n", p );


		for ( int i = 0 ; i < 100 ; i++ ) {
			if ( fds[i].revents & POLLERR ) {
				printf( "ERR - Closing connection stream %d\n", fds[i].fd  );
				close( fds[i].fd );
				fds[i].events = 0;
				fds[i].fd = -1;
			}

			if ( fds[i].revents & POLLHUP ) {
				printf( "HUP - Closing connection stream %d\n", fds[i].fd  );
				close( fds[i].fd );
				fds[i].events = 0;
				fds[i].fd = -1;
			}
			if ( fds[i].revents & POLLNVAL ) {
				printf( "HUP - Closing connection stream %d\n", fds[i].fd  );
				close( fds[i].fd );
				fds[i].events = 0;
				fds[i].fd = -1;
			}



			if ( fds[i].revents & POLLIN ) {
				printf( "read available on %d\n", i );
				if ( fds[i].fd == listenSocket ) {
					printf("Waiting for a connection...\n");
					t = sizeof(remote);
					if ((s2 = accept(listenSocket, (struct sockaddr *)&remote, &t)) == -1) {
						perror("accept");
						exit(1);
					}

					printf( "New connection socket s2=%d\n", s2 );
					int assigned = false;
					for ( int x = 0 ; x < 100 ; x++ ) {
						if ( fds[x].fd == s2 ) {
							printf( "reusing slot %d socket %d\n", x, s2 );
							fds[x].events = POLLIN;
							assigned = true;
						}
					}
					if ( assigned == false ) {
						fds[nextFreeSlot].fd = s2;
						fds[nextFreeSlot].events = POLLIN;
						nextFreeSlot++;
					}


				} else {
					int n = recv(fds[i].fd, str, 100, 0);
					if (n <= 0) {
						if (n < 0) perror("recv");
					}
					str[n] = '\0';
					printf( ">>>> FD:{%d} SZ:(%d)", fds[i].fd, n );
					hexPrint( str, n );
					printf( "\n" );

					// Echo to all connections
					for ( int x = 0 ; x < 100 ; x++ ) {
						int sock = fds[x].fd;
						if ( sock > 0 && sock != listenSocket && x != i ) {
							printf( "echo to %d\n", sock);
							send(sock, str, n, 0);
						}
					}


				}

			}

		}

	}

	return 0;
}



