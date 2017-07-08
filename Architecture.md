This is an in-depth technical review of the makernet implementation and API. FOr more general architecture stuff, see the README file.


# new mailbox design


The MailboxService controls a collection of Mailboxes which can be used to transfer and preserve state across the network in an efficient and low-maintenance fashion. 

To use it, instantiate a series of Mailboxes somewhere in your code. Each mailbox has an ID, a type and a description. 

design requriements
- you'd want to be able to add devices that have different firmware bases than yours (e.g. adding a new device type shouldn't force the old devices to recompile)
- you want to have different developers assign IDs within their own namespaces and not have to consult some global "mailbox" registry to prevent conflict

- Observation: the pollPacket/handlePacket interface is efficient and durable and used as a pattern in many other places in the code.

All the above suggests that the awareness of a mailbox meaning should be strictly kept between the BasePeripheral object and the mailbox service on the peripheral end.

design option 1: the mailbox service is instantiated multiple times, once per each peripheral. On a controller, all incoming traffic FROM a particular address gets automatically routed to the peripheral object, which then has a port jump table. On the peripheral, incoming packets go directly to services. CON: lots of dispatching, and every mailbox service must be subclassed. Also, what effort is needed by the peripheral to route to its mailbox object? Is that handled by BasePeripheral?

design option 2: one mailbox service instance on both the peripheral and on the controller. When an incoming packet comes in for the mailbox, the service somehow routes it elsewhere on the controller.

It seems to me that every baseperipheral should have a mailbox object. My brain is freaking out about vtables, but I actually honestly don't see the problem with them. It makes for the cleanest, best design. Dispatch has to happen ANYWAY, either with a load of switch statements OR with function pointers. FPs are just as fast. The freaking i'm having has to do with the fact that a vtable forces code for all virtual functions (and their call-path dependencies) to be included in the final object code. So just to self-talk here a bit, the fail would be a case where a peripheral device uses an object but doesn't ever have to call function virtual X because that function is ONLY used by the controller but has to keep the code there anyway. If you can avoid that "fail" situation, vtables really honestly add nothing to the code and run way faster than a chain of if statements. So the question is when you make something virtual, are you doing it on something that will be handled by even the tiniest peripheral object? The unsettling question: if there are 10 subclasses of a virtual object in the codebase, but only 1 subclass is ever "touched", will they get optimized out? I assume the answer is yes. And actually vtables are also superior in common routing/framework code over chained IF or switch. In the switch case, you sadly end up with a shit ton of references that the compiler likely HAS to include. 



 
# special semantics: initialize, configure, loop and busReset

Makernet has some special terminology around these verbs

* Initialize() called within the core parts of the makernet framework to set up objects exactly once, when framework user called Makernet.initialize()
* Configure() is a function often called by initialize and intended to be overriden by new peripheral objects to take whatever steps
are needed to arrange the framework (for instance, assigning mailboxes, setting up services, etc, etc.)
* loop() is called on many objects during Makernet::loop() so there is an opportunity for housekeeping
* busReset() is a special verb that is intended to reset all state around the entire network. When this is called, every object should assume all information it has about other devices in the network is potentially wrong.. clear buffers, interrupt work in progress, and wait for new synchronization data to restore state


7/8/2017 program size calculation

6660 for MakernetD11 UART, no USB, INTERNAL_OSC - perhipheral chain
5964 when CONTROLLER_SUPPORT set to 0
5924 conditionalizing the entire BasePeripheral framework
5884 with some really small scale adds of CONTROLLER_SUPPORT
5836 with removal of all loop logic from services incluing one virtual function
5860 with just re-addition of empty loop() virtual handler in 2 services
5860 with addition of one more (unreferenced) service object


WHAT A RELIEF-new framework signficantly smaller!!


program size calculations as of 7/7/2017

3552 base SAMD11 no USB, internal clock
4332 for adding Makernet.h
7232 for adding DCS, a peripheral and the i2c datalink
7304 for everything except init and loop
8272 for everything

Wow, taking out random saved almost 700 bytes!!!
7576 for everything with a single random() call removed


a super condensed version of the old makernet peripheral code compiled to this but its way less functional:

Sketch uses 6980 bytes (42%) of program storage space. Maximum is 16384 bytes.

arm-none-eabi-nm makernet_g2_master.ino.elf -S --size-sort -C



https://stackoverflow.com/questions/24649504/gcc-gc-sections-and-finding-symbol-dependencies

nice set of tools here for tracking where size is going and why

The root of the problem for me was having a class that inherited from another. This created a virtual table, and the compiler cannot remove dead code that is referenced in a virtual table.




