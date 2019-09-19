#include "vdac.h"

static uint16_t *pusSineWave = NULL;
static ldma_descriptor_t __attribute__ ((aligned (4))) pVDACDMADescriptor[1];

void vdac_init()
{
    CMU->HFPERCLKEN1 |= CMU_HFPERCLKEN1_VDAC0;

    VDAC0->CTRL = VDAC_CTRL_WARMUPMODE_KEEPINSTANDBY | (0x23 << _VDAC_CTRL_PRESC_SHIFT) | VDAC_CTRL_REFSEL_2V5LN;  // VDAC Keep in Stdby, DAC_CLK = HFPERCLK / 36, 2.5V low noise Ref
    VDAC0->CH0CTRL = VDAC_CH0CTRL_TRIGMODE_SW | VDAC_CH0CTRL_CONVMODE_CONTINUOUS; // DACconversion triggered by sw write to data, continuous conversion

    VDAC0->CAL = DEVINFO->VDAC0MAINCAL;

    VDAC0->OPA[0].CTRL = VDAC_OPA_CTRL_OUTSCALE_FULL | (0x3 << _VDAC_OPA_CTRL_DRIVESTRENGTH_SHIFT); // Enable full drive strength, rail-to-rail inputs
    VDAC0->OPA[0].TIMER = (0x000<< _VDAC_OPA_TIMER_SETTLETIME_SHIFT) | (0x05 << _VDAC_OPA_TIMER_WARMUPTIME_SHIFT) | (0x00 << _VDAC_OPA_TIMER_STARTUPDLY_SHIFT); // Recommended settings
    VDAC0->OPA[0].MUX = VDAC_OPA_MUX_RESSEL_RES0 | VDAC_OPA_MUX_GAIN3X | VDAC_OPA_MUX_RESINMUX_VSS | VDAC_OPA_MUX_NEGSEL_OPATAP | VDAC_OPA_MUX_POSSEL_DAC; // Unity gain with DAC as non-inverting input
    VDAC0->OPA[0].OUT = VDAC_OPA_OUT_MAINOUTEN; // Drive the main outp
    VDAC0->OPA[0].CAL = DEVINFO->OPA0CAL7; // Calibration for DRIVESTRENGTH = 0x3, INCBW = 0

    VDAC0->CMD = VDAC_CMD_OPA0EN; // Enable OPA0
    while(!(VDAC0->STATUS & VDAC_STATUS_OPA0ENS)); // Wait for it to be enabled

    VDAC0->CMD = VDAC_CMD_CH0EN;    // Enable VDAC0CH0
    while(!(VDAC0->STATUS & VDAC_STATUS_CH0ENS)); // Wait for it to be enabled
}

void vdac_set_dc(uint16_t usOutput)
{
    ldma_ch_disable(VDAC_DMA_CH);
    ldma_ch_peri_req_disable(VDAC_DMA_CH);
    ldma_ch_req_clear(VDAC_DMA_CH);

    VDAC0->CH0DATA = usOutput;
}
void vdac_set_ac(uint16_t *pWave, uint16_t usFrequency, uint16_t usSamplesPeriod)
{
    CMU->HFPERCLKEN1 |= CMU_HFPERCLKEN1_WTIMER1;
    WTIMER1->CMD |= WTIMER_CMD_STOP;

    ldma_ch_disable(VDAC_DMA_CH);
    ldma_ch_peri_req_disable(VDAC_DMA_CH);
    ldma_ch_req_clear(VDAC_DMA_CH);

    ldma_ch_config(VDAC_DMA_CH, LDMA_CH_REQSEL_SOURCESEL_WTIMER1 | LDMA_CH_REQSEL_SIGSEL_WTIMER1UFOF, LDMA_CH_CFG_SRCINCSIGN_POSITIVE, LDMA_CH_CFG_DSTINCSIGN_DEFAULT, LDMA_CH_CFG_ARBSLOTS_DEFAULT, 0);

    // VDAC DMA descriptor
    pVDACDMADescriptor[0].CTRL = LDMA_CH_CTRL_DSTMODE_ABSOLUTE | LDMA_CH_CTRL_SRCMODE_ABSOLUTE | LDMA_CH_CTRL_DSTINC_NONE | LDMA_CH_CTRL_SIZE_HALFWORD | LDMA_CH_CTRL_SRCINC_ONE | LDMA_CH_CTRL_REQMODE_BLOCK | LDMA_CH_CTRL_BLOCKSIZE_UNIT1 | (((usSamplesPeriod - 1) << _LDMA_CH_CTRL_XFERCNT_SHIFT) & _LDMA_CH_CTRL_XFERCNT_MASK) | LDMA_CH_CTRL_STRUCTTYPE_TRANSFER;
    pVDACDMADescriptor[0].SRC = pWave;
    pVDACDMADescriptor[0].DST = &VDAC0->CH0DATA;
    pVDACDMADescriptor[0].LINK = 0x00000000 | LDMA_CH_LINK_LINK | LDMA_CH_LINK_LINKMODE_RELATIVE;

    // each descriptor is 4 words 1 word = 4 bytes, each descriptor step is +-16 (in this case 2 * 16 = -32 = 0xFFFFFFE0)

    ldma_ch_peri_req_enable(VDAC_DMA_CH);
    ldma_ch_enable(VDAC_DMA_CH);
    ldma_ch_load(VDAC_DMA_CH, pVDACDMADescriptor);

    WTIMER1->CTRL = WTIMER_CTRL_RSSCOIST | WTIMER_CTRL_PRESC_DIV1 | WTIMER_CTRL_CLKSEL_PRESCHFPERCLK | WTIMER_CTRL_FALLA_NONE | WTIMER_CTRL_RISEA_NONE | WTIMER_CTRL_DMACLRACT | WTIMER_CTRL_MODE_UP;
    WTIMER1->TOP = (HFPER_CLOCK_FREQ / (usFrequency * usSamplesPeriod)) - 1;

    WTIMER1->CMD |= WTIMER_CMD_START;
}

void vdac_gen_sine(float fAmplitude, uint16_t usFrequency, uint32_t ulMaxSamplesSecond)
{
    if(pusSineWave)
        free(pusSineWave);

    uint16_t usNSamplesPeriod = MAX(ulMaxSamplesSecond / usFrequency, 64);

    pusSineWave = (uint16_t *)malloc(usNSamplesPeriod * sizeof(uint16_t));

    if(!pusSineWave)
        return;

    fAmplitude = CLIP(0, fAmplitude, 1);

    for(uint16_t usNSample = 0; usNSample < usNSamplesPeriod; usNSample++)
        *(pusSineWave + usNSamplesPeriod - usNSample - 1) = fAmplitude * (2048 + (2047 * cosf(((2.f * 3.14159f) / usNSamplesPeriod) * usNSample)));

    vdac_set_ac(pusSineWave, usFrequency, usNSamplesPeriod);
}