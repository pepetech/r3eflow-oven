#include "ldma.h"


void ldma_init()
{
    CMU->HFBUSCLKEN0 |= CMU_HFBUSCLKEN0_LDMA;
}

void ldma_sync_set(uint8_t ubMask)
{
    PERI_REG_BIT_SET(&(LDMA->SYNC)) = ubMask;
}
void ldma_sync_clear(uint8_t ubMask)
{
    PERI_REG_BIT_CLEAR(&(LDMA->SYNC)) = ubMask;
}

void ldma_ch_config(uint8_t ubChannel, uint32_t ulSource, uint32_t ulSrcIncSign, uint32_t ulDstIncSign, uint32_t ulArbitrationSlots, uint8_t ubLoopCount)
{
    if(ubChannel >= DMA_CHAN_COUNT)
        return;

    LDMA->CH[ubChannel].REQSEL = ulSource;
    LDMA->CH[ubChannel].CFG = ulSrcIncSign ;
}
void ldma_ch_load(uint8_t ubChannel, ldma_descriptor_t *pDescriptor)
{
    if(ubChannel >= DMA_CHAN_COUNT)
        return;

    if(!pDescriptor)
        return;

    if((uint32_t)pDescriptor & 3) // Descriptors must be word aligned
        return;

    LDMA->CH[ubChannel].LINK = (uint32_t)pDescriptor;

    LDMA->LINKLOAD = BIT(ubChannel);
}
void ldma_ch_req(uint8_t ubChannel)
{
    if(ubChannel >= DMA_CHAN_COUNT)
        return;

    LDMA->SWREQ = BIT(ubChannel);
}
void ldma_ch_enable(uint8_t ubChannel)
{
    if(ubChannel >= DMA_CHAN_COUNT)
        return;

    PERI_REG_BIT_SET(&(LDMA->CHEN)) = BIT(ubChannel);
}
void ldma_ch_disable(uint8_t ubChannel)
{
    if(ubChannel >= DMA_CHAN_COUNT)
        return;

    PERI_REG_BIT_CLEAR(&(LDMA->CHEN)) = BIT(ubChannel);
}