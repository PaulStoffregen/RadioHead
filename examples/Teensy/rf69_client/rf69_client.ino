// rf69_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_RF69 class. RH_RF69 class does not provide for addressing or
// reliability, so you should only use RH_RF69  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf69_server.
// Demonstrates the use of AES encryption, setting the frequency and modem 
// configuration
// Tested on Moteino with RFM69 http://lowpowerlab.com/moteino/
// Tested on miniWireless with RFM69 www.anarduino.com/miniwireless
// Tested on Teensy 3.1 with RF69 on PJRC breakout board

#include <SPI.h>
#include <RH_RF69.h>

//For Teensy 3.x and T4.x the following format is required to operate correctly
//This is a limitation of the RadioHead radio drivers
#define RFM69_RST     3
#define RFM69_CS      10
#define RFM69_INT     digitalPinToInterrupt(2)

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS,RFM69_INT);
//----- END TEENSY CONFIG

//RH_RF69 rf69(15, 16); // For RF69 on PJRC breakout board with Teensy 3.1

void setup() 
{
  Serial.begin(9600);
  
  //For Teensy 3.x and T4.x the following format is required to operate correctly
  //pinMode(LED, OUTPUT);     
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println("Teensy RFM69 Client!");
  Serial.println();

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
//----- END TEENSY CONFIG

  if (!rf69.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  // No encryption
  if (!rf69.setFrequency(433.0))
    Serial.println("setFrequency failed");

  // If you are using a high power RF69, you *must* set a Tx power in the
  // range 14 to 20 like this:
   rf69.setTxPower(14);

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
}


void loop()
{
  Serial.println("Sending to rf69_server");
  // Send a message to rf69_server
  uint8_t data[] = "Hello World!";
  rf69.send(data, sizeof(data));
  
  rf69.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (rf69.waitAvailableTimeout(500))
  { 
    // Should be a reply message for us now   
    if (rf69.recv(buf, &len))
    {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is rf69_server running?");
  }
  delay(400);
}
