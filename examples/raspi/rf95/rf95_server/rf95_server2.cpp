// rf95_server.cpp
// -*- mode: C++ -*-
// Example app showing how to create a simple messageing server
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf95_client.
//
// Requires Pigpio GPIO library. Install by downloading and compiling from
// http://abyz.me.uk/rpi/pigpio/, or install via command line with 
// "sudo apt install pigpio". To use, run "make" at the command line in 
// the folder where this source code resides. Then execute application with
// sudo ./rf95_server.
// Tested on Raspberry Pi Zero and Zero W with LoRaWan/TTN RPI Zero Shield 
// by ElectronicTricks. Although this application builds and executes on
// Raspberry Pi 3, there seems to be missed messages and hangs.
// Strategically adding delays does seem to help in some cases.

//(9/20/2019)   Contributed by Brody M. Based off rf22_server.pde.
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
//Pin Definitions
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

//Main Function
int main (int argc, const char* argv[] )
{
  if (gpioInitialise()<0)
  {
    printf( "\n\nRPI rf95_server startup Failed.\n" );
    return 1;
  }
  gpioSetSignalFunc(2, sig_handler); //2 is SIGINT. Ctrl+C will cause signal.

  printf( "\nRPI rf95_server startup OK.\n" );
  printf( "\nRPI GPIO settings:\n" );
  printf("CS-> GPIO %d\n", (uint8_t) RFM95_CS_PIN);
  printf("IRQ-> GPIO %d\n", (uint8_t) RFM95_IRQ_PIN);
#ifdef RFM95_LED
  gpioSetMode(RFM95_LED, PI_OUTPUT);
  printf("LED-> GPIO %d\n", (uint8_t) RFM95_LED);
  gpioWrite(RFM95_LED, PI_ON);
  gpioDelay(500000);
  gpioWrite(RFM95_LED, PI_OFF);
#endif

  if (!rf95.init())
  {
    printf( "\n\nRF95 Driver Failed to initialize.\n\n" );
    return 1;
  }

  /* Begin Manager/Driver settings code */
  printf("\nRFM 95 Settings:\n");
  printf("Frequency= %d MHz\n", (uint16_t) RFM95_FREQUENCY);
  printf("Power= %d\n", (uint8_t) RFM95_TXPOWER);
  printf("Client Address= %d\n", CLIENT_ADDRESS);
  printf("Server(This) Address= %d\n", SERVER_ADDRESS);
  rf95.setTxPower(RFM95_TXPOWER, false);
  rf95.setFrequency(RFM95_FREQUENCY);
  rf95.setThisAddress(SERVER_ADDRESS);
  rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096);
  /* End Manager/Driver settings code */

  rf95.printRegisters();
  /* Begin Datagram Server Code */


  // start gps of dragino gps hat
#ifdef DRAGINO_GPS_HAT
  pthread_t read_gps_thread;
  int iret;
  pthread_mutex_init(&gps_data_mutex,NULL);
  iret = pthread_create( &read_gps_thread, NULL, ((void* (*)(void*))&gps_MT3339::read_gps),  &gps);
  if (iret != 0)
	  printf("\nCould not start thread to read gps data\n");



#endif

  print_scheduler();
  /* Begin Datagram Server Code */
  while(!flag)
  {
    if (rf95.available())
    {
      // Should be a message for us now
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);
      if (rf95.recv(buf, &len))
      {
#ifdef RFM95_LED
        gpioWrite(RFM95_LED, PI_ON);
#endif
//      RF95::printBuffer("request: ", buf, len);
        printf("got request: \"%s\"\n", (char*)buf);
        printf("RSSI: %d\n",rf95.lastRssi());

        uint8_t data[50] = "And hello back to you";

        //cut out sequence number from received message (format: "(...)") if in message
        // and copy to outgoing message
        char* temp1 = strchr((char*)buf,'(');
        char* temp2 = strchr((char*)buf,')');
        if ((temp1 != NULL) && (temp2 != NULL))
        {
        	// if received message contains sequence number,
        	// reply with received sequence number
        	temp1++;
        	uint8_t wordlen = temp2 - temp1;
        	temp1[wordlen] = 0x00;
#ifndef DRAGINO_GPS_HAT
        	snprintf((char*)data+21,8,"(%5s)",temp1);
#else
        	// In Case of Dragino Lora/GPS hat add timestamp and gps position of server to reply
        	pthread_mutex_lock(&gps_data_mutex);

        	snprintf((char*)data,47,"R:(%5s):%10s,%10s,%11s",
        			temp1,gps.getTimestamp(),gps.getLatitude(),gps.getLongitude());
        	pthread_mutex_unlock(&gps_data_mutex);
#endif
        }

        //Send a reply including a trailing 0x00
        rf95.send(data, strlen((char*)data)+1);
        rf95.waitPacketSent();
        printf("Sent a reply: \"%s\"\n",data);
#ifdef RFM95_LED
        gpioWrite(RFM95_LED, PI_OFF);
#endif
      }
      else
      {
        Serial.println("recv failed");
      }
    }
  }
  /* End Datagram Server Code */
  printf( "\nrf95_server Tester Ending\n" );
  gpioTerminate();
  return 0;
}

void sig_handler(int sig)
{
  flag=1;
}
