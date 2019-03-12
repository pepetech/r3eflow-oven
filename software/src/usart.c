#include "usart.h"

#if defined(USART0_MODE_SPI)
void usart0_init(uint32_t ulBaud, uint8_t ubMode, uint8_t ubBitMode, int8_t bMISOLocation, int8_t bMOSILocation, uint8_t ubCLKLocation)
{
    if(bMISOLocation > AFCHANLOC_MAX)
        return;

    if(bMOSILocation > AFCHANLOC_MAX)
        return;

    if(ubCLKLocation > AFCHANLOC_MAX)
        return;

    CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_USART0;

    USART0->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX | UART_CMD_TXTRIDIS | USART_CMD_RXBLOCKDIS | USART_CMD_TXDIS | USART_CMD_RXDIS;

    USART0->CTRL = USART_CTRL_TXBIL_EMPTY | USART_CTRL_CSMA_NOACTION | (ubBitMode == USART_SPI_MSB_FIRST ? USART_CTRL_MSBF : 0) | (ubBitMode & 1 ? USART_CTRL_CLKPHA_SAMPLETRAILING : USART_CTRL_CLKPHA_SAMPLELEADING) | (ubBitMode & 2 ? USART_CTRL_CLKPOL_IDLEHIGH : USART_CTRL_CLKPOL_IDLELOW) | USART_CTRL_SYNC;
    USART0->FRAME = USART_FRAME_DATABITS_EIGHT;
    USART0->CLKDIV = ((HFPER_CLOCK_FREQ / (2 * ulBaud)) - 1) << 8;

    USART0->ROUTEPEN = (bMISOLocation >= 0 ? USART_ROUTEPEN_RXPEN : 0) | (bMOSILocation >= 0 ? USART_ROUTEPEN_TXPEN : 0) | USART_ROUTEPEN_CLKPEN;
    USART0->ROUTELOC0 = ((uint32_t)(bMISOLocation >= 0 ? bMISOLocation : 0) << _USART_ROUTELOC0_RXLOC_SHIFT) | ((uint32_t)(bMOSILocation >= 0 ? bMOSILocation : 0) << _USART_ROUTELOC0_TXLOC_SHIFT) | ((uint32_t)ubCLKLocation << _USART_ROUTELOC0_CLKLOC_SHIFT);

    USART0->CMD = USART_CMD_MASTEREN | USART_CMD_TXEN | USART_CMD_RXEN;
}
uint8_t usart0_spi_transfer_byte(const uint8_t ubData)
{
    while(!(USART0->STATUS & USART_STATUS_TXBL));

    USART0->TXDATA = ubData;

    while(!(USART0->STATUS & USART_STATUS_TXC));

    return USART0->RXDATA;
}
void usart0_spi_transfer(const uint8_t* pubSrc, uint32_t ulSize, uint8_t* pubDst)
{
	if(pubSrc)
	{
		while(ulSize--)
		{
			if(pubDst)
				*(pubDst++) = usart0_spi_transfer_byte(*(pubSrc++));
			else
				usart0_spi_transfer_byte(*(pubSrc++));
		}
	}
	else if(pubDst)
	{
		while(ulSize--)
			*(pubDst++) = usart0_spi_transfer_byte(0x00);
	}
}
#else
static volatile uint8_t *pubUSART0DMABuffer = NULL;
static volatile uint8_t *pubUSART0FIFO = NULL;
static volatile uint16_t usUSART0FIFOWritePos, usUART2FIFOReadPos;

void usart0_init(uint32_t ulBaud, uint32_t ulFrameSettings, int8_t bRXLocation, int8_t bTXLocation, int8_t bCTSLocation, int8_t bRTSLocation)
{

}
void usart0_write_byte(const uint8_t ubData)
{

}
uint8_t usart0_read_byte()
{

}
uint32_t usart0_available()
{

}
void usart0_flush()
{

}
#endif