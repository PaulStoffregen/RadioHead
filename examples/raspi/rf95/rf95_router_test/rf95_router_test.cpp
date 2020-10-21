// rf95_router_test.cpp
// -*- mode: C++ -*-
//
// Test code used during library development, showing how
// to do various things, and how to call various functions
//
// Requires Pigpio GPIO library. Install by downloading and compiling from
// http://abyz.me.uk/rpi/pigpio/, or install via command line with 
// "sudo apt install pigpio". To use, run "make" at the command line in 
// the folder where this source code resides. Then execute application with
// sudo ./rf95_router_test.
// Tested on Raspberry Pi Zero and Zero W with LoRaWan/TTN RPI Zero Shield 
// by ElectronicTricks. Although this application builds and executes on
// Raspberry Pi 3, there seems to be missed messages and hangs.
// Strategically adding delays does seem to help in some cases.

//(10/6/2019)   Contributed by Brody M. Based off rf22_router_tester.pde.
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
void test_tx(void);
void test_routes(void);

uint8_t data[] = "Hello World!";
// Dont put this on the stack:
//uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

//Pin Definitions
#define RFM95_CS_PIN 8
#define RFM95_IRQ_PIN 25
#define RFM95_LED 4

#define CLIENT_ADDRESS 1
#define ROUTER_ADDRESS 2
#define SERVER_ADDRESS 3

//RFM95 Configuration
#define RFM95_FREQUENCY  915.00
#define RFM95_TXPOWER 14

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS_PIN, RFM95_IRQ_PIN);

// Class to manage message delivery and receipt, using the driver declared above
RHRouter manager(rf95, CLIENT_ADDRESS);

//Flag for Ctrl-C
int flag = 0;

//Main Function
int main (int argc, const char* argv[] )
{
  if (gpioInitialise()<0)
  {
    printf( "\n\nRPI rf95_router_tester startup Failed.\n" );
    return 1;
  }

  gpioSetSignalFunc(2, sig_handler); //2 is SIGINT. Ctrl+C will cause signal.

  printf( "\nRPI rf95_router_tester startup OK.\n" );

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
  printf("Client(This) Address= %d\n", CLIENT_ADDRESS);
  printf("Router Address = %d\n", ROUTER_ADDRESS);
  printf("Server Address = %d\n", SERVER_ADDRESS);
  rf95.setTxPower(RFM95_TXPOWER, false);
  rf95.setFrequency(RFM95_FREQUENCY);
  /* End Manager/Driver settings code */
 
  while(!flag)
  {
    Serial.println("Running test function...");
#ifdef RFM95_LED
    gpioWrite(RFM95_LED, PI_ON);
#endif
    //  test_routes();
    test_tx();
#ifdef RFM95_LED
    gpioWrite(RFM95_LED, PI_OFF);
#endif
    gpioDelay(500000);
  }
  printf( "\nrf95_router_test Tester Ending\n" );
  gpioTerminate();
  return 0;
}

void sig_handler(int sig)
{
  flag=1;
}

void test_routes()
{
  manager.clearRoutingTable();
//  manager.printRoutingTable();
  manager.addRouteTo(1, 101);
  manager.addRouteTo(2, 102);
  manager.addRouteTo(3, 103);
  RHRouter::RoutingTableEntry* e;
  e = manager.getRouteTo(0);
  if (e) // Should fail
    Serial.println("getRouteTo 0 failed");
    
  e = manager.getRouteTo(1);
  if (!e)
    Serial.println("getRouteTo 1 failed");
  if (e->dest != 1)
    Serial.println("getRouteTo 2 failed");
  if (e->next_hop != 101)
    Serial.println("getRouteTo 3 failed");
  if (e->state != RHRouter::Valid)
    Serial.println("getRouteTo 4 failed");
    
  e = manager.getRouteTo(2);
  if (!e)
    Serial.println("getRouteTo 5 failed");
  if (e->dest != 2)
    Serial.println("getRouteTo 6 failed");
  if (e->next_hop != 102)
    Serial.println("getRouteTo 7 failed");
  if (e->state != RHRouter::Valid)
    Serial.println("getRouteTo 8 failed");
    
  if (!manager.deleteRouteTo(1))
      Serial.println("deleteRouteTo 1 failed");
  // Route to 1 should now be gone
  e = manager.getRouteTo(1);
  if (e)
    Serial.println("deleteRouteTo 2 failed");
    
  Serial.println("-------------------");

//  manager.printRoutingTable();
  delay(500);

}

void test_tx()
{
  manager.addRouteTo(SERVER_ADDRESS, ROUTER_ADDRESS);
  uint8_t errorcode;
  errorcode = manager.sendtoWait(data, sizeof(data), 100); // Should fail with no route
  if (errorcode != RH_ROUTER_ERROR_NO_ROUTE)
    Serial.println("sendtoWait 1 failed");
  errorcode = manager.sendtoWait(data, 255, 10); // Should fail too big
  if (errorcode != RH_ROUTER_ERROR_INVALID_LENGTH)
    Serial.println("sendtoWait 2 failed");   
  errorcode = manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS); // Should fail after timeouts to 110
  if (errorcode != RH_ROUTER_ERROR_UNABLE_TO_DELIVER)
    Serial.println("sendtoWait 3 failed");
  Serial.println("-------------------");
  delay(500);
}

