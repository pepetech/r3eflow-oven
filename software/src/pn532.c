#include "pn532.h"

static uint8_t ubPn532Command = 0;

uint8_t pn532_init()
{
    PN532_RESET();
    delay_ms(10);
    PN532_UNRESET();
    delay_ms(100);

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
    uint8_t ubCmd = PN532_COMMAND_GETFIRMWAREVERSION;
    if(pn532_writeCommand(&ubCmd, 1, NULL, 0))
        return 0;

    uint8_t pn532_packetbuffer[4];
    if(pn532_readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer), 100) < 0)
        return 0;
    

    uint32_t ulVersion = 0;

    ulVersion = pn532_packetbuffer[0];
    ulVersion <<= 8;
    ulVersion |= pn532_packetbuffer[1];
    ulVersion <<= 8;
    ulVersion |= pn532_packetbuffer[2];
    ulVersion <<= 8;
    ulVersion |= pn532_packetbuffer[3];

    return ulVersion;
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
    uint32_t ullTimeoutStart = g_ullSystemTick;
    while (!pn532_ready()) 
    {
        if(g_ullSystemTick > (ullTimeoutStart + timeout))
            return PN532_TIMEOUT;
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
            break;
        }

        uint8_t ubLength = usart0_spi_transfer_byte(0x00);
        if ((uint8_t)(ubLength + usart0_spi_transfer_byte(0x00)) != 0x00) // checksum of length
        {   
            sResult = PN532_INVALID_FRAME;
            break;
        }

        uint8_t ubCmd = ubPn532Command + 1;               // response command
        if (usart0_spi_transfer_byte(0x00) != PN532_PN532TOHOST || usart0_spi_transfer_byte(0x00) != (ubCmd)) 
        {
            sResult = PN532_INVALID_FRAME;
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

            DBGPRINTLN_CTX("0x%02X", buf[i]);
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

uint8_t pn532_ready()
{
    PN532_SELECT();

    usart0_spi_transfer_byte(PN532_STATUS_READ);
    uint8_t ubStatus = usart0_spi_transfer_byte(0x00) & 1;

    PN532_UNSELECT();

    return ubStatus;
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
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};

    uint8_t ackBuf[sizeof(PN532_ACK)];

    PN532_SELECT();
    delay_ms(2);

    usart0_spi_transfer_byte(PN532_DATA_READ);

    for (uint8_t i = 0; i < sizeof(PN532_ACK); i++) 
    {
        ackBuf[i] = usart0_spi_transfer_byte(0x00);
    }

    PN532_UNSELECT();

    return memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK));
}