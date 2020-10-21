// rf95_client.cpp
// -*- mode: C++ -*-
// Example app showing how to create a simple messaging client
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf95_server.
//
// Requires Pigpio GPIO library. Install by downloading and compiling from
// http://abyz.me.uk/rpi/pigpio/, or install via command line with 
// "sudo apt install pigpio". To use, run "make" at the command line in 
// the folder where this source code resides. Then execute application with
// sudo ./rf95_client.
// Tested on Raspberry Pi Zero and Zero W with LoRaWan/TTN RPI Zero Shield 
// by ElectronicTricks. Although this application builds and executes on
// Raspberry Pi 3, there seems to be missed messages and hangs.
// Strategically adding delays does seem to help in some cases.

//(9/20/2019)   Contributed by Brody M. Based off rf22_client.pde.
//              Raspberry Pi mods influenced by nrf24 example by Mike Poublon,
//              and Charles-Henri Hallard (https://github.com/hallard/RadioHead)

#include <pigpio.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <RH_RF95.h>
#include <help_functions.h>

//Function Definitions
void sig_handler(int sig);



#ifdef DRAGINO_GPS_HAT
pthread_mutex_t gps_data_mutex;
#include <gpsMT3339.h>

// Pin definitions for dragino gps hat
#define RFM95_CS_PIN 25  // Slave Select on GPIO25 so P1 connector pin #22
#define RFM95_IRQ_PIN 4 // IRQ on GPIO4 so P1 connector pin #7
#define RFM95_RESET_PIN 17 // Reset on GPIO17 so P1 connector pin #11
#undef RFM95_LED  // Dragino Board as no LED to drive
#else
//Pin definitions for other board
#define RFM95_CS_PIN 8
#define RFM95_IRQ_PIN 25
#define RFM95_LED 4
#endif

//Client and Server Addresses
#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

//RFM95 Configuration
#ifdef EUROPE
#define RFM95_FREQUENCY  868.00
#else
#define RFM95_FREQUENCY 915.00
#endif
#define RFM95_TXPOWER 14

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS_PIN, RFM95_IRQ_PIN);
#ifdef DRAGINO_GPS_HAT
gps_MT3339 gps(GPS_DEVICE, &gps_data_mutex);
#endif
//Flag for Ctrl-C
int flag = 0;


// function to handle Lora device
void* lora_main(void* ptr)
{
#ifndef DRAGINO_GPS_HAT
	char temp[10];
	uint8_t data[] = "Hello World(xxxxx)!";
#else
	uint8_t data[50];
#endif
	uint16_t sequenceCounter = 0;

	printf("\nLORA Handler started\n");
	print_scheduler();
	print_scope();

	while(!flag)
	{
//		printf("Sending to rf95_server");
		// Send a message to rf95_server
#ifdef RFM95_LED
		gpioWrite(RFM95_LED, PI_ON);
#endif


#ifndef DRAGINO_GPS_HAT
		snprintf(temp,6,"%05d",sequenceCounter);
		strncpy((char*)data+12,temp,5);
#else
		pthread_mutex_lock(&gps_data_mutex);
		snprintf((char*)data,45,"(%05d):%10s,%10s,%11s",sequenceCounter,gps.getTimestamp(),gps.getLatitude(),gps.getLongitude());
		pthread_mutex_unlock(&gps_data_mutex);
#endif
		printf("Message=\"");
		printf("%s",(char*)data);
		printf("\"\n");
		rf95.send(data, sizeof(data));
		sequenceCounter++;
		rf95.waitPacketSent();

#ifdef RFM95_LED
		gpioWrite(RFM95_LED, PI_OFF);
#endif
		// Now wait for a reply
		uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
		uint8_t len = sizeof(buf);

		if (rf95.waitAvailableTimeout(5000))
		{
			// Should be a reply message for us now
			if (rf95.recv(buf, &len))
			{
#ifdef RFM95_LED
				gpioWrite(RFM95_LED, PI_ON);
#endif
				printf("got reply: ");
				printf("\"%s\"\n",(char*)buf);

#ifdef RFM95_LED
				gpioWrite(RFM95_LED, PI_OFF);
#endif
			}
			else
			{
				printf("recv failed");
			}
		}
		else
		{
			printf("No reply, is rf95_server running?\n");
		}
#ifdef DRAGINO_GPS_HAT
		pthread_yield();
#endif
	}
}


