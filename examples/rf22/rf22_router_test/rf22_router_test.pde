// rf22_router_test.pde
// -*- mode: C++ -*-
//
// Test code used during library development, showing how
// to do various things, and how to call various functions

#include <SPI.h>
#include <RF22Router.h>

#define CLIENT_ADDRESS 1
#define ROUTER_ADDRESS 2
#define SERVER_ADDRESS 3

// Singleton instance of the radio
RF22Router rf22(CLIENT_ADDRESS);

void setup() 
{
  Serial.begin(9600);
  if (!rf22.init())
    Serial.println("RF22 init failed");
  // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
}

void test_routes()
{
  rf22.clearRoutingTable();
//  rf22.printRoutingTable();
  rf22.addRouteTo(1, 101);
  rf22.addRouteTo(2, 102);
  rf22.addRouteTo(3, 103);
  RF22Router::RoutingTableEntry* e;
  e = rf22.getRouteTo(0);
  if (e) // Should fail
    Serial.println("getRouteTo 0 failed");
    
  e = rf22.getRouteTo(1);
  if (!e)
    Serial.println("getRouteTo 1 failed");
  if (e->dest != 1)
    Serial.println("getRouteTo 2 failed");
  if (e->next_hop != 101)
    Serial.println("getRouteTo 3 failed");
  if (e->state != RF22Router::Valid)
    Serial.println("getRouteTo 4 failed");
    
  e = rf22.getRouteTo(2);
  if (!e)
    Serial.println("getRouteTo 5 failed");
  if (e->dest != 2)
    Serial.println("getRouteTo 6 failed");
  if (e->next_hop != 102)
    Serial.println("getRouteTo 7 failed");
  if (e->state != RF22Router::Valid)
    Serial.println("getRouteTo 8 failed");
    
  if (!rf22.deleteRouteTo(1))
      Serial.println("deleteRouteTo 1 failed");
  // Route to 1 should now be gone
  e = rf22.getRouteTo(1);
  if (e)
    Serial.println("deleteRouteTo 2 failed");
    
  Serial.println("-------------------");

//  rf22.printRoutingTable();
  delay(500);

}

uint8_t data[] = "Hello World!";
// Dont put this on the stack:
//uint8_t buf[RF22_MAX_MESSAGE_LEN];

void test_tx()
{
  rf22.addRouteTo(SERVER_ADDRESS, ROUTER_ADDRESS);
  uint8_t errorcode;
  errorcode = rf22.sendtoWait(data, sizeof(data), 100); // Should fail with no route
  if (errorcode != RF22_ROUTER_ERROR_NO_ROUTE)
    Serial.println("sendtoWait 1 failed");
  errorcode = rf22.sendtoWait(data, 255, 10); // Should fail too big
  if (errorcode != RF22_ROUTER_ERROR_INVALID_LENGTH)
    Serial.println("sendtoWait 2 failed");   
  errorcode = rf22.sendtoWait(data, sizeof(data), SERVER_ADDRESS); // Should fail after timeouts to 110
  if (errorcode != RF22_ROUTER_ERROR_UNABLE_TO_DELIVER)
    Serial.println("sendtoWait 3 failed");
  Serial.println("-------------------");
  delay(500);
}

void loop()
{
//  test_routes();
  test_tx();
}


