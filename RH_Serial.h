// RH_Serial.h
//
// Copyright (C) 2014 Mike McCauley
// $Id: RH_Serial.h,v 1.7 2014/06/24 02:40:12 mikem Exp $

// Works with any serial port. Tested with Arduino Mega connected to Serial1
// Also works with 3DR Radio V1.3 Telemetry kit (serial at 57600baud)

#ifndef RH_Serial_h
#define RH_Serial_h

#include <RHGenericDriver.h>

// Special characters
#define STX 0x02
#define ETX 0x03
#define DLE 0x10
#define SYN 0x16

// Maximum message length (incgluding the headers) we are willing to support
#define RH_SERIAL_MAX_PAYLOAD_LEN 64

// The length of the headers we add.
// The headers are inside the payload and are therefore protected by the FCS
#define RH_SERIAL_HEADER_LEN 4

// This is the maximum message length that can be supported by this library. 
// It is an arbitrary limit.
// Can be pre-defined to a smaller size (to save SRAM) prior to including this header
// Here we allow for 4 bytes of address and header and payload to be included in the 64 byte encryption limit.
// the one byte payload length is not encrpyted
#ifndef RH_SERIAL_MAX_MESSAGE_LEN
#define RH_SERIAL_MAX_MESSAGE_LEN (RH_SERIAL_MAX_PAYLOAD_LEN - RH_SERIAL_HEADER_LEN)
#endif

class HardwareSerial;

/////////////////////////////////////////////////////////////////////
/// \class RH_Serial RH_Serial.h <RH_Serial.h>
/// \brief Driver to send and receive unaddressed, unreliable datagrams via a serial connection
///
/// This class sends and received packetized messages over a serial connection.
/// It can be used for point-to-point or multidrop, RS232, RS488 or other serial connections as
/// supported by your controller hardware.
/// It can also be used to communicate via radios with serial interfaces such as:
/// - APC220 Radio Data Module http://www.dfrobot.com/image/data/TEL0005/APC220_Datasheet.pdf
///   http://www.dfrobot.com/image/data/TEL0005/APC220_Datasheet.pdf
/// - 3DR Telemetry Radio https://store.3drobotics.com/products/3dr-radio
/// - HopeRF HM-TR module http://www.hoperf.com/upload/rf_app/HM-TRS.pdf
/// - Others
///
/// The packetised messages include message encapsulation, headers, a message payload and a checksum.
///
/// \par Packet Format
///
/// All messages sent and received by this RH_Serial Driver conform to this packet format:
/// \code
/// DLE 
/// STX
/// TO Header                (1 octet)
/// FROM Header              (1 octet)
/// ID Header                (1 octet)
/// FLAGS Header             (1 octet)
/// Message payload          (0 to 60 octets)
/// DLE
/// ETX
/// Frame Check Sequence FCS CCITT CRC-16 (2 octets)
/// \endcode
///
/// If any of octets from TO header through to the end of the payload are a DLE, 
/// then they are preceded by a DLE (ie DLE stuffing).
/// The FCS covers everything from the TO header to the ETX inclusive, but not any stuffed DLEs
///
/// \par Physical connection
///
/// The physical connection to your serial port will depend on the type of platform you are on.
///
/// For example, many arduinos only support a single Serial port on pins 0 and 1, 
/// which is shared with the USB host connections. On such Arduinos, it is not possible to use both 
/// RH_Serial on the Serial port as well as using the Serial port for debugand other printing or communications.
/// 
/// On Arduino Mega and Due, there are 4 serial ports:
/// - Serial: this is the serial port connected to the USB interface and the programming host.
/// - Serial1: on pins 18 (Tx) and 19 (Rx)
/// - Serial2: on pins 16 (Tx) and 17 (Rx)
/// - Serial3: on pins 14 (Tx) and 15 (Rx)
///
/// On Uno32, there are 2 serial ports:
/// - SerialUSB: this is the port for the USB host connection.
/// - Serial1: on pins 39 (Rx) and 40 (Tx) 
///
/// On Maple and Flymaple, there are 4 serial ports:
/// - SerialUSB: this is the port for the USB host connection.
/// - Serial1: on pins 7 (Tx) and 8 (Rx)
/// - Serial2: on pins 0 (Rx) and 1 (Tx)
/// - Serial3: on pins 29 (Tx) and 30 (Rx)
///
/// Note that it is necessary for you to select which Serial port your RF_Serial will use and pass it to the 
/// contructor.
class RH_Serial : public RHGenericDriver
{
public:
    /// Constructor
    /// \param[in] serial Reference to the HardwareSerial port which will be used by this instance
    RH_Serial(HardwareSerial& serial);

