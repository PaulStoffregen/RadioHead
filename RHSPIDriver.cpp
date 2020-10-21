// RHSPIDriver.cpp
//
// Copyright (C) 2014 Mike McCauley
// $Id: RHSPIDriver.cpp,v 1.13 2020/08/04 09:02:14 mikem Exp $

#include <RHSPIDriver.h>

// Some platforms may need special slave select driving

RHSPIDriver::RHSPIDriver(uint8_t slaveSelectPin, RHGenericSPI& spi)
    : 
    _spi(spi),
    _slaveSelectPin(slaveSelectPin)
{
}

bool RHSPIDriver::init()
{
    // start the SPI library with the default speeds etc:
    // On Arduino Due this defaults to SPI1 on the central group of 6 SPI pins
    _spi.begin();

    // Initialise the slave select pin
    // On Maple, this must be _after_ spi.begin

    // Sometimes we dont want to work the _slaveSelectPin here
    if (_slaveSelectPin != 0xff)
	pinMode(_slaveSelectPin, OUTPUT);

    deselectSlave();

    // This delay is needed for ATMega and maybe some others, but
    // 100ms is too long for STM32L0, and somehow can cause the USB interface to fail
    // in some versions of the core.
#if (RH_PLATFORM == RH_PLATFORM_STM32L0) && (defined STM32L082xx || defined STM32L072xx)
    delay(10);
#else
    delay(100);
#endif
    
    return true;
}

uint8_t RHSPIDriver::spiRead(uint8_t reg)
{
    uint8_t val = 0;
    ATOMIC_BLOCK_START;
    _spi.beginTransaction();
    selectSlave();
    _spi.transfer(reg & ~RH_SPI_WRITE_MASK); // Send the address with the write mask off
    val = _spi.transfer(0); // The written value is ignored, reg value is read
    deselectSlave();
    _spi.endTransaction();
    ATOMIC_BLOCK_END;
    return val;
}

uint8_t RHSPIDriver::spiWrite(uint8_t reg, uint8_t val)
{
    uint8_t status = 0;
    ATOMIC_BLOCK_START;
    _spi.beginTransaction();
    selectSlave();
    status = _spi.transfer(reg | RH_SPI_WRITE_MASK); // Send the address with the write mask on
    _spi.transfer(val); // New value follows
    deselectSlave();
    _spi.endTransaction();
    ATOMIC_BLOCK_END;
    return status;
}

uint8_t RHSPIDriver::spiBurstRead(uint8_t reg, uint8_t* dest, uint8_t len)
{
    uint8_t status = 0;
    ATOMIC_BLOCK_START;
    _spi.beginTransaction();
    selectSlave();
    status = _spi.transfer(reg & ~RH_SPI_WRITE_MASK); // Send the start address with the write mask off
    while (len--)
	*dest++ = _spi.transfer(0);
    deselectSlave();
    _spi.endTransaction();
    ATOMIC_BLOCK_END;
    return status;
}

uint8_t RHSPIDriver::spiBurstWrite(uint8_t reg, const uint8_t* src, uint8_t len)
{
    uint8_t status = 0;
    ATOMIC_BLOCK_START;
    _spi.beginTransaction();
    selectSlave();
    status = _spi.transfer(reg | RH_SPI_WRITE_MASK); // Send the start address with the write mask on
    while (len--)
	_spi.transfer(*src++);
    deselectSlave();
    _spi.endTransaction();
    ATOMIC_BLOCK_END;
    return status;
}

void RHSPIDriver::setSlaveSelectPin(uint8_t slaveSelectPin)
{
    _slaveSelectPin = slaveSelectPin;
}

void RHSPIDriver::spiUsingInterrupt(uint8_t interruptNumber)
{
    _spi.usingInterrupt(interruptNumber);
}

void RHSPIDriver::selectSlave()
{
    digitalWrite(_slaveSelectPin, LOW);
}
    
void RHSPIDriver::deselectSlave()
{
    digitalWrite(_slaveSelectPin, HIGH);
}
