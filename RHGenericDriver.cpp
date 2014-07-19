// RHGenericDriver.cpp
//
// Copyright (C) 2014 Mike McCauley
// $Id: RHGenericDriver.cpp,v 1.11 2014/07/01 01:23:58 mikem Exp mikem $

#include <RHGenericDriver.h>

RHGenericDriver::RHGenericDriver()
    :
    _mode(RHModeInitialising),
    _thisAddress(RH_BROADCAST_ADDRESS),
    _txHeaderTo(RH_BROADCAST_ADDRESS),
    _txHeaderFrom(RH_BROADCAST_ADDRESS),
    _txHeaderId(0),
    _txHeaderFlags(0),
    _rxBad(0),
    _rxGood(0),
    _txGood(0)
{
}

bool RHGenericDriver::init()
{
    return true;
}

// Blocks until a valid message is received
void RHGenericDriver::waitAvailable()
{
    while (!available())
	YIELD;
}

// Blocks until a valid message is received or timeout expires
// Return true if there is a message available
// Works correctly even on millis() rollover
bool RHGenericDriver::waitAvailableTimeout(uint16_t timeout)
{
    unsigned long starttime = millis();
    while ((millis() - starttime) < timeout)
    {
        if (available())
           return true;
	YIELD;
    }
    return false;
}

bool RHGenericDriver::waitPacketSent()
{
    while (_mode == RHModeTx)
	YIELD; // Wait for any previous transmit to finish
    return true;
}

bool RHGenericDriver::waitPacketSent(uint16_t timeout)
{
    unsigned long starttime = millis();
    while ((millis() - starttime) < timeout)
    {
        if (_mode != RHModeTx) // Any previous transmit finished?
           return true;
	YIELD;
    }
    return false;
}

void RHGenericDriver::setPromiscuous(bool promiscuous)
{
    _promiscuous = promiscuous;
}

void RHGenericDriver::setThisAddress(uint8_t address)
{
    _thisAddress = address;
}

void RHGenericDriver::setHeaderTo(uint8_t to)
{
    _txHeaderTo = to;
}

void RHGenericDriver::setHeaderFrom(uint8_t from)
{
    _txHeaderFrom = from;
}

void RHGenericDriver::setHeaderId(uint8_t id)
{
    _txHeaderId = id;
}

void RHGenericDriver::setHeaderFlags(uint8_t set, uint8_t clear)
{
    _txHeaderFlags |= set;
    _txHeaderFlags &= ~clear;
}

uint8_t RHGenericDriver::headerTo()
{
    return _rxHeaderTo;
}

uint8_t RHGenericDriver::headerFrom()
{
    return _rxHeaderFrom;
}

uint8_t RHGenericDriver::headerId()
{
    return _rxHeaderId;
}

uint8_t RHGenericDriver::headerFlags()
{
    return _rxHeaderFlags;
}

int8_t RHGenericDriver::lastRssi()
{
    return _lastRssi;
}

RHGenericDriver::RHMode  RHGenericDriver::mode()
{
    return _mode;
}

void  RHGenericDriver::setMode(RHMode mode)
{
    _mode = mode;
}

// Diagnostic help
void RHGenericDriver::printBuffer(const char* prompt, const uint8_t* buf, uint8_t len)
{
    uint8_t i;

    Serial.println(prompt);
    for (i = 0; i < len; i++)
    {
	if (i % 16 == 15)
	    Serial.println(buf[i], HEX);
	else
	{
	    Serial.print(buf[i], HEX);
	    Serial.print(' ');
	}
    }
    Serial.println(' ');
}