# Address vending logic design

In the current Makernet architecutre, devices only receive IDs when they are linked to a controll object on the controller. This is basically the entire connection architecture, and devices that cannot be linked will not get addresses.
Periodically devices needing addresses emit a REQUEST_ADDRESS packet with some meta-data including their unique device ID. The controller on the network will assign them an available address by calling into the Peripheral framework with the metadata. The peripheral wanting a link will return its pointer and the address is assigned. A packet is emitted to make the assignment. If the packet is received, the remote device will define its network location to the new variable and all further peripheral data will now flow to the address issued completing the link. If the packet is dropped, subsequent REQUEST_ADDRESS requests will re-establish th


# datalink

The bottom-most layer is the DataLink layer. It handles low level physical transport. Only concerned with putting bytes on the wire and arbitration. Each burst of bytes from a single sender is called a "frame". 

There are three varieties of datalink: Peer, Master, Slave. 
Peer: Every node is equal and can send and receive when it wants
Master: A node that initiates all network activity
Slave: A node that waits for the master to initiate network activity

Inheritance is: Datalink (interface), with subclasses I2CSlave, I2CMaster.

We also define a UNIX datalink layer that can act as a master or a slave. The UNIX datalink layer lets us do full stack testing.

The datalink receives periodic calls from the Makernet:loop layer. It is responsible for capturing bytes on the wire and assembling them into a frame using whatever underlying hardware architecture exists. When a full frame is captured, it calls Makernet.network.handleFrame(uint8_t *buffer, uint8_t len ) for the frame to be dispatched at the network layer. The datalink itself owns the buffer used for these operations, so there is never any byte copying.

When the upper layers wish to send a frame, they call Datalink.sendFrame( uint8_t *inBuffer, uint8_t len ). This puts a frame on the wire.

In full peer-to-peer architectures, the bus has no concept of transactions or locking. Instead, every peer is free to send its own packets whenever it wishes via the calls at the Network layer. 

There is also a mechanism used to poll for packets, and this has the advantage of stronger encapsulation so that the calling applications don't have to trigger and set reminders to generate new packets. Each service can receive a pollPacket(Packet *p) call asking if it has anything to send. 

If the device is a master or a peer, periodically, the network layer handler called sendNextPacket is called. This polls along all producers of packets (typically Service objects) shopping around for something to send. This is useful because these services will be called for packet generation at optimal times for the network stack (e.g. when the bus is clear.) 

In master/slave architectures, the slaves cannot transmit unless the master issues a call for packets. In these archtiectures (for example,  I2C), there is a very clear semantic for who is initiating the transaction and who is replying. In this case, the Datalink layer explicitly knows when a call for a packet has occurred because it can read the bus state. In this case, the datalink layer triggers the Network layer with a "pollFrame()" event.




## sendframe

// Sends LEN bytes onto the datalink. Maximum that can be sent is UINT8
int r = sendFrame( *bytes, len);

I2CMaster, this is implemented as: beginTransmission, write, then endTransmission. This is followed by a receiveFrom() which will store the most recently received frame into the datalink layer.

I2CSlave: this returns an error unless a transaction has been started.

We always assume that its kosher to send MAX_FRAME_LENGTH

## receiveFrame

non-blocking call
len MUST be at least MAX_FRAME_LENGTH
actual length is returned, 0 for no message, negative for errors
r = receiveFrame( *bytes, len)

NOTE: probably doesn't have to be implemented
buffer you are given is a statically allocated message buffer

## onReceive

a function pointer you configure to your receive handler. The handler will pass you (*bytes, len) as if you had called recv().
buffer you are given is a staticly allocated message buffer and may be used for any purpose until the next packet comes in.

function pointer: onReceive( &fp )
receiveHandler()

# onClearToSend

a function pointer called when you are free to send a message. Your final step should be to call sendFrame(). You are given a scrachpad to use to populate the message. Using the scratchpad MAY potentially save an extra buffer copy.








Network-contiguous set of nodes over one or more data link. Handles forming packets, managing addresses, etc


Address vending 
Net control signals
Poll unassigned
Request assignment
Global reset
Assign address 

Why not just have a polling packet (e.g. Send me anything)

I guess each layer must pass up to the next one

So datalink passes to network
Network will handle net control packets I guess

What if we have a concept of a node. The node knows how to send and receive. And a node proxy 

I guess mailboxes and streams would be node level things. 

The network layer decides which node to send packets to. In most or all cases that is probably the local node object

As a node i can get pushes and pulls from my streams and mailboxes. And I can exchange data grams 

The question I have now: is all further datagram handling just pushed up to the node? 

Node.handlePacket()
Node.pollPacket()

This elegantly could handle the situation where a single device operates multiple nodes but it adds a new byte to the message format with sort of low value-add. 

NodeProxy is the object you get to talk to a remote node. 

Node and node proxy are distressingly similar interfaces. They basically implement the same interface. 

A peripheral could be thought of as a subclass of the node which handles a mailbox and stream and command interface. 

What if the command byte was basically a structure that let you both include a command and a node Id? Maybe 0 would be reserved for broadcast or device control stuff. 

Wow. This is a very compelling design. Solves a lot of problems. And good end to end design actually. 

this would mean that every Node object can do whatever it wants with its datagrams. Each node is unqiuely addressable by a network, address and finally a node index. 

Node index 0 would always be a DeviceControl node and would handle address assignment, discovery, enumeration, and other kinds of shit like that.

so what is the identity of the other endpoint?

I have an encoder and a encoderController

how do they talk to each other? that would mean that mailbox updates would be directed somewhere.

So the rule is when i get an inbound packet of any type, i find the node object corresponding to the destination address. 

so maybe nodes are either controllers or peripherals?

when the peripheral node starts up, it has no where to send its messages. 

then on discovery, we get a type ID. If we match the type ID we could connect it to a local object. The network layer can send any arbitrary message to any NodeAddress. Once the match is made, each Node object stores the NodeAddress of its opposite endpoint.

each node has a central timer which represents the # of ms since the last message was sent.

should we also support a mode of communication where nodes simply broadcast mailbox updates and allow multiple subscribers?

in the first impkementaiton, the subscriber of a node was always just implictly whatever master initiatied the connection. But in the new architecture, return packets are addressed too. And preseujmably the sender has to be tagged as well.

So a return packet says its "for" XYZ, and the receiver has to decide if it is XYZ. then XYZ just enteres the handlePacket queue like any other packet. So it has to get routed to the proxy object.

this seems to imply an architecture where each node searches for its endpoint. So is "Node" the right concept here? Maybe channel? Its really one half of a pair now in this new architecture. Maybe Endpoint is a better term?

best thing probably to do is implement a simple version of this and observe its behavior and limitations. 

In essence, it makes sense. If I'm a peripheral, i'm either connected (e.g. i have an address of the actual thing) or i'm disconnected waiting for that.   

And I can always implement other kinds of nodes based on broadcast paradims in the future.

or is it better that we really think of this as a connection interface? and you have sockets basically which route up to listeners. 

or maybe what i'm calling a node is really a "service", and the service gets and receives bytes.

this could get very complicated.

lets take the mailbox concept whihc i feel very confident is a good abstraction.

the mailbox needs to know WHERE to push its updates. And the mailbox is enherently an end-to-end construct that doens't have a clear meaning besides the exchange between two nodes.

so in the service architecuture, I have a collection of Endpoints. Those endpoints could be streams or mailboxes. 

when a EncoderController objects boots up, it needs to register to get its updates and possiblt its streams. Its been matched to a device ID. so now it needs some sort of discovery protocol to get its sub-objects looped up to its counterparts.

the core philisophical question is which type of thing holds the concept of a connection? is the connection at the level of the Node/Service? or is it at the level of the stream/mailbox/command interface.

I think the most flexible appraoch right now is to implement the node as the holder of the connection and multiplex from there into passing types. In the future, you could just havea  pure 1 endpoint to 1 service corrspondance.


# Rethinking again
Have looked at more implementation now. Clearly what I have so far is tidy and clean and I want to keep it that way. 

I think the best design is the simple one. The peipherla registers services for different streams and mailboxes. 

Each device has a type code

The type code is used on device address discovery to match to a peripheral object like the ones I have now. These objects will automatically get routed any traffic from the device in question. 

Maybe the framework reattached and connects packets to services based on the src port. 

Framework keeps track of Connect and disconnect status

Also emits some random numbers that if changed cause all devices to reset. 

So address basically becomes a connection identifier. When the address is vended it means a connection is reached. That may just work perfectly for now. 

What I love about the work done so far is that it is really tidy. 



# peripheral connection

assigning addresses is the primary way of pairing now. each device has a profile which corresponds to its address, type, connection state, hardware ID, and a few other things.

the general rule for pairing is that first we seek to find an exact hardware match. If not, we find the first peripheral object with that device type. 

all of this is implemented by populating a deviceprofile object and passing it to "requestPair()". On true, the pairing is established and the address is assigned. On false, the next eligble object is consulted.












