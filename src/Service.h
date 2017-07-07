/********************************************************
 ** 
 **  Service.h
 ** 
 **  Part of the Makernet framework by Jeremy Gilbert
 ** 
 **  License: GPL 3
 **  See footer for copyright and license details.
 ** 
 ********************************************************/


#ifndef SERVICE_H
#define SERVICE_H

#include <Packet.h>

// A "service" is a generic endpoint that can be a source, sink or both for
// packets. The makernet framework will pass handlePacket() to its registered
// services. Inside a handlePacket control flow, the handler is free to
// populate a response packet which will be immediately transmitted when the
// call is over.  It may also wait for the next polling event. If
// handlePacket() returns > 0, a new packet is replied immediately. Otherwise,
// no response is assumed unless the handler calls sendPacket.
//
// Services are provided periodic opportunities to generate new packets. If
// datalink requires a master/slave architecture and the device is the slave,
// than the services are polled in order of port number to generate packets
// via pollPacket(). This function offers a pre-allocated buffer and pointer
// to the address block. In cases of being a master, or in cases of a peer-to-
// peer datalink (such as RS485 or RFM69W radio), the polling occurs during
// system loop(). This architecture allows a Service to be largely ignorant if
// it is being used in a master/slave or peer-to-peer network.
//
// Of course, it is always possible simply to get a handle to the Network
// object and spontaneously generate a new packet at any time but the
// mechanisms above will offer a slight efficiency gain by re-using the low-
// level buffers, thus avoiding copying.
//
// Note that in the makernet low-level architecture, packets are never queued.
// Therefore, as soon as a packet is formed, it is either passed to a caller
// and read, or it is passed to the network. This allows reuse of the ~255
// bytes of buffer space.

class Service {

public:
	// Called when we are registered, do run-time setup here
	virtual void initialize() = 0;
	// Called when a packet is routed to us, return > 0 means a outgoing packet was generated
	virtual int handlePacket( Packet *p ) = 0;
	// Called with a scatch packet template, return 1 if a packet populated, 0 otherwise
	virtual int pollPacket( Packet *p ) = 0;
	// Called periodically during Makernet::loop()
	virtual void loop() = 0;
	// Called once during init, and then again when controller has lost state
	virtual void busReset() = 0;
	int port;

};

#endif
