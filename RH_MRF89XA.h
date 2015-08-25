/*  MRF89XA driver
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

#ifndef RH_MRF89XA_h
#define RH_MRF89XA_h

#include "RHGenericSPI.h"
#include "RHNRFSPIDriver.h"

// This is the maximum number of bytes that can be carried by the MRF89XA.
// We use some for headers, keeping fewer for RadioHead messages
#define RH_MRF89XA_MAX_PAYLOAD_LEN 64

// The length of the headers we add.
// The headers are inside the MRF89XA payload
#define RH_MRF89XA_HEADER_LEN 4

// This is the maximum RadioHead user message length that can be supported by this library. Limited by
// the supported message lengths in the MRF89XA
#define RH_MRF89XA_MAX_MESSAGE_LEN (RH_MRF89XA_MAX_PAYLOAD_LEN-RH_MRF89XA_HEADER_LEN)


// Register names
const byte RH_MRF89XA_REGISTER_MASK=0x1f;
const byte GCONREG=0x00;
const byte DMODREG=0x01;
const byte FDEVREG=0x02;
const byte BRSREG=0x03;
const byte FLTHREG=0x04;
const byte FIFOCREG=0x05;
const byte R1CREG=0x06;
const byte P1CREG=0x07;
const byte S1CREG=0x08;
const byte R2CREG=0x09;
const byte P2CREG=0x0A;
const byte S2CREG=0x0B;
const byte PACREG=0x0C;
const byte FTXRXIREG=0x0D;
const byte FTPRIREG=0x0E;
const byte RSTHIREG=0x0F;
const byte FILCREG=0x10;
const byte PFCREG=0x11;
const byte SYNCREG=0x12;
const byte RSVREG=0x13;
const byte RSTSREG=0x14;
const byte OOKCREG=0x15;
const byte SYNCV31REG=0x16;
const byte SYNCV23REG=0x17;
const byte SYNCV15REG=0x18;
const byte SYNCV07REG=0x19;
const byte TXCONREG=0x1A;
const byte CLKOREG=0x1B;
const byte PLOADREG=0x1C;
const byte NADDSREG=0x1D;
const byte PKTCREG=0x1E;
const byte FCRCREG=0x1F;

// SPI Command names
const byte RH_MRF89XA_COMMAND_R_REGISTER=0x20;
const byte RH_MRF89XA_COMMAND_W_REGISTER=0x00;

//GCONREG
const byte RH_MRF89XA_MASK_CMOD=0xE0;
const byte RH_MRF89XA_CMOD_SLEEP=0x00;
const byte RH_MRF89XA_CMOD_STANDBY=0x20;
const byte RH_MRF89XA_CMOD_FS=0x40;
const byte RH_MRF89XA_CMOD_RX=0x60;
const byte RH_MRF89XA_CMOD_TX=0x80;

class RH_MRF89XA : public RHNRFSPIDriver
{
public:
    RH_MRF89XA(uint8_t slaveSelectPinData, uint8_t slaveSelectPinCommand, uint8_t irq0pin, uint8_t irq1pin, RHGenericSPI& spi = hardware_spi);
    bool init();
    uint8_t spiReadRegister(uint8_t reg);
    uint8_t spiWriteRegister(uint8_t reg, uint8_t val);
    uint8_t spiBurstReadRegister(uint8_t reg, uint8_t* dest, uint8_t len);
    uint8_t spiBurstWriteRegister(uint8_t reg, uint8_t* src, uint8_t len);
    virtual bool sleep();
    void setModeIdle();
    void setModeRx();
    void setModeTx();
    bool send(const uint8_t* data, uint8_t len);
    virtual bool waitPacketSent();
    bool available();
    bool recv(uint8_t* buf, uint8_t* len);
    uint8_t maxMessageLength();
    void validateRxBuf();
    void clearRxBuf();

private:
    void writeFifo(uint8_t data);
    bool setOperatingMode(uint8_t om);
    uint8_t readFifo();
    uint8_t _irq0pin;
    uint8_t _irq1pin;
    bool _rxBufValid;
    uint8_t _opMode;
    uint8_t _bufLen;
    uint8_t _buf[RH_MRF89XA_MAX_PAYLOAD_LEN];
};
#endif
