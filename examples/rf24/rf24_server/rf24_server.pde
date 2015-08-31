// rf24_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing server
// with the RH_RF24 class. RH_RF24 class does not provide for addressing or
// reliability, so you should only use RH_RF24  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf24_client
// Tested on Anarduino Mini http://www.anarduino.com/mini/ with RFM24W and RFM26W

#include <SPI.h>
#include <RH_RF24.h>

// Singleton instance of the radio driver
RH_RF24 rf24;

void setup() 
{
  Serial.begin(9600);
  if (!rf24.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb5Fd10, power 0x10
//  if (!rf24.setFrequency(433.0))
//    Serial.println("setFrequency failed");

}

void loop()
{
  if (rf24.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF24_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf24.recv(buf, &len))
    {
//      RF24::printBuffer("request: ", buf, len);
      Serial.print("got request: ");
      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println((uint8_t)rf24.lastRssi(), DEC);
      
      // Send a reply
      uint8_t data[] = "And hello back to you";
      rf24.send(data, sizeof(data));
      rf24.waitPacketSent();
      Serial.println("Sent a reply");
    }
    else
    {
      Serial.println("recv failed");
    }
  }
}

