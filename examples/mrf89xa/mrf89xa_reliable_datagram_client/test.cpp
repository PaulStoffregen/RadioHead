/*Demo application of MRF89XA driver
    Copyright (C) 2014  Christoph Tack

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//#define SERVER_MRF89XA
#define CLIENT_MRF89XA

#include <RH_MRF89XA.h>
#include <RHReliableDatagram.h>

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

//  IRQ1    = MRF89XAM8A: pin 9 => Arduino Uno pin 2
//  /CSDATA = MRF89XAM8A: pin 8 => Arduino Uno pin 3
//  /CSCON  = MRF89XAM8A: pin 3 => Arduino Uno pin 5
//  IRQ0    = MRF89XAM8A: pin 4 => Arduino Uno pin 4
RH_MRF89XA driver(3, 5, 4, 2);

// Class to manage message delivery and receipt, using the driver declared above
#ifdef CLIENT_MRF89XA
#ifdef SERVER_MRF89XA
#error You have to choose: client OR server, not both
#endif
RHReliableDatagram manager(driver, CLIENT_ADDRESS);
#elif defined(SERVER_MRF89XA)
#ifdef CLIENT_MRF89XA
#error You have to choose: client OR server, not both
#endif
RHReliableDatagram manager(driver, SERVER_ADDRESS);
#endif

uint8_t data[] = "Hello World!";
// Dont put this on the stack:
uint8_t buf[RH_MRF89XA_MAX_MESSAGE_LEN];

void setup()
{
    Serial.begin(9600);
    if (!manager.init()){
        Serial.println("init failed");
        return;
    }
    Serial.println("init ok");
}

void loop()
{
#ifdef CLIENT_MRF89XA
    Serial.println("Sending to mrf89xa_reliable_datagram_server");

    // Send a message to manager_server
    if (manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS))
    {
        // Now wait for a reply from the server
        uint8_t len = sizeof(buf);
        uint8_t from;
        if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
        {
            Serial.print("got reply from : 0x");
            Serial.print(from, HEX);
            Serial.print(": ");
            Serial.println((char*)buf);
        }
        else
        {
            Serial.println("No reply, is mrf89xa_reliable_datagram_server running?");
        }
    }
    delay(500);

#elif defined(SERVER_MRF89XA)
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
        }else{
            Serial.println("not valid");
        }
    }
#endif
}
