// RHNRFSPIDriver.cpp
//
// Copyright (C) 2014 Mike McCauley
// $Id: RHNRFSPIDriver.cpp,v 1.2 2014/05/03 00:20:36 mikem Exp $

#include <RHNRFSPIDriver.h>

RHNRFSPIDriver::RHNRFSPIDriver(uint8_t slaveSelectPin, RHGenericSPI& spi)
    : 
    _spi(spi),
    _slaveSelectPin(slaveSelectPin),
    _slaveSelectPin2(slaveSelectPin),
    _currentDataMode(DataKindGeneric)
{
}

RHNRFSPIDriver::RHNRFSPIDriver(uint8_t slaveSelectPin, uint8_t slaveSelectPin2, RHGenericSPI& spi)
    :
    _spi(spi),
    _slaveSelectPin(slaveSelectPin),
    _slaveSelectPin2(slaveSelectPin2),
    _currentDataMode(DataKindGeneric)
{
}


bool RHNRFSPIDriver::init()
{
    // start the SPI library with the default speeds etc:
    // On Arduino Due this defaults to SPI1 on the central group of 6 SPI pins
    _spi.begin();

    // Initialise the slave select pin
    // On Maple, this must be _after_ spi.begin
    pinMode(_slaveSelectPin, OUTPUT);
    digitalWrite(_slaveSelectPin, HIGH);
    pinMode(_slaveSelectPin2, OUTPUT);
    digitalWrite(_slaveSelectPin2, HIGH);
    delay(100);
    return true;
}

void RHNRFSPIDriver::setDataKind(RHNRFSPIDriver::DataKind d){
    _currentDataMode=d;
}

void RHNRFSPIDriver::setSlaveSelect(uint8_t level){
    switch (_currentDataMode) {
    case DataKindGeneric:
    case DataKindPayload:
        digitalWrite(_slaveSelectPin, level);
        break;
    case DataKindCommand:
        digitalWrite(_slaveSelectPin2, level);
    default:
        break;
    }
}

// Low level commands for interfacing with the device
uint8_t RHNRFSPIDriver::spiCommand(uint8_t command)
{
    uint8_t status;
    ATOMIC_BLOCK_START;
    setSlaveSelect(LOW);
    status = _spi.transfer(command);
    setSlaveSelect(HIGH);
    ATOMIC_BLOCK_END;
    return status;
}

uint8_t RHNRFSPIDriver::spiRead(uint8_t reg)
{
    uint8_t val;
    ATOMIC_BLOCK_START;
    setSlaveSelect(LOW);
    _spi.transfer(reg); // Send the address, discard the status
    val = _spi.transfer(0); // The written value is ignored, reg value is read
    setSlaveSelect(HIGH);
    ATOMIC_BLOCK_END;
    return val;
}

uint8_t RHNRFSPIDriver::spiWrite(uint8_t reg, uint8_t val)
{
    uint8_t status = 0;
    ATOMIC_BLOCK_START;
    setSlaveSelect(LOW);
    status = _spi.transfer(reg); // Send the address
    _spi.transfer(val); // New value follows
    setSlaveSelect(HIGH);
    ATOMIC_BLOCK_END;
    return status;
}

uint8_t RHNRFSPIDriver::spiBurstRead(uint8_t reg, uint8_t* dest, uint8_t len)
{
    uint8_t status = 0;
    ATOMIC_BLOCK_START;
    setSlaveSelect(LOW);
    status = _spi.transfer(reg); // Send the start address
    while (len--)
	*dest++ = _spi.transfer(0);
    setSlaveSelect(HIGH);
    ATOMIC_BLOCK_END;
    return status;
}

uint8_t RHNRFSPIDriver::spiBurstWrite(uint8_t reg, const uint8_t* src, uint8_t len)
{
    uint8_t status = 0;
    ATOMIC_BLOCK_START;
    setSlaveSelect(LOW);
    status = _spi.transfer(reg); // Send the start address
    while (len--)
	_spi.transfer(*src++);
    setSlaveSelect(HIGH);
    ATOMIC_BLOCK_END;
    return status;
}



