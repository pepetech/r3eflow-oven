#include "pn532.h"

static uint8_t ubPn532Command = 0;

uint8_t pn532_init()
{
    PN532_RESET();
    delay_ms(10);
    PN532_UNRESET();
    delay_ms(100);
    PN532_SELECT();
    delay_ms(2);
    PN532_UNSELECT();

    return 1;
}

void pn532_wakeup()
{
    PN532_SELECT();
    delay_ms(2);
    PN532_UNSELECT();
}

uint32_t pn532_getVersion()
{
    uint8_t pn532_packetbuffer[4];

    pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;
    if(pn532_writeCommand(pn532_packetbuffer, 1, NULL, 0))
        return 0;

    if(pn532_readResponse(pn532_packetbuffer, 4, 10000) < 0)
        return 0;
    
    DBGPRINTLN_CTX("buf 0: 0x%02X", pn532_packetbuffer[0]);
    DBGPRINTLN_CTX("buf 1: 0x%02X", pn532_packetbuffer[1]);
    DBGPRINTLN_CTX("buf 2: 0x%02X", pn532_packetbuffer[2]);
    DBGPRINTLN_CTX("buf 3: 0x%02X", pn532_packetbuffer[3]);

    uint32_t ulVersion = 0;

    ulVersion = pn532_packetbuffer[0];
    ulVersion <<= 8;
    ulVersion |= pn532_packetbuffer[1];
    ulVersion <<= 8;
    ulVersion |= pn532_packetbuffer[2];
    ulVersion <<= 8;
    DBGPRINTLN_CTX("buf 3: 0x%02X", pn532_packetbuffer[3]);
    ulVersion |= pn532_packetbuffer[3];

    return ulVersion;
}

uint8_t pn532_readRegister(uint16_t reg)
{
    uint8_t pn532_packetbuffer[3];
    uint8_t response;

    pn532_packetbuffer[0] = PN532_COMMAND_READREGISTER;
    pn532_packetbuffer[1] = (reg >> 8) & 0xFF;
    pn532_packetbuffer[2] = reg & 0xFF;

    if (pn532_writeCommand(pn532_packetbuffer, 3, NULL, 0))
        return 0;


    // read data packet
    if(pn532_readResponse(pn532_packetbuffer, 1, 10) < 0)
        return 0;

    response = pn532_packetbuffer[0];

    return response;
}

uint32_t pn532_writeRegister(uint16_t reg, uint8_t val)
{
    uint8_t pn532_packetbuffer[4];
    uint32_t response;

    pn532_packetbuffer[0] = PN532_COMMAND_WRITEREGISTER;
    pn532_packetbuffer[1] = (reg >> 8) & 0xFF;
    pn532_packetbuffer[2] = reg & 0xFF;
    pn532_packetbuffer[3] = val;


    if(pn532_writeCommand(pn532_packetbuffer, 4, NULL, 0))
        return 0;

    // read data packet
    if(pn532_readResponse(pn532_packetbuffer, 1, 10) < 0)
        return 0;

    return 1;
}

uint8_t pn532_ready()
{
    PN532_SELECT();
    delay_ms(2);

    usart0_spi_transfer_byte(PN532_STATUS_READ);
    uint8_t ubStatus = usart0_spi_transfer_byte(0x00) & 1;

    PN532_UNSELECT();

    return ubStatus;
}

