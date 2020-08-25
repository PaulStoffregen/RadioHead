// rf95_router_server3.cpp
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, routed reliable messaging server
// with the RHRouter class.
// It is designed to work with the other example rf95_router_client.
//
// Requires Pigpio GPIO library. Install by downloading and compiling from
// http://abyz.me.uk/rpi/pigpio/, or install via command line with 
// "sudo apt install pigpio". To use, run "make" at the command line in 
// the folder where this source code resides. Then execute application with
// sudo ./rf95_router_server3.
// Tested on Raspberry Pi Zero and Zero W with LoRaWan/TTN RPI Zero Shield 
// by ElectronicTricks. Although this application builds and executes on
// Raspberry Pi 3, there seems to be missed messages and hangs.
// Strategically adding delays does seem to help in some cases.

//(9/20/2019)   Contributed by Brody M. Based off rf22_router_server3.pde.
//              Raspberry Pi mods influenced by nrf24 example by Mike Poublon,
//              and Charles-Henri Hallard (https://github.com/hallard/RadioHead)


#include <pigpio.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <RHRouter.h>
#include <RH_RF95.h>

//Function Definitions
void sig_handler(int sig);

//Pin Definitions
#define RFM95_CS_PIN 8
#define RFM95_IRQ_PIN 25
#define RFM95_LED 4

// In this small artifical network of 4 nodes,
// messages are routed via intermediate nodes to their destination
// node. All nodes can act as routers
// CLIENT_ADDRESS <-> SERVER1_ADDRESS <-> SERVER2_ADDRESS<->SERVER3_ADDRESS
#define CLIENT_ADDRESS 1
#define SERVER1_ADDRESS 2
#define SERVER2_ADDRESS 3
#define SERVER3_ADDRESS 4

//RFM95 Configuration
#define RFM95_FREQUENCY  915.00
#define RFM95_TXPOWER 14

// Create an instance of a driver
RH_RF95 rf95(RFM95_CS_PIN, RFM95_IRQ_PIN);

// Class to manage message delivery and receipt, using the driver declared above
RHRouter manager(rf95, SERVER3_ADDRESS);


//Flag for Ctrl-C
int flag = 0;

//Main Function
int main (int argc, const char* argv[] )
{
  if (gpioInitialise()<0)
  {
    printf( "\n\nRPI rf95_router_server3 startup Failed.\n" );
    return 1;
  }

  gpioSetSignalFunc(2, sig_handler); //2 is SIGINT. Ctrl+C will cause signal.

  printf( "\nRPI rf95_router_server3 startup OK.\n" );

#ifdef RFM95_LED
  gpioSetMode(RFM95_LED, PI_OUTPUT);
  printf("\nINFO: LED on GPIO %d\n", (uint8_t) RFM95_LED);
  gpioWrite(RFM95_LED, PI_ON);
  gpioDelay(500000);
  gpioWrite(RFM95_LED, PI_OFF);
#endif

  if (!manager.init())
  {
    printf( "\n\nRouter Manager Failed to initialize.\n\n" );
    return 1;
  }

  /* Begin Manager/Driver settings code */
  printf("\nRFM 95 Settings:\n");
  printf("Frequency= %d MHz\n", (uint16_t) RFM95_FREQUENCY);
  printf("Power= %d\n", (uint8_t) RFM95_TXPOWER);
  printf("Client Address= %d\n", CLIENT_ADDRESS);
  printf("Server Address 1 = %d\n", SERVER1_ADDRESS);
  printf("Server Address 2 = %d\n", SERVER2_ADDRESS);
  printf("Server(This) Address 3 = %d\n", SERVER3_ADDRESS);
  printf("Route: Client-> Server 1-> Server 2-> Server 3\n");
  rf95.setTxPower(RFM95_TXPOWER, false);
  rf95.setFrequency(RFM95_FREQUENCY);
  rf95.setThisAddress(SERVER3_ADDRESS);

  // Manually define the routes for this network
  manager.addRouteTo(CLIENT_ADDRESS, CLIENT_ADDRESS);
  manager.addRouteTo(SERVER2_ADDRESS, SERVER2_ADDRESS);
  manager.addRouteTo(SERVER3_ADDRESS, SERVER2_ADDRESS);
  /* End Manager/Driver settings code */

  uint8_t data[] = "And hello back to you from server3";
  // Dont put this on the stack:
  uint8_t buf[RH_ROUTER_MAX_MESSAGE_LEN];

  while(!flag)
  {
  	uint8_t len = sizeof(buf);
  	uint8_t from;
  	if (manager.recvfromAck(buf, &len, &from))
  	{
#ifdef RFM95_LED
    	gpioWrite(RFM95_LED, PI_ON);
#endif
    	Serial.print("got request from : 0x");
    	Serial.print(from, HEX);
    	Serial.print(": ");
     	Serial.println((char*)buf);

    	// Send a reply back to the originator client
    	if (manager.sendtoWait(data, sizeof(data), from) != RH_ROUTER_ERROR_NONE)
      		Serial.println("sendtoWait failed");
#ifdef RFM95_LED
    	gpioWrite(RFM95_LED, PI_OFF);
#endif
  	}
  }
  printf( "\nrf95_router_server3 Tester Ending\n" );
  gpioTerminate();
  return 0;

}

void sig_handler(int sig)
{
  flag=1;
}

