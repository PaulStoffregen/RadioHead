// RHHardwareSPI.h
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2011 Mike McCauley
// Contributed by Joanna Rutkowska
// $Id: RHHardwareSPI.cpp,v 1.11 2014/08/12 00:54:52 mikem Exp $

#include "RHHardwareSPI.h"

// Declare a single default instance of the hardware SPI interface class
RHHardwareSPI hardware_spi;

#ifdef RH_HAVE_HARDWARE_SPI

#if (RH_PLATFORM == RH_PLATFORM_STM32 || RH_PLATFORM == RH_PLATFORM_STM32STD) // Maple etc
// Declare an SPI interface to use
HardwareSPI SPI(1);
#endif

RHHardwareSPI::RHHardwareSPI(Frequency frequency, BitOrder bitOrder, DataMode dataMode)
    :
      RHGenericSPI(frequency, bitOrder, dataMode)
{
}

uint8_t RHHardwareSPI::transfer(uint8_t data) 
{
#if RH_PLATFORM == RH_PLATFORM_ARDUINO
    SPI.beginTransaction(_spiSettings);
#endif
    byte outByte=SPI.transfer(data);
#if RH_PLATFORM == RH_PLATFORM_ARDUINO
    SPI.endTransaction();
#endif
    return outByte;
}

void RHHardwareSPI::attachInterrupt() 
{
#if (RH_PLATFORM == RH_PLATFORM_ARDUINO)
    SPI.attachInterrupt();
#endif
}

void RHHardwareSPI::detachInterrupt() 
{
#if (RH_PLATFORM == RH_PLATFORM_ARDUINO)
    SPI.detachInterrupt();
#endif
}

void RHHardwareSPI::begin() 
{
    // Sigh: there are no common symbols for some of these SPI options across all platforms
#if (RH_PLATFORM == RH_PLATFORM_ARDUINO) || (RH_PLATFORM == RH_PLATFORM_UNO32)
    uint8_t dataMode;
    if (_dataMode == DataMode0)
        dataMode = SPI_MODE0;
    else if (_dataMode == DataMode1)
        dataMode = SPI_MODE1;
    else if (_dataMode == DataMode2)
        dataMode = SPI_MODE2;
    else if (_dataMode == DataMode3)
        dataMode = SPI_MODE3;
    else
        dataMode = SPI_MODE0;
#if (RH_PLATFORM == RH_PLATFORM_ARDUINO) && defined(__arm__) && defined(CORE_TEENSY)
    // Temporary work-around due to problem where avr_emulation.h does not work properly for the setDataMode() cal
    SPCR &= ~SPI_MODE_MASK;
#endif

#if (RH_PLATFORM == RH_PLATFORM_ARDUINO) && defined (__arm__) && !defined(CORE_TEENSY)
    // Arduino Due and Nucleo in 1.5.5 have their own BitOrder :-(
    ::BitOrder bitOrder;
#else
    uint8_t bitOrder;
#endif
    if (_bitOrder == BitOrderLSBFirst)
        bitOrder = LSBFIRST;
    else
        bitOrder = MSBFIRST;

    uint32_t freq;
    switch(_frequency){
    case Frequency1MHz:
        freq=1000000;
        break;
    case Frequency2MHz:
        freq=2000000;
        break;
    case Frequency4MHz:
        freq=4000000;
        break;
    case Frequency8MHz:
        freq=8000000;
        break;
    case Frequency16MHz:
        freq=16000000;
        break;
    }
    _spiSettings=SPISettings(freq, bitOrder, dataMode);
    SPI.begin();

#elif (RH_PLATFORM == RH_PLATFORM_STM32) // Maple etc
    spi_mode dataMode;
    // Hmmm, if we do this as a switch, GCC on maple gets v confused!
    if (_dataMode == DataMode0)
        dataMode = SPI_MODE_0;
    else if (_dataMode == DataMode1)
        dataMode = SPI_MODE_1;
    else if (_dataMode == DataMode2)
        dataMode = SPI_MODE_2;
    else if (_dataMode == DataMode3)
        dataMode = SPI_MODE_3;
    else
        dataMode = SPI_MODE_0;

    uint32 bitOrder;
    if (_bitOrder == BitOrderLSBFirst)
        bitOrder = LSBFIRST;
    else
        bitOrder = MSBFIRST;

    SPIFrequency frequency; // Yes, I know these are not exact equivalents.
    switch (_frequency)
    {
    case Frequency1MHz:
    default:
        frequency = SPI_1_125MHZ;
        break;

    case Frequency2MHz:
        frequency = SPI_2_25MHZ;
        break;

    case Frequency4MHz:
        frequency = SPI_4_5MHZ;
        break;

    case Frequency8MHz:
        frequency = SPI_9MHZ;
        break;

    case Frequency16MHz:
        frequency = SPI_18MHZ;
        break;

    }
    SPI.begin(frequency, bitOrder, dataMode);

#elif (RH_PLATFORM == RH_PLATFORM_STM32STD) // STM32F4 discovery
    uint8_t dataMode;
    if (_dataMode == DataMode0)
        dataMode = SPI_MODE0;
    else if (_dataMode == DataMode1)
        dataMode = SPI_MODE1;
    else if (_dataMode == DataMode2)
        dataMode = SPI_MODE2;
    else if (_dataMode == DataMode3)
        dataMode = SPI_MODE3;
    else
        dataMode = SPI_MODE0;

    uint32_t bitOrder;
    if (_bitOrder == BitOrderLSBFirst)
        bitOrder = LSBFIRST;
    else
        bitOrder = MSBFIRST;

    SPIFrequency frequency; // Yes, I know these are not exact equivalents.
    switch (_frequency)
    {
    case Frequency1MHz:
    default:
        frequency = SPI_1_3125MHZ;
        break;

    case Frequency2MHz:
        frequency = SPI_2_625MHZ;
        break;

    case Frequency4MHz:
        frequency = SPI_5_25MHZ;
        break;

    case Frequency8MHz:
        frequency = SPI_10_5MHZ;
        break;

    case Frequency16MHz:
        frequency = SPI_21_0MHZ;
        break;

    }
    SPI.begin(frequency, bitOrder, dataMode);
#else
#warning RHHardwareSPI does not support this platform yet. Consider adding it and contributing a patch.
#endif
}

void RHHardwareSPI::end() 
{
    return SPI.end();
}

#endif

