// RHHardwareSPI2.h
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2011 Mike McCauley
// Contributed by Joanna Rutkowska
// $Id: RHHardwareSPI2.cpp,v 1.16 2016/07/07 00:02:53 mikem Exp mikem $
// This is a copy of the standard SPI node, that is hopefully setup to work on those processors
// who have SPI2.  Currently I only have it setup for Teensy 3.5/3.6 
#if defined(__arm__) && defined(TEENSYDUINO) && (defined(__MK64FX512__) || defined(__MK66FX1M0__) )

#include <RHHardwareSPI2.h>

// Declare a single default instance of the hardware SPI interface class
RHHardwareSPI2 hardware_spi2;

#ifdef RH_HAVE_HARDWARE_SPI

RHHardwareSPI2::RHHardwareSPI2(Frequency frequency, BitOrder bitOrder, DataMode dataMode)
    :
    RHGenericSPI(frequency, bitOrder, dataMode)
{
}

uint8_t RHHardwareSPI2::transfer(uint8_t data) 
{
    return SPI2.transfer(data);
}

void RHHardwareSPI2::attachInterrupt() 
{
#if (RH_PLATFORM == RH_PLATFORM_ARDUINO)
    SPI2.attachInterrupt();
#endif
}

void RHHardwareSPI2::detachInterrupt() 
{
#if (RH_PLATFORM == RH_PLATFORM_ARDUINO)
    SPI2.detachInterrupt();
#endif
}
    
void RHHardwareSPI2::begin() 
{
    // Sigh: there are no common symbols for some of these SPI options across all platforms
#if (RH_PLATFORM == RH_PLATFORM_ARDUINO) || (RH_PLATFORM == RH_PLATFORM_UNO32) || (RH_PLATFORM == RH_PLATFORM_CHIPKIT_CORE)
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
#else
 #if (RH_PLATFORM == RH_PLATFORM_ARDUINO) && defined (__arm__) && defined(ARDUINO_ARCH_SAMD)
    // Zero requires begin() before anything else :-)
    SPI2.begin();
 #endif

    SPI2.setDataMode(dataMode);
#endif
#if (RH_PLATFORM == RH_PLATFORM_ARDUINO) && defined(SPI_HAS_TRANSACTION)
    uint32_t frequency32;
    if (_frequency == Frequency16MHz) {
        frequency32 = 16000000;
    } else if (_frequency == Frequency8MHz) {
        frequency32 = 8000000;
    } else if (_frequency == Frequency4MHz) {
        frequency32 = 4000000;
    } else if (_frequency == Frequency2MHz) {
        frequency32 = 2000000;
    } else {
        frequency32 = 1000000;
    }
    _settings = SPISettings(frequency32, 
        (_bitOrder == BitOrderLSBFirst) ? LSBFIRST : MSBFIRST, 
        dataMode);
#endif
    

#if (RH_PLATFORM == RH_PLATFORM_ARDUINO) && defined (__arm__) && (defined(ARDUINO_SAM_DUE) || defined(ARDUINO_ARCH_SAMD))
    // Arduino Due in 1.5.5 has its own BitOrder :-(
    // So too does Arduino Zero
    ::BitOrder bitOrder;
#else
    uint8_t bitOrder;
#endif
    if (_bitOrder == BitOrderLSBFirst)
	bitOrder = LSBFIRST;
    else
	bitOrder = MSBFIRST;
    SPI2.setBitOrder(bitOrder);
    uint8_t divider;
    switch (_frequency)
    {
	case Frequency1MHz:
	default:
#if F_CPU == 8000000
	    divider = SPI_CLOCK_DIV8;
#else
	    divider = SPI_CLOCK_DIV16;
#endif
	    break;

	case Frequency2MHz:
#if F_CPU == 8000000
	    divider = SPI_CLOCK_DIV4;
#else
	    divider = SPI_CLOCK_DIV8;
#endif
	    break;

	case Frequency4MHz:
#if F_CPU == 8000000
	    divider = SPI_CLOCK_DIV2;
#else
	    divider = SPI_CLOCK_DIV4;
#endif
	    break;

	case Frequency8MHz:
	    divider = SPI_CLOCK_DIV2; // 4MHz on an 8MHz Arduino
	    break;

	case Frequency16MHz:
	    divider = SPI_CLOCK_DIV2; // Not really 16MHz, only 8MHz. 4MHz on an 8MHz Arduino
	    break;

    }

    SPI2.setClockDivider(divider);
    SPI2.begin();
    // Teensy requires it to be set _after_ begin()
    SPI2.setClockDivider(divider);

#else
 #warning RHHardwareSPI does not support this platform yet. Consider adding it and contributing a patch.
#endif
}

void RHHardwareSPI2::end() 
{
    return SPI2.end();
}

// If our platform is arduino and we support transactions then lets use the begin/end transaction
#if (RH_PLATFORM == RH_PLATFORM_ARDUINO) && defined(SPI_HAS_TRANSACTION)
void RHHardwareSPI2::beginTransaction()
{
  SPI2.beginTransaction(_settings);
}

void RHHardwareSPI2::endTransaction() 
{
  SPI2.endTransaction();
}
 #endif

#endif

#endif