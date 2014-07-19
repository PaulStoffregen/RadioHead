// serial_reliable_datagram_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging server
// with the RHReliableDatagram class, using the RH_Serial driver to
// communicate using packets over a serial port (or a radio connected to a
// serial port, such as the 3DR Telemetry radio V1 and others).
// It is designed to work with the other example serial_reliable_datagram_client
// Tested on Arduino Mega and ChipKit Uno32 (normal Arduinos only have one
// serial port and so it not possible to test on them and still have debug
// output)
// Tested with Arduino Mega, Teensy 3.1, Moteino, Arduino Due


#include <RHReliableDatagram.h>
#include <RH_Serial.h>
#include <SPI.h>

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// Singleton instance of the Serial driver, configured
// to use the port Serial1. Caution: on Uno32, Serial1 is on pins 39 (Rx) and
// 40 (Tx)
RH_Serial driver(Serial1);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);

void setup() 
{
  Serial.begin(9600);
  // Configure the port RH_Serial will use:
  Serial1.begin(9600);
  if (!manager.init())
    Serial.println("init failed");
}

uint8_t data[] = "And hello back to you";
// Dont put this on the stack:
uint8_t buf[RH_SERIAL_MAX_MESSAGE_LEN];

void loop()
{
  if (manager.available())
  {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      Serial.print("got request from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);

      // Send a reply back to the originator client
      if (!manager.sendtoWait(data, sizeof(data), from))
        Serial.println("sendtoWait failed");
    }
  }
}

