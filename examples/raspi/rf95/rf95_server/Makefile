# Makefile
# Example for RH_RF95 on Raspberry Pi
# Requires pigpio to be installed: http://abyz.me.uk/rpi/pigpio/

CROSSCOMPILER=$(shell if [ -z `which 'arm-linux-gnueabihf-g++'` ]; then echo "no"; else echo "yes"; fi)
ifeq ($(CROSSCOMPILER),yes)
CC            = arm-linux-gnueabihf-g++
CFLAGS        = -DRASPBERRY_PI -DRH_USE_MUTEX -pthread 
LIBS          = -lpigpio -lrt -L$(PIGPIOBASE) -lpthread  -D_REENTRANT
RADIOHEADBASE = ../../../..
PIGPIOBASE    = ../../../../../pigpio/
SHARED        = ../shared/
INCLUDE       = -I$(RADIOHEADBASE) -I$(PIGPIOBASE) -I$(SHARED)
else
CC            = g++
CFLAGS        = -DRASPBERRY_PI -DRH_USE_MUTEX -pthread
LIBS          = -lpigpio -lrt
RADIOHEADBASE = ../../../..
PIGPIOBASE    = ../../../../../pigpio/
SHARED        = ../shared/
INCLUDE       = -I$(RADIOHEADBASE) -I$(PIGPIOBASE) -I$(SHARED)
endif

ifeq ($(REGION),Europe)
CFLAGS += -DEUROPE
endif

ifeq ($(LORABOARD),DraginoLoraGPSHat)
CFLAGS += -DDRAGINO_GPS_HAT
endif

ifeq ($(DEBUG),1)
CFLAGS += -g3
endif

all: rf95_server1 rf95_server2

RasPi.o: $(RADIOHEADBASE)/RHutil_pigpio/RasPi.cpp
	$(CC) $(CFLAGS) -c $(RADIOHEADBASE)/RHutil_pigpio/RasPi.cpp $(INCLUDE)
	
help_functions.o: $(SHARED)/help_functions.cpp
	$(CC) $(CFLAGS) -c $(INCLUDE) $<
	
rf95_server1.o: rf95_server1.cpp
	$(CC) $(CFLAGS) -c $(INCLUDE) $<

rf95_server2.o: rf95_server2.cpp
	$(CC) $(CFLAGS) -c $(INCLUDE) $<
	
RH_RF95.o: $(RADIOHEADBASE)/RH_RF95.cpp
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

gpsMT3339.o: $(SHARED)/gpsMT3339.cpp
	$(CC) $(CFLAGS) -c $(INCLUDE) $<
	
rf95_server1: rf95_server1.o help_functions.o RH_RF95.o RHDatagram.o RasPi.o RHHardwareSPI.o RHSPIDriver.o RHGenericDriver.o RHGenericSPI.o gpsMT3339.o
	$(CC) $^ $(LIBS) -o rf95_server1
	
rf95_server2: rf95_server2.o help_functions.o RH_RF95.o RHDatagram.o RasPi.o RHHardwareSPI.o RHSPIDriver.o RHGenericDriver.o RHGenericSPI.o gpsMT3339.o
	$(CC) $^ $(LIBS) -o rf95_server2


clean:
	rm -rf *.o rf95_server1 rf95_server2
	
deploy: rf95_server1 rf95_server2
	sshpass -p"$(PASSWD)" scp rf95_server1 pi@$(HOST):/home/pi
	sshpass -p"$(PASSWD)" scp rf95_server2 pi@$(HOST):/home/pi

