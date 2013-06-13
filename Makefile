ENDIANESS=OSC_HOST_LITTLE_ENDIAN
PLATFORM=$(shell uname)

TUIO_RELAY = tuio-to-tlay
TUIO_STATIC  = libTUIO.a

INCLUDES = -I./TUIO -I./oscpack
CFLAGS  = -g -Wall -O3 $(SDL_CFLAGS) -fPIC
CXXFLAGS = $(CFLAGS) $(INCLUDES) -D$(ENDIANESS)

RELAY_SOURCES = tuio-to-tlay.cpp
RELAY_OBJECTS = tuio-to-tlay.o

TUIO_SOURCES = ./TUIO/TuioClient.cpp ./TUIO/TuioServer.cpp ./TUIO/TuioTime.cpp
OSC_SOURCES = ./oscpack/osc/OscTypes.cpp ./oscpack/osc/OscOutboundPacketStream.cpp ./oscpack/osc/OscReceivedElements.cpp ./oscpack/osc/OscPrintReceivedElements.cpp ./oscpack/ip/posix/NetworkingUtils.cpp ./oscpack/ip/posix/UdpSocket.cpp
UDP_SOURCES = ./tlay-touch/TLayTouch.cpp ./tlay-touch/json/json.cpp ./tlay-touch/udp/UdpClient.cpp

COMMON_SOURCES = $(TUIO_SOURCES) $(OSC_SOURCES) $(UDP_SOURCES)
COMMON_OBJECTS = $(COMMON_SOURCES:.cpp=.o)

all: relay static

static:	$(COMMON_OBJECTS)
	ar rcs $(TUIO_STATIC) $(COMMON_OBJECTS)

relay:	$(COMMON_OBJECTS) $(RELAY_OBJECTS)
	$(CXX) -o $(TUIO_RELAY) $+ -lpthread

clean:
	rm -rf $(TUIO_RELAY) $(TUIO_STATIC) $(COMMON_OBJECTS) $(RELAY_OBJECTS)
