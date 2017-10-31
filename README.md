# Overview 

Makernet is a general-purpose modular framework for making interactive electronics projects. Makernet radically simplifies the work of connecting and coding for projects that that are made up of different devices such as displays, keyboards, knobs, switches, LED strips, sound playing devices, etc. It aims to bring 21st-century concepts of clean abstraction and modularity to hardware design.

An early demonstration video here:

[![Image](images/youtube-video-thumb.png)](https://www.youtube.com/watch?v=rZLKJq-t7fU)

With Makernet, designing interactive projects is less painful, more fun, and way more flexible. Makernet devices designed today include:



Makernet is comprised of several elements:

- A common network or field-bus (loosely based on I2C) that daisy-chains all project components with a 6-pin connector providing data, power and ground
- A network framework that supports dynamic addresses, device discovery, introspection, message passing for highly memory/processor constrained devices
- A high quality object-oriented software framework that allows the project designer to remain focused functionality, not intricate hardware details
- A set of existing hardware designs implementing a variety of peripheral types such as knobs, buttons, sounds, displays, capacitive touch, etc.
- A easy-to-use extension framework for creating new peripherals

The framework requires no soldering, is 100% Arduino compatible, and supports low-power applications. It has been shown to work on the Teensy 3.1/3.2, Adafruit Feather M0, and likely works with any SAMD21-based board such a Arduino Zero. The design supports hot-plugging (devices can enter or leave the network without disruption to running programs). Due to its flexible architecture and strong abstraction around the physical layer, Makernet can be adapted for wireless and RS485 protocols.

Makernet is aimed at artists, educators, students, hobbyists and prototypers. The aim is to democratize the creation of cool hardware and software, and make hardware work in a more clean, 21st century fashion. Because it is expandable and new designs can be added over time, Makernet is intended to seed an expanding ecosystem of compatible components and software.

You might think that the extra complexity of a network layer adds unnecessary expense to a new project. However, the reality is that the prices of high-speed 32-bit ARM processors have fallen so low that the extra expense is negligible especially for prototyping and limited-run projects. Makernet compatibility adds less than $2.00 to the BOM of its peripherals in _single_ quantities, requiring nothing more than a commodity JST-SH connector and a $1.18 ATSAMD11 processor (plus a few passives like decoupling capacitors.) Peripherals do not need an external clock or any transceivers. 

# Simple example: Hello World

The following is a "hello world" where a bank of LEDs is blinked when a user presses a button. It uses three modules:

- A ATSAMD21 controller board (GMxxxx)
- A DPad button controller (GMxxxx)
- A LED strip (GMxxxx)

Physically it looks like this:

The network diagram created by these components:

An a video demo of what it looks like:

The source code

# Getting started

To get started you must do the following:

1. Install the Arduino IDE
2. Install the Makernet library in your Arduino IDE 
3. Purchase or build a Makernet controller and peripherals and some JST-SH cables
4. String together the components and write some software

# Available modules

The following modules have been built and tested with many more in development:

For more details on the capabilities and documentation of these modules, see the documentation.

# Makernet networking

Makernet supports a variety of transport layers, but the most common one is based on the I2C specification. Makernet's design has been shown to be compatible with RS485 and other more robust signaling frameworks. Datalink layers in Makernet are actually pluggable code modules with strong design contracts, therefore swapping out I2C with another transmission medium will not disrupt the higher layers.

What follows is a tour of the "default" makernet construct which uses I2C. 

# Makernet specification: Physical layer (OSI 1)

The physical layer of Makernet is a 6-pin JST-SH header. This header looks like this:

Digikey part number XXXX.

The six pins are mapped as follows

1. GND (project-wide ground)
2. 3.3V (regulated voltage)
3. VIN (unregulated battery supply between 3.3-5V, often supplied by a battery or USB connection
4. Signal (used for interrupts)
5. SCL - An I2C compatable clock line with V(high) = 3.3V and 4.7K pull ups
6. SDA - An I2C compatable address line with V(high) = 3.3V and 4.7K pull ups

Makernet peripherals designed so far use the JST-SH header for maximum hobbiest flexibility, but technically there is nothing special about this six-wire format. It is perfectly acceptable to simply connect SCL and SDA between devices and let each of them handle their own power requirements. This is a commonly tested use case and should not present any problems as long as the pull-ups resistors are sufficient on both ends.

# Makernet specification: Data-link layer

The default data-link layer of Makernet based around I2C with a few modifications. In an I2C network, masters and slaves communicate in a series of bi-directional transactions over a two-wire interface. The master provides the clock synchronization for the communication, and a set of rules of engagement let other devices know who has "write" authority on the bus. These rules of engagement implement common data-link requirements such as medium-access control (defining who can speak), timing (defining the clock domain for the transfer of information), and link-detection. For more on I2C, see XXXX, YYYY and ZZZZ.

I2C is a great choice for a network protocol because it is commonly implemented with hardware peripherals (e.g. in silicon, not software) on a vast number of embedded MCUs including the SAMD21 and SAMD11 next generation "Arduino" chips. 

Makernet uses I2C with one single, somewhat unusual modification: all nodes use a single I2C address (0x09) rather than each device receiving its own address. This tweak does not interfere in any way with non-Makernet-aware I2C devices such as displays, sound amplifiers, GPIO devices, etc allowing for I2C busses to be mixed between Makernet and non-Makernet I2C devices. Address 0x09 is used because it is reserved for certain types of SMBus peripherals and no commercially available I2C devices will use that address.

I2C provides a surprisingly sophisticated medium access control mechanism not unlike RS485 or CANBUS. On initiation of  a message, each sender ("master device") detects if it has control of the clock line. If it doesn't it backs off the transaction allowing the other master to proceed. In most cases this is implemented in silicon as opposed to software.

I2C also supports respose and broadcast arbitration, meaning that multiple responses are generated to a master's message, only one of them will succeed. This functionality is based on something called "dominant/recessive" arbitration. If multiple devices respond to a message, the sending peripherals can discover this situation and the "loosing" device backs off allowing the "winner" to continue the transaction.

Makernet's datalink layers are pluggable and implemented as subclasses of the Datalink class. This allows Makernet to work on more robust physical layer types beyond I2C such as RS485, packet wireless or even Ethernet. Makernet allows for both transactional transports (I2C) and non-transactional links such as RS485. In fact, Makernet was built to allow full stack testing on a emulated data link layer that can be hosted on a OSX or Linux architecture using UNIX-domain sockets.

Note that in Makernet terminology, the groups of bytes that go out on the wire are called "frames". The datalink layer is responsible for taking a bytestream and putting it out on the wire. This means that within the Makernet code, an I2C message is actually a "frame" even though that is not the usual terminology.

# Network layer

The network layer of Makernet is handled by a custom, minimalist protocol stack that has been carefully designed for low memory and code consumption. This stack handles vending Makernet addresses to peripherals, discovery of new devices, broadcast communication, and introspection. 

The network layer of Makernet is how you actually send and receive packets. 


The basic byte structure of a makernet packet:
