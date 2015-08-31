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

#include "RH_MRF89XA.h"

RH_MRF89XA::RH_MRF89XA(
        uint8_t slaveSelectPinData,
        uint8_t slaveSelectPinCommand,
        uint8_t irq0pin,
        uint8_t irq1pin, RHGenericSPI& spi)
    :
      RHNRFSPIDriver(slaveSelectPinData, slaveSelectPinCommand, spi),
      _irq0pin(irq0pin),
      _irq1pin(irq1pin),
      _rxBufValid(false)
{
}

bool RH_MRF89XA::init(){
    pinMode(_irq0pin, INPUT);
    pinMode(_irq1pin, INPUT);
    _spi.setFrequency(RHGenericSPI::Frequency1MHz);
    _spi.setDataMode(RHGenericSPI::DataMode0);
    _spi.setBitOrder(RHGenericSPI::BitOrderMSBFirst);
    if (!RHNRFSPIDriver::init()){
        return false;
    }
    //check if MRF89XA is responding
    /*uint8_t uiDummy=spiReadRegister(GCONREG);
    if(uiDummy!=0x28){
        return false;
    }*/
    spiWriteRegister(GCONREG, RH_MRF89XA_CMOD_STANDBY | (0x2<<3) | (0x3<<1) | 0x00);
    spiWriteRegister(DMODREG, (0x2<<6) | (0x0<<5) | (0x1<<2) | 0x00); //packet mode
    spiWriteRegister(FDEVREG, 0x9);//frequency deviation 40kHz
    spiWriteRegister(BRSREG, 0x9);//bitrate 20kpbs
    spiWriteRegister(FLTHREG, 0x0C);//default
    spiWriteRegister(FIFOCREG,(0x3<<6) | 0x01 );//64byte fifo, interrupt threshold at 1
    spiWriteRegister(R1CREG, 125);//values from github-x893 for 868MHz
    spiWriteRegister(P1CREG, 100);//868MHz = 9/8 * 12.8e6 / (R+1)*(75*(P+1)+S)
    spiWriteRegister(S1CREG, 20);
    spiWriteRegister(R2CREG, 0);
    spiWriteRegister(P2CREG, 0);
    spiWriteRegister(S2CREG, 0);
    spiWriteRegister(PACREG, 0x38);
    spiWriteRegister(FTXRXIREG, 0xC8);
    //IRQ0RXS=11b : in RX or standby: SYNC or ARDSMATCH
    //IRQ1RXS=00b : in RX or standby: CRCOK
    //IRQ1TX=1    : in TX mode : TXDONE
    spiWriteRegister(FTPRIREG, 0x0D);
    //RSSI as IRQ source, PLL lock detect enabled,
    //IRQ0TXST=0 : in TX mode: FIFO_THRESHOLD
    spiWriteRegister(RSTHIREG, 0x00);//default
    spiWriteRegister(FILCREG, 0x70 | 0x02); //234kHz passive filter | 75kHz Butterworth filter
    spiWriteRegister(PFCREG, 0x38);//default
    spiWriteRegister(SYNCREG, 0x38);//enable sync word recognition, 32bit sync word
    spiWriteRegister(RSVREG, 0x07);//default
    spiWriteRegister(OOKCREG, 0x00);//default
    spiWriteRegister(SYNCV31REG, 0x69); //byte3 of syncword
    spiWriteRegister(SYNCV23REG, 0x81); //byte2 of syncword
    spiWriteRegister(SYNCV15REG, 0x7E); //byte1 of syncword
    spiWriteRegister(SYNCV07REG, 0x96); //byte0 of syncword
    spiWriteRegister(TXCONREG, 0xF0);//TX cutoff freq=375kHz, 13dBm output
    spiWriteRegister(CLKOREG, 0x0);//disable clock output to save power
    spiWriteRegister(PLOADREG, 0x40);//payload=64bytes (no RX-filtering on packet length)
    spiWriteRegister(NADDSREG, 0x00);//node address (0=default)
    spiWriteRegister(PKTCREG, 0xE8);//variable length packet, 4byte preamble (=sync?), CRC enabled, RX-filtering on address disabled
    spiWriteRegister(FCRCREG, 0x00);//default (FIFO access in standby=write, clear FIFO on CRC mismatch)

    //Verify PLL-lock
    uint8_t yftpriVal=spiReadRegister(FTPRIREG);
    spiWriteRegister(FTPRIREG, yftpriVal | 0x2);//clear PLL lock bit by writing a 1 to it.
    setOperatingMode(RH_MRF89XA_CMOD_FS);//frequency synthesizer mode
    unsigned long ulStartTime=millis();
    while((millis()-ulStartTime<1000))
    {
        yftpriVal=spiReadRegister(FTPRIREG);
        if((yftpriVal & 0x2)!=0){
            break;
        }
    }
    setOperatingMode(RH_MRF89XA_CMOD_STANDBY);
    return ((yftpriVal & 0x2)!=0);
}

