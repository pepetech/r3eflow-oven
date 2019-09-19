#include "sk9822.h"

static volatile uint8_t ubLEDBuff[SK9822_BUFF_LEN];
static ldma_descriptor_t __attribute__((aligned (4))) pDMADescriptor[4];

void sk9822_init()
{
    memset((void *)ubLEDBuff, 0x00, SK9822_BUFF_LEN);

    for(uint8_t i = 0; i < SK9822_NUM_LEDS; i++)
        sk9822_set_color(i, 0x00, 0x00, 0x00, 0x00, 0);

    ldma_ch_disable(SK9822_DMA_CHANNEL);
    ldma_ch_peri_req_disable(SK9822_DMA_CHANNEL);
    ldma_ch_req_clear(SK9822_DMA_CHANNEL);

    ldma_ch_config(SK9822_DMA_CHANNEL, LDMA_CH_REQSEL_SOURCESEL_USART0 | LDMA_CH_REQSEL_SIGSEL_USART0TXEMPTY, LDMA_CH_CFG_SRCINCSIGN_POSITIVE, LDMA_CH_CFG_DSTINCSIGN_DEFAULT, LDMA_CH_CFG_ARBSLOTS_DEFAULT, 0);

    pDMADescriptor[0].CTRL = LDMA_CH_CTRL_DSTMODE_ABSOLUTE | LDMA_CH_CTRL_SRCMODE_ABSOLUTE | LDMA_CH_CTRL_DSTINC_NONE | LDMA_CH_CTRL_SIZE_BYTE | LDMA_CH_CTRL_SRCINC_ONE | LDMA_CH_CTRL_REQMODE_BLOCK | LDMA_CH_CTRL_BLOCKSIZE_UNIT1 | ((SK9822_BUFF_LEN << _LDMA_CH_CTRL_XFERCNT_SHIFT) & _LDMA_CH_CTRL_XFERCNT_MASK) | LDMA_CH_CTRL_STRUCTREQ | LDMA_CH_CTRL_STRUCTTYPE_TRANSFER;
    pDMADescriptor[0].SRC = ubLEDBuff;
    pDMADescriptor[0].DST = &(USART0->TXDATA);
    pDMADescriptor[0].LINK = 0x00000000;

    ldma_ch_peri_req_enable(SK9822_DMA_CHANNEL);
    ldma_ch_enable(SK9822_DMA_CHANNEL);
    ldma_ch_load(SK9822_DMA_CHANNEL, pDMADescriptor);
}
void sk9822_set_color(uint16_t usLED, uint8_t ubBrightness, uint8_t ubRed, uint8_t ubGreen, uint8_t ubBlue, uint8_t ubUpdate)
{
    if(usLED >= SK9822_NUM_LEDS)
        return;

    ubLEDBuff[4 + (usLED * 4)] = 0xE0 + (ubBrightness & 0x1F);
    ubLEDBuff[5 + (usLED * 4)] = ubBlue;
    ubLEDBuff[6 + (usLED * 4)] = ubGreen;
    ubLEDBuff[7 + (usLED * 4)] = ubRed;

    if(ubUpdate)
        sk9822_update();
}
void sk9822_update()
{
    while(!(USART0->STATUS & USART_STATUS_TXBL));

    ldma_ch_req_clear(SK9822_DMA_CHANNEL);
    ldma_ch_load(SK9822_DMA_CHANNEL, pDMADescriptor);
}