//Main Function
int main (int argc, const char* argv[] )
{
	// startup
	if (gpioInitialise()<0)
	{
		printf( "\n\nRPI rf95_client startup Failed.\n" );
		gpioTerminate();
		return 1;
	}


  gpioSetSignalFunc(2, sig_handler); //2 is SIGINT. Ctrl+C will cause signal.

  printf( "\nRPI rf95_client startup OK.\n" );
  printf( "\nRPI GPIO settings:\n" );
  printf("CS-> GPIO %d\n", (uint8_t) RFM95_CS_PIN);
  printf("IRQ-> GPIO %d\n", (uint8_t) RFM95_IRQ_PIN);
#ifdef RFM95_LED
  gpioSetMode(RFM95_LED, PI_OUTPUT);
  printf("\nINFO: LED on GPIO %d\n", (uint8_t) RFM95_LED);
  gpioWrite(RFM95_LED, PI_ON);
  gpioDelay(500000);
  gpioWrite(RFM95_LED, PI_OFF);
#endif

#ifdef RFM95_RESET_PIN
  printf( "RST-> GPIO %d\n", (uint8_t) RFM95_RESET_PIN );
  // Pulse a reset on module
  gpioSetMode(RFM95_RESET_PIN, PI_OUTPUT);
  digitalWrite(RFM95_RESET_PIN, LOW );
  gpioDelay(150);
  digitalWrite(RFM95_RESET_PIN, HIGH );
  gpioDelay(100);
#endif

  if (!rf95.init())
  {
	  printf( "\n\nRF95 driver failed to initialize.\n\n" );
	  gpioTerminate();
	  return 1;
  }

  printf("\nRFM 95 Device Version:0x%x\n",rf95.getDeviceVersion());
  /* Begin Manager/Driver settings code */
  printf("\nRFM 95 Settings:\n");
  printf("Frequency= %d MHz\n", (uint16_t) RFM95_FREQUENCY);
  printf("Power= %d\n", (uint8_t) RFM95_TXPOWER);
  printf("Client(This) Address= %d\n", CLIENT_ADDRESS);
  printf("Server Address= %d\n", SERVER_ADDRESS);
  rf95.setTxPower(RFM95_TXPOWER, false);
  rf95.setFrequency(RFM95_FREQUENCY);
  rf95.setThisAddress(CLIENT_ADDRESS);
  rf95.setHeaderFrom(CLIENT_ADDRESS);
  rf95.setHeaderTo(SERVER_ADDRESS);
  rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096);
  /* End Manager/Driver settings code */

  /* Begin Datagram Client Code */

#ifdef DRAGINO_GPS_HAT
  pthread_t read_gps_thread, lora_thread;
  int iret;
  pthread_mutex_init(&gps_data_mutex,NULL);
  iret = pthread_create( &read_gps_thread, NULL, ((void* (*)(void*))&gps_MT3339::read_gps),  &gps);
  if (iret != 0)
	  printf("\nCould not start thread to read gps data\n");
  iret = pthread_create( &lora_thread, NULL, lora_main,  NULL);
  if (iret != 0)
	  printf("\nCould not start thread to handle lora\n");

  print_scheduler();
#endif

// main loop
  while(!flag)
  {
#ifndef DRAGINO_GPS_HAT
	  lora_main(NULL);
#else
   sleep(1);
#endif
  }
  printf( "\nrf95_client Tester Ending\n" );
#ifdef DRAGINO_GPS_HAT
  pthread_join(lora_thread,NULL);
  pthread_join(read_gps_thread,NULL);
#endif
  gpioTerminate();
  return 0;
}

// signal handler terminating the program on CTRL-c
void sig_handler(int sig)
{
  flag=1;
#ifdef DRAGINO_GPS_HAT
  gps.stop();
#endif
}