uint8_t RH_MRF89XA::spiBurstReadRegister(uint8_t reg, uint8_t* dest, uint8_t len)
{
    Serial.println("sorry, not implemented");
    return 0;
}

uint8_t RH_MRF89XA::spiBurstWriteRegister(uint8_t reg, uint8_t* src, uint8_t len)
{
    Serial.println("sorry, not implemented");
    return 0;
}


uint8_t RH_MRF89XA::spiReadRegister(uint8_t reg){
    setDataKind(DataKindCommand);
    return spiRead((RH_MRF89XA_COMMAND_R_REGISTER | (reg & RH_MRF89XA_REGISTER_MASK)) << 1 );
}

uint8_t RH_MRF89XA::spiWriteRegister(uint8_t reg, uint8_t val){
    setDataKind(DataKindCommand);
    return spiWrite((RH_MRF89XA_COMMAND_W_REGISTER | (reg & RH_MRF89XA_REGISTER_MASK)) << 1, val);
}

void RH_MRF89XA::writeFifo(uint8_t data){
    setDataKind(DataKindPayload);
    spiCommand(data);
}

uint8_t RH_MRF89XA::readFifo(){
    setDataKind(DataKindPayload);
    return spiCommand(0);
}

bool RH_MRF89XA::sleep()
{
    if(setOperatingMode(RH_MRF89XA_CMOD_SLEEP)){
        _mode = RHModeSleep;
        return true;
    }
    return false;
}

void RH_MRF89XA::setModeIdle()
{
    if(setOperatingMode(RH_MRF89XA_CMOD_STANDBY)){
        _mode = RHModeIdle;
    }
}

void RH_MRF89XA::setModeRx()
{
    if(setOperatingMode(RH_MRF89XA_CMOD_RX)){
        _mode = RHModeRx;
    }
}

void RH_MRF89XA::setModeTx()
{
    if(setOperatingMode(RH_MRF89XA_CMOD_TX)){
        _mode = RHModeTx;
    }
}

bool RH_MRF89XA::setOperatingMode(uint8_t om){
    uint8_t regVal=spiReadRegister(GCONREG);
    while(_opMode!=om){
        switch(_opMode){
        case RH_MRF89XA_CMOD_SLEEP:
            //Go to standby
            spiWriteRegister(GCONREG, (regVal & (~RH_MRF89XA_MASK_CMOD)) | RH_MRF89XA_CMOD_STANDBY);
            //Wait for XO setting, TSOSC
            delay(5);
            _opMode=RH_MRF89XA_CMOD_STANDBY;
            break;
        case RH_MRF89XA_CMOD_STANDBY:
            switch(om){
            case RH_MRF89XA_CMOD_SLEEP:
                //immediately go to a lower power mode
                spiWriteRegister(GCONREG, (regVal & (~RH_MRF89XA_MASK_CMOD)) | om);
                _opMode=om;
                break;
            case RH_MRF89XA_CMOD_FS:
            case RH_MRF89XA_CMOD_RX:
            case RH_MRF89XA_CMOD_TX:
                //Go to frequency synthesizer mode
                spiWriteRegister(GCONREG, (regVal & (~RH_MRF89XA_MASK_CMOD)) | RH_MRF89XA_CMOD_FS);
                //Wait for Receiver settling, TSFS
                delayMicroseconds(800);
                _opMode=RH_MRF89XA_CMOD_FS;
                break;
            default:
                Serial.println("invalid opmode");
                return false;
            }
            break;
        case RH_MRF89XA_CMOD_FS:
            switch(om){
            case RH_MRF89XA_CMOD_SLEEP:
            case RH_MRF89XA_CMOD_STANDBY:
                //immediately go to a lower power mode
                spiWriteRegister(GCONREG, (regVal & (~RH_MRF89XA_MASK_CMOD)) | om);
                _opMode=om;
                break;
            case RH_MRF89XA_CMOD_RX:
                //go to RX-mode
                spiWriteRegister(GCONREG, (regVal & (~RH_MRF89XA_MASK_CMOD)) | RH_MRF89XA_CMOD_RX);
                //wait TSRWF
                delayMicroseconds(800);
                _opMode=RH_MRF89XA_CMOD_RX;
                break;
            case RH_MRF89XA_CMOD_TX:
                //go to TX-mode
                spiWriteRegister(GCONREG, (regVal & (~RH_MRF89XA_MASK_CMOD)) | RH_MRF89XA_CMOD_TX);
                //wait TSTWF
                delayMicroseconds(500);
                _opMode=RH_MRF89XA_CMOD_TX;
                break;
            default:
                Serial.println("unknown opmode");
                return false;
            }
        case RH_MRF89XA_CMOD_RX:
        case RH_MRF89XA_CMOD_TX:
            //immediately go to any other mode
            spiWriteRegister(GCONREG, (regVal & (~RH_MRF89XA_MASK_CMOD)) | om);
            _opMode=om;
            break;
        default:
            Serial.println("invalid opmode");
            return false;
        }
    }
    return true;
}