    /// Initialise the Driver transport hardware and software.
    /// Make sure the Driver is properly configured before calling init().
    /// \return true if initialisation succeeded.
    virtual bool init();

    /// Tests whether a new message is available
    /// from the Driver. 
    /// On most drivers, this will also put the Driver into RHModeRx mode until
    /// a message is actually received bythe transport, when it wil be returned to RHModeIdle.
    /// This can be called multiple times in a timeout loop
    /// \return true if a new, complete, error-free uncollected message is available to be retreived by recv()
    virtual bool available();

    /// Turns the receiver on if it not already on.
    /// If there is a valid message available, copy it to buf and return true
    /// else return false.
    /// If a message is copied, *len is set to the length (Caution, 0 length messages are permitted).
    /// You should be sure to call this function frequently enough to not miss any messages
    /// It is recommended that you call it in your main loop.
    /// \param[in] buf Location to copy the received message
    /// \param[in,out] len Pointer to available space in buf. Set to the actual number of octets copied.
    /// \return true if a valid message was copied to buf
    virtual bool recv(uint8_t* buf, uint8_t* len);

    /// Waits until any previous transmit packet is finished being transmitted with waitPacketSent().
    /// Then loads a message into the transmitter and starts the transmitter. Note that a message length
    /// of 0 is NOT permitted. 
    /// \param[in] data Array of data to be sent
    /// \param[in] len Number of bytes of data to send (> 0)
    /// \return true if the message length was valid and it was correctly queued for transmit
    virtual bool send(const uint8_t* data, uint8_t len);

    /// Returns the maximum message length 
    /// available in this Driver.
    /// \return The maximum legal message length
    virtual uint8_t maxMessageLength();


protected:
    /// \brief Defines different receiver states in teh receiver state machine
    typedef enum
    {
	RxStateInitialising = 0,  ///< Before init() is called
	RxStateIdle,              ///< Waiting for an STX
	RxStateDLE,               ///< Waiting for the DLE after STX
	RxStateData,              ///< Receiving data
	RxStateEscape,            ///< Got a DLE while receiving data.
	RxStateWaitFCS1,          ///< Got DLE ETX, waiting for first FCS octet
	RxStateWaitFCS2           ///< Waiting for second FCS octet
    } RxState;

    /// HAndle a character received from the serial port. IMplements
    /// the receiver state machine
    void  handleRx(uint8_t ch);

    /// Empties the Rx buffer
    void  clearRxBuf();

    /// Adds a charater to the Rx buffer
    void  appendRxBuf(uint8_t ch);

    /// Checks whether the Rx buffer contains valid data that is complete and uncorrupted
    /// Check the FCS, the TO address, and extracts the headers
    void  validateRxBuf();

    /// Sends a single data octet to the serial port.
    /// Implements DLE stuffing and keeps track of the senders FCS
    void  txData(uint8_t ch);

    /// Reference to the HardwareSerial port we will use
    HardwareSerial& _serial;

    /// The current state of the Rx state machine
    RxState         _rxState;

    /// Progressive FCS calc (CCITT CRC-16 covering all received data (but not stuffed DLEs), plus trailing DLE, ETX)
    uint16_t        _rxFcs;

    /// The received FCS at the end of the current message
    uint16_t        _rxRecdFcs; 

    /// The Rx buffer
    uint8_t         _rxBuf[RH_SERIAL_MAX_PAYLOAD_LEN];

    /// Current length of data in the Rx buffer
    uint8_t         _rxBufLen;

    /// True if the data in the Rx buffer is value and uncorrupted and complete message is available for collection
    bool            _rxBufValid;

    /// FCS for transmitted data
    uint16_t        _txFcs;
};

/// @example serial_reliable_datagram_client.pde
/// @example serial_reliable_datagram_server.pde

#endif
