# Makefile
# Example for RH_RF95 on Raspberry Pi
# Requires pigpio to be installed: http://abyz.me.uk/rpi/pigpio/

CC            = g++
CFLAGS        = -DRASPBERRY_PI -pthread
LIBS          = -lpigpio -lrt
RADIOHEADBASE = ../../../..
INCLUDE       = -I$(RADIOHEADBASE)

all: rf95_reliable_datagram_server

RasPi.o: $(RADIOHEADBASE)/RHutil_pigpio/RasPi.cpp
	$(CC) $(CFLAGS) -c $(RADIOHEADBASE)/RHutil_pigpio/RasPi.cpp $(INCLUDE)

rf95_reliable_datagram_server.o: rf95_reliable_datagram_server.cpp
	$(CC) $(CFLAGS) -c $(INCLUDE) $<

RH_RF95.o: $(RADIOHEADBASE)/RH_RF95.cpp
	$(CC) $(CFLAGS) -c $(INCLUDE) $<

RHReliableDatagram.o: $(RADIOHEADBASE)/RHReliableDatagram.cpp
	$(CC) $(CFLAGS) -c $(INCLUDE) $<

RHDatagram.o: $(RADIOHEADBASE)/RHDatagram.cpp
	$(CC) $(CFLAGS) -c $(INCLUDE) $<

RHHardwareSPI.o: $(RADIOHEADBASE)/RHHardwareSPI.cpp
	$(CC) $(CFLAGS) -c $(INCLUDE) $<

RHSPIDriver.o: $(RADIOHEADBASE)/RHSPIDriver.cpp
	$(CC) $(CFLAGS) -c $(INCLUDE) $<

RHGenericDriver.o: $(RADIOHEADBASE)/RHGenericDriver.cpp
	$(CC) $(CFLAGS) -c $(INCLUDE) $<

RHGenericSPI.o: $(RADIOHEADBASE)/RHGenericSPI.cpp
	$(CC) $(CFLAGS) -c $(INCLUDE) $<

rf95_reliable_datagram_server: rf95_reliable_datagram_server.o RH_RF95.o RHReliableDatagram.o RHDatagram.o RasPi.o RHHardwareSPI.o RHSPIDriver.o RHGenericDriver.o RHGenericSPI.o
	$(CC) $^ $(LIBS) -o rf95_reliable_datagram_server


clean:
	rm -rf *.o rf95_reliable_datagram_server