bool RH_MRF89XA::available()
{
    if (_rxBufValid){
        return true;
    }
    if (_mode == RHModeTx){
        return false;
    }
    setModeRx();
    //IRQ1 in RX or standby: CRCOK => IRQ1=high
    if(digitalRead(_irq1pin)==LOW){
        return false;
    }
    setModeIdle();
    uint8_t yRegVal=spiReadRegister(FCRCREG);
    bitSet(yRegVal, 6);//set FRWAXS-bit (FIFO access = reading from FIFO)
    spiWriteRegister(FCRCREG,yRegVal);
    int i;
    for(i=-1;spiReadRegister(FTXRXIREG) & 0x2;i++){//bit1=/FIFOEMPTY
        if(i==-1){
            //read packet length
            _bufLen=readFifo();
        }else{
            _buf[i]=readFifo();
        }
    }
    if(i!=_bufLen){//the packet lengths should match

        return false;
    }
    validateRxBuf();
    if (_rxBufValid){
        setModeIdle(); // Got one
    }
    return _rxBufValid;
}

void RH_MRF89XA::validateRxBuf()
{
    if (_bufLen < 4){
    return; // Too short to be a real message
    }
    // Extract the 4 headers
    _rxHeaderTo    = _buf[0];
    _rxHeaderFrom  = _buf[1];
    _rxHeaderId    = _buf[2];
    _rxHeaderFlags = _buf[3];
    if (_promiscuous ||
    _rxHeaderTo == _thisAddress ||
    _rxHeaderTo == RH_BROADCAST_ADDRESS)
    {
    _rxGood++;
    _rxBufValid = true;
    }
}


void RH_MRF89XA::clearRxBuf()
{
    _rxBufValid = false;
    _bufLen = 0;
}

bool RH_MRF89XA::recv(uint8_t* buf, uint8_t* len)
{
    if (!available()){
        return false;
    }
    if (buf && len)
    {
        if (*len > _bufLen-RH_MRF89XA_HEADER_LEN){
            *len = _bufLen-RH_MRF89XA_HEADER_LEN;
        }
        memcpy(buf, _buf+RH_MRF89XA_HEADER_LEN, *len);
    }
    clearRxBuf(); // This message accepted and cleared
    return true;
}

uint8_t RH_MRF89XA::maxMessageLength()
{
    return RH_MRF89XA_MAX_MESSAGE_LEN;
}

bool RH_MRF89XA::send(const uint8_t* data, uint8_t len){
    if (len > RH_MRF89XA_MAX_MESSAGE_LEN){
        return false;
    }
    setModeIdle();
    uint8_t yRegVal=spiReadRegister(FCRCREG);
    bitClear(yRegVal, 6);//clear FRWAXS-bit (FIFO access = writing to FIFO)
    spiWriteRegister(FCRCREG,yRegVal);
    uint8_t yVal=spiReadRegister(FTXRXIREG);
    spiWriteRegister(FTXRXIREG, yVal | 0x1);//reset FIFO
    writeFifo(len+RH_MRF89XA_HEADER_LEN);//first write datalength
    // Set up the headers
    writeFifo(_txHeaderTo);
    writeFifo(_txHeaderFrom);
    writeFifo(_txHeaderId);
    writeFifo(_txHeaderFlags);
    for(int i=0;i<len;i++){
        writeFifo(data[i]);
    }
    setModeTx();
    _txGood++;
    return true;
}

bool RH_MRF89XA::waitPacketSent()
{
    // If we are not currently in transmit mode, there is no packet to wait for
    if (_mode != RHModeTx){
        return false;
    }
    unsigned long ulStartTime=millis();
    //IRQ1 in TX mode : TXDONE => IRQ1 = high
    while((millis()-ulStartTime<1000) && digitalRead(_irq1pin)==LOW){};
    setModeIdle();
    return digitalRead(_irq1pin)==LOW;
}