uint8_t pn532_writeGPIO(uint8_t pinstate)
{
    /*
        Writes an 8-bit value that sets the state of the PN532's GPIO pins
        warning: This function is provided exclusively for board testing and
            is dangerous since it will throw an error if any pin other
            than the ones marked "Can be used as GPIO" are modified!  All
            pins that can not be used as GPIO should ALWAYS be left high
            value = 1) or the system will become unstable and a HW reset
            will be required to recover the PN532.
            pinState[0]  = P30     Can be used as GPIO
            pinState[1]  = P31     Can be used as GPIO
            pinState[2]  = P32     *** RESERVED (Must be 1!) ***
            pinState[3]  = P33     Can be used as GPIO
            pinState[4]  = P34     *** RESERVED (Must be 1!) ***
            pinState[5]  = P35     Can be used as GPIO
    returns 1 if everything executed properly, 0 for an error
    */

    // Make sure pinstate does not try to toggle P32 or P34
    pinstate |= (1 << PN532_GPIO_P32) | (1 << PN532_GPIO_P34);

    uint8_t pn532_packetbuffer[3];

    // Fill command buffer
    pn532_packetbuffer[0] = PN532_COMMAND_WRITEGPIO;
    pn532_packetbuffer[1] = PN532_GPIO_VALIDATIONBIT | pinstate;  // P3 Pins
    pn532_packetbuffer[2] = 0x00;    // P7 GPIO Pins (not used ... taken by I2C)

    DBGPRINTLN_CTX("Writing P3 GPIO: 0x%02X", pn532_packetbuffer[1]);

    // Send the WRITEGPIO command (0x0E)
    if(pn532_writeCommand(pn532_packetbuffer, 3, NULL, 0))
        return 0;

    return (0 < pn532_readResponse(pn532_packetbuffer, 1, 10));
}

uint8_t pn532_readGPIO(void)
{
    uint8_t pn532_packetbuffer[1];

    pn532_packetbuffer[0] = PN532_COMMAND_READGPIO;

    // Send the READGPIO command (0x0C)
    if (pn532_writeCommand(pn532_packetbuffer, 1, NULL, 0))
        return 0x0;

    pn532_readResponse(pn532_packetbuffer, 1, 10);

    /* READGPIO response without prefix and suffix should be in the following format:
      byte            Description
      -------------   ------------------------------------------
      b0              P3 GPIO Pins
      b1              P7 GPIO Pins (not used ... taken by I2C)
      b2              Interface Mode Pins (not used ... bus select pins)
    */


    DBGPRINTLN_CTX("P3 GPIO: 0x%2X", pn532_packetbuffer[7]);
    DBGPRINTLN_CTX("P7 GPIO: 0x%2X", pn532_packetbuffer[8]);
    DBGPRINTLN_CTX("I0I1 GPIO: 0x%2X", pn532_packetbuffer[9]);

    return pn532_packetbuffer[0];
}

int8_t pn532_writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    ubPn532Command = header[0];
    pn532_writeFrame(header, hlen, body, blen);
    
    uint32_t ullTimeoutStart = g_ullSystemTick;
    while(!pn532_ready()) 
    {
        if(g_ullSystemTick > (ullTimeoutStart + PN532_ACK_WAIT_TIME)) 
        {
            DBGPRINTLN_CTX("Time out when waiting for ACK");
            return PN532_TIMEOUT;
        }
    }
    if(pn532_readAckFrame()) 
    {
        DBGPRINTLN_CTX("Invalid ACK");
        return PN532_INVALID_ACK;
    }
    return 0;
}

int16_t pn532_readResponse(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint64_t ullTimeoutStart = g_ullSystemTick;
    while (!pn532_ready()) 
    {
        if(g_ullSystemTick > (ullTimeoutStart + timeout))
        {
            DBGPRINTLN_CTX("Timed out waiting for ready");
            return PN532_TIMEOUT;
        }
    }

    PN532_SELECT();
    delay_ms(2);

    int16_t sResult;
    do {
        usart0_spi_transfer_byte(PN532_DATA_READ);

        if( usart0_spi_transfer_byte(0x00) != 0x00  ||       // PREAMBLE
            usart0_spi_transfer_byte(0x00) != 0x00  ||       // STARTCODE1
            usart0_spi_transfer_byte(0x00) != 0xFF  )        // STARTCODE2
        {
            sResult = PN532_INVALID_FRAME;
            DBGPRINTLN_CTX("PN532_INVALID_FRAME preamble");
            break;
        }

        uint8_t ubLength = usart0_spi_transfer_byte(0x00);
        if ((uint8_t)(ubLength + usart0_spi_transfer_byte(0x00)) != 0x00) // checksum of length
        {   
            sResult = PN532_INVALID_FRAME;
            DBGPRINTLN_CTX("PN532_INVALID_FRAME length checksum");
            break;
        }

        uint8_t ubCmd = ubPn532Command + 1;               // response command
        if (usart0_spi_transfer_byte(0x00) != PN532_PN532TOHOST || usart0_spi_transfer_byte(0x00) != (ubCmd)) 
        {
            sResult = PN532_INVALID_FRAME;
            DBGPRINTLN_CTX("PN532_INVALID_FRAME PN532_PN532TOHOST");
            break;
        }

        DBGPRINTLN_CTX("read: 0x%02X", ubCmd);

        ubLength -= 2;
        if(ubLength > len) 
        {
            for(uint8_t i = 0; i < ubLength; i++) 
            {
                DBGPRINTLN_CTX("0x%02X", usart0_spi_transfer_byte(0x00));    // dump message
            }
            DBGPRINTLN_CTX("Not enough space");
            usart0_spi_transfer_byte(0x00);
            usart0_spi_transfer_byte(0x00);

            sResult = PN532_NO_SPACE;  // not enough space
            break;
        }

        uint8_t ubSum = PN532_PN532TOHOST + ubCmd;
        for(uint8_t i = 0; i < ubLength; i++) 
        {
            buf[i] = usart0_spi_transfer_byte(0x00);
            ubSum += buf[i];

            DBGPRINTLN_CTX("0x%02X -> %hhu", buf[i], i);
        }

        uint8_t ubChecksum = usart0_spi_transfer_byte(0x00);
        if ((uint8_t)(ubSum + ubChecksum) != 0x00) {
            DBGPRINTLN_CTX("checksum nok");
            sResult = PN532_INVALID_FRAME;
            break;
        }
        usart0_spi_transfer_byte(0x00);         // POSTAMBLE

        sResult = ubLength;
    } while (0);

    PN532_UNSELECT();

    return sResult;
}

void pn532_writeFrame(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    PN532_SELECT(); // wake up PN532
    delay_ms(2);

    usart0_spi_transfer_byte(PN532_DATA_WRITE);
    usart0_spi_transfer_byte(PN532_PREAMBLE);
    usart0_spi_transfer_byte(PN532_STARTCODE1);
    usart0_spi_transfer_byte(PN532_STARTCODE2);

    uint8_t ubLength = hlen + blen + 1;   // length of data field: TFI + DATA

    usart0_spi_transfer_byte(ubLength);
    usart0_spi_transfer_byte(~ubLength + 1);         // checksum of length

    usart0_spi_transfer_byte(PN532_HOSTTOPN532);

    uint8_t ubSum = PN532_HOSTTOPN532;    // sum of TFI + DATA

    DBGPRINTLN_CTX("write: ");

    for (uint8_t i = 0; i < hlen; i++) {
        usart0_spi_transfer_byte(header[i]);
        ubSum += header[i];

        DBGPRINTLN_CTX("0x%02X", header[i]);
    }
    for (uint8_t i = 0; i < blen; i++) {
        usart0_spi_transfer_byte(body[i]);
        ubSum += body[i];

        DBGPRINTLN_CTX("0x%02X", body[i]);
    }

    uint8_t ubChecksum = ~ubSum + 1;        // checksum of TFI + DATA
    usart0_spi_transfer_byte(ubChecksum);
    usart0_spi_transfer_byte(PN532_POSTAMBLE);

    PN532_UNSELECT(); 
}


int8_t pn532_readAckFrame()
{
    uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};

    PN532_SELECT();
    delay_ms(2);

    usart0_spi_transfer_byte(PN532_DATA_READ);

    for (uint8_t i = 0; i < sizeof(PN532_ACK); i++) 
    {
        uint8_t data = usart0_spi_transfer_byte(0x00);

        if(data != PN532_ACK[i])
        {
            PN532_UNSELECT();

            return 1;
        }
    }

    PN532_UNSELECT();

    return 0;
}