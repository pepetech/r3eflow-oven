#include <em_device.h>
#include <stdlib.h>
#include <math.h>
#include "debug_macros.h"
#include "utils.h"
#include "nvic.h"
#include "atomic.h"
#include "systick.h"
#include "emu.h"
#include "cmu.h"
#include "gpio.h"
#include "dbg.h"
#include "msc.h"
#include "crypto.h"
#include "crc.h"
#include "trng.h"
#include "rtcc.h"
#include "adc.h"
#include "i2c.h"
#include "mcp9600.h"
#include "pid.h"
#include "pac_lookup.h"

#define ZEROCROSS_DELAY     1400    // us
#define SSR_LATCH_OFFSET    5       // us
#define ZEROCROSS_DEADTIME  300     // us

#define PHASE_ANGLE_WIDTH   10000   // us
#define MAX_PHASE_ANGLE     (PHASE_ANGLE_WIDTH - ZEROCROSS_DEADTIME)
#define MIN_PHASE_ANGLE     (2 * SSR_LATCH_OFFSET)

#define PID_KP  500     // PID Proportional gain
#define PID_KI  50       // PID Integration gain
#define PID_KD  10       // PID Derivative gain

// Structs
static pid_t *pOvenPID = NULL;

// Forward declarations
static void reset() __attribute__((noreturn));
static void sleep();

static uint32_t get_free_ram();

void get_device_name(char *pszDeviceName, uint32_t ulDeviceNameSize);
static uint16_t get_device_revision();

// Variables
// static volatile float fPhaseAngle = 0.f;

// ISRs
void _wtimer0_isr()
{
    uint32_t ulFlags = WTIMER0->IFC;

    if(ulFlags & WTIMER_IF_OF)
    {
        WTIMER1->CC[1].CCV = (PHASE_ANGLE_WIDTH - CLAMP((uint16_t)pOvenPID->fOutput, MIN_PHASE_ANGLE, MAX_PHASE_ANGLE)) / 0.028f;
        WTIMER1->CC[2].CCV = WTIMER1->CC[1].CCV + ((float)SSR_LATCH_OFFSET / 0.028f);
    }
}

// Functions
void reset()
{
    SCB->AIRCR = 0x05FA0000 | _VAL2FLD(SCB_AIRCR_SYSRESETREQ, 1);

    while(1);
}
void sleep()
{
    rtcc_set_alarm(rtcc_get_time() + 5);

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk; // Configure Deep Sleep (EM2/3)

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        IRQ_CLEAR(RTCC_IRQn);

        __DMB(); // Wait for all memory transactions to finish before memory access
        __DSB(); // Wait for all memory transactions to finish before executing instructions
        __ISB(); // Wait for all memory transactions to finish before fetching instructions
        __SEV(); // Set the event flag to ensure the next WFE will be a NOP
        __WFE(); // NOP and clear the event flag
        __WFE(); // Wait for event
        __NOP(); // Prevent debugger crashes

        cmu_init();
        cmu_update_clocks();
    }
}

uint32_t get_free_ram()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        extern void *_sbrk(int);

        void *pCurrentHeap = _sbrk(1);

        if(!pCurrentHeap)
            return 0;

        uint32_t ulFreeRAM = (uint32_t)__get_MSP() - (uint32_t)pCurrentHeap;

        _sbrk(-1);

        return ulFreeRAM;
    }
}

void get_device_name(char *pszDeviceName, uint32_t ulDeviceNameSize)
{
    uint8_t ubFamily = (DEVINFO->PART & _DEVINFO_PART_DEVICE_FAMILY_MASK) >> _DEVINFO_PART_DEVICE_FAMILY_SHIFT;
    const char* szFamily = "?";

    switch(ubFamily)
    {
        case 0x10: szFamily = "EFR32MG1P";  break;
        case 0x11: szFamily = "EFR32MG1B";  break;
        case 0x12: szFamily = "EFR32MG1V";  break;
        case 0x13: szFamily = "EFR32BG1P";  break;
        case 0x14: szFamily = "EFR32BG1B";  break;
        case 0x15: szFamily = "EFR32BG1V";  break;
        case 0x19: szFamily = "EFR32FG1P";  break;
        case 0x1A: szFamily = "EFR32FG1B";  break;
        case 0x1B: szFamily = "EFR32FG1V";  break;
        case 0x1C: szFamily = "EFR32MG12P"; break;
        case 0x1D: szFamily = "EFR32MG12B"; break;
        case 0x1E: szFamily = "EFR32MG12V"; break;
        case 0x1F: szFamily = "EFR32BG12P"; break;
        case 0x20: szFamily = "EFR32BG12B"; break;
        case 0x21: szFamily = "EFR32BG12V"; break;
        case 0x25: szFamily = "EFR32FG12P"; break;
        case 0x26: szFamily = "EFR32FG12B"; break;
        case 0x27: szFamily = "EFR32FG12V"; break;
        case 0x28: szFamily = "EFR32MG13P"; break;
        case 0x29: szFamily = "EFR32MG13B"; break;
        case 0x2A: szFamily = "EFR32MG13V"; break;
        case 0x2B: szFamily = "EFR32BG13P"; break;
        case 0x2C: szFamily = "EFR32BG13B"; break;
        case 0x2D: szFamily = "EFR32BG13V"; break;
        case 0x2E: szFamily = "EFR32ZG13P"; break;
        case 0x31: szFamily = "EFR32FG13P"; break;
        case 0x32: szFamily = "EFR32FG13B"; break;
        case 0x33: szFamily = "EFR32FG13V"; break;
        case 0x34: szFamily = "EFR32MG14P"; break;
        case 0x35: szFamily = "EFR32MG14B"; break;
        case 0x36: szFamily = "EFR32MG14V"; break;
        case 0x37: szFamily = "EFR32BG14P"; break;
        case 0x38: szFamily = "EFR32BG14B"; break;
        case 0x39: szFamily = "EFR32BG14V"; break;
        case 0x3A: szFamily = "EFR32ZG14P"; break;
        case 0x3D: szFamily = "EFR32FG14P"; break;
        case 0x3E: szFamily = "EFR32FG14B"; break;
        case 0x3F: szFamily = "EFR32FG14V"; break;
        case 0x47: szFamily = "EFM32G";     break;
        case 0x48: szFamily = "EFM32GG";    break;
        case 0x49: szFamily = "EFM32TG";    break;
        case 0x4A: szFamily = "EFM32LG";    break;
        case 0x4B: szFamily = "EFM32WG";    break;
        case 0x4C: szFamily = "EFM32ZG";    break;
        case 0x4D: szFamily = "EFM32HG";    break;
        case 0x51: szFamily = "EFM32PG1B";  break;
        case 0x53: szFamily = "EFM32JG1B";  break;
        case 0x55: szFamily = "EFM32PG12B"; break;
        case 0x57: szFamily = "EFM32JG12B"; break;
        case 0x64: szFamily = "EFM32GG11B"; break;
        case 0x67: szFamily = "EFM32TG11B"; break;
        case 0x6A: szFamily = "EFM32GG12B"; break;
        case 0x78: szFamily = "EZR32LG";    break;
        case 0x79: szFamily = "EZR32WG";    break;
        case 0x7A: szFamily = "EZR32HG";    break;
    }

    uint8_t ubPackage = (DEVINFO->MEMINFO & _DEVINFO_MEMINFO_PKGTYPE_MASK) >> _DEVINFO_MEMINFO_PKGTYPE_SHIFT;
    char cPackage = '?';

    if(ubPackage == 74)
        cPackage = '?';
    else if(ubPackage == 76)
        cPackage = 'L';
    else if(ubPackage == 77)
        cPackage = 'M';
    else if(ubPackage == 81)
        cPackage = 'Q';

    uint8_t ubTempGrade = (DEVINFO->MEMINFO & _DEVINFO_MEMINFO_TEMPGRADE_MASK) >> _DEVINFO_MEMINFO_TEMPGRADE_SHIFT;
    char cTempGrade = '?';

    if(ubTempGrade == 0)
        cTempGrade = 'G';
    else if(ubTempGrade == 1)
        cTempGrade = 'I';
    else if(ubTempGrade == 2)
        cTempGrade = '?';
    else if(ubTempGrade == 3)
        cTempGrade = '?';

    uint16_t usPartNumber = (DEVINFO->PART & _DEVINFO_PART_DEVICE_NUMBER_MASK) >> _DEVINFO_PART_DEVICE_NUMBER_SHIFT;
    uint8_t ubPinCount = (DEVINFO->MEMINFO & _DEVINFO_MEMINFO_PINCOUNT_MASK) >> _DEVINFO_MEMINFO_PINCOUNT_SHIFT;

    snprintf(pszDeviceName, ulDeviceNameSize, "%s%huF%hu%c%c%hhu", szFamily, usPartNumber, FLASH_SIZE >> 10, cTempGrade, cPackage, ubPinCount);
}
uint16_t get_device_revision()
{
    uint16_t usRevision;

    /* CHIP MAJOR bit [3:0]. */
    usRevision = ((ROMTABLE->PID0 & _ROMTABLE_PID0_REVMAJOR_MASK) >> _ROMTABLE_PID0_REVMAJOR_SHIFT) << 8;
    /* CHIP MINOR bit [7:4]. */
    usRevision |= ((ROMTABLE->PID2 & _ROMTABLE_PID2_REVMINORMSB_MASK) >> _ROMTABLE_PID2_REVMINORMSB_SHIFT) << 4;
    /* CHIP MINOR bit [3:0]. */
    usRevision |= (ROMTABLE->PID3 & _ROMTABLE_PID3_REVMINORLSB_MASK) >> _ROMTABLE_PID3_REVMINORLSB_SHIFT;

    return usRevision;
}

int init()
{
    emu_init(); // Init EMU

    cmu_hfxo_startup_calib(0x200, 0x0FE); // Config HFXO Startup for 1280 uA, 30.036 pF
    cmu_hfxo_steady_calib(0x00A, 0x0FE); // Config HFXO Steady state for 20.00 uA, 30.036 pF

    cmu_init(); // Init Clocks

    cmu_ushfrco_calib(1, USHFRCO_CALIB_8M, 8000000); // Enable and calibrate USHFRCO for 8 MHz
    cmu_auxhfrco_calib(1, AUXHFRCO_CALIB_32M, 32000000); // Enable and calibrate AUXHFRCO for 32 MHz

    cmu_update_clocks(); // Update Clocks

    dbg_init(); // Init Debug module
    dbg_swo_config(BIT(0) | BIT(1), 2000000); // Init SWO channels 0 and 1 at 2 MHz

    msc_init(); // Init Flash, RAM and caches

    systick_init(); // Init system tick

    gpio_init(); // Init GPIOs
    rtcc_init(); // Init RTCC
    crypto_init(); // Init Crypto engine
    crc_init(); // Init CRC calculation unit
    adc_init(); // Init ADCs

    float fAVDDHighThresh, fAVDDLowThresh;
    float fDVDDHighThresh, fDVDDLowThresh;
    float fIOVDDHighThresh, fIOVDDLowThresh;

    emu_vmon_avdd_config(1, 3.1f, &fAVDDLowThresh, 3.22f, &fAVDDHighThresh); // Enable AVDD monitor
    emu_vmon_dvdd_config(1, 2.5f, &fDVDDLowThresh); // Enable DVDD monitor
    emu_vmon_iovdd_config(1, 3.15f, &fIOVDDLowThresh); // Enable IOVDD monitor

    fDVDDHighThresh = fDVDDLowThresh + 0.026f; // Hysteresis from datasheet
    fIOVDDHighThresh = fIOVDDLowThresh + 0.026f; // Hysteresis from datasheet

    i2c1_init(I2C_NORMAL, 1, 1); // Init I2C1 at 100 kHz on location 1

    char szDeviceName[32];

    get_device_name(szDeviceName, 32);

    DBGPRINTLN_CTX("Device: %s", szDeviceName);
    DBGPRINTLN_CTX("Device Revision: 0x%04X", get_device_revision());
    DBGPRINTLN_CTX("Calibration temperature: %hhu C", (DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK) >> _DEVINFO_CAL_TEMP_SHIFT);
    DBGPRINTLN_CTX("Flash Size: %hu kB", FLASH_SIZE >> 10);
    DBGPRINTLN_CTX("RAM Size: %hu kB", SRAM_SIZE >> 10);
    DBGPRINTLN_CTX("Free RAM: %lu B", get_free_ram());
    DBGPRINTLN_CTX("Unique ID: %08X-%08X", DEVINFO->UNIQUEH, DEVINFO->UNIQUEL);

    DBGPRINTLN_CTX("CMU - HFXO Clock: %.1f MHz!", (float)HFXO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - HFRCO Clock: %.1f MHz!", (float)HFRCO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - USHFRCO Clock: %.1f MHz!", (float)USHFRCO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - AUXHFRCO Clock: %.1f MHz!", (float)AUXHFRCO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - LFXO Clock: %.3f kHz!", (float)LFXO_VALUE / 1000);
    DBGPRINTLN_CTX("CMU - LFRCO Clock: %.3f kHz!", (float)LFRCO_VALUE / 1000);
    DBGPRINTLN_CTX("CMU - ULFRCO Clock: %.3f kHz!", (float)ULFRCO_VALUE / 1000);
    DBGPRINTLN_CTX("CMU - HFSRC Clock: %.1f MHz!", (float)HFSRC_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HF Clock: %.1f MHz!", (float)HF_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFBUS Clock: %.1f MHz!", (float)HFBUS_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFCORE Clock: %.1f MHz!", (float)HFCORE_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFEXP Clock: %.1f MHz!", (float)HFEXP_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFPER Clock: %.1f MHz!", (float)HFPER_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFPERB Clock: %.1f MHz!", (float)HFPERB_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFPERC Clock: %.1f MHz!", (float)HFPERC_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFLE Clock: %.1f MHz!", (float)HFLE_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - ADC0 Clock: %.1f MHz!", (float)ADC0_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - ADC1 Clock: %.1f MHz!", (float)ADC1_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - DBG Clock: %.1f MHz!", (float)DBG_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - AUX Clock: %.1f MHz!", (float)AUX_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - LFA Clock: %.3f kHz!", (float)LFA_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LESENSE Clock: %.3f kHz!", (float)LESENSE_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - RTC Clock: %.3f kHz!", (float)RTC_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LCD Clock: %.3f kHz!", (float)LCD_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LETIMER0 Clock: %.3f kHz!", (float)LETIMER0_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LETIMER1 Clock: %.3f kHz!", (float)LETIMER1_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LFB Clock: %.3f kHz!", (float)LFB_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LEUART0 Clock: %.3f kHz!", (float)LEUART0_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LEUART1 Clock: %.3f kHz!", (float)LEUART1_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - SYSTICK Clock: %.3f kHz!", (float)SYSTICK_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - CSEN Clock: %.3f kHz!", (float)CSEN_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LFC Clock: %.3f kHz!", (float)LFC_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LFE Clock: %.3f kHz!", (float)LFE_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - RTCC Clock: %.3f kHz!", (float)RTCC_CLOCK_FREQ / 1000);

    DBGPRINTLN_CTX("EMU - AVDD Fall Threshold: %.2f mV!", fAVDDLowThresh * 1000);
    DBGPRINTLN_CTX("EMU - AVDD Rise Threshold: %.2f mV!", fAVDDHighThresh * 1000);
    DBGPRINTLN_CTX("EMU - AVDD Voltage: %.2f mV", adc_get_avdd());
    DBGPRINTLN_CTX("EMU - AVDD Status: %s", g_ubAVDDLow ? "LOW" : "OK");
    DBGPRINTLN_CTX("EMU - DVDD Fall Threshold: %.2f mV!", fDVDDLowThresh * 1000);
    DBGPRINTLN_CTX("EMU - DVDD Rise Threshold: %.2f mV!", fDVDDHighThresh * 1000);
    DBGPRINTLN_CTX("EMU - DVDD Voltage: %.2f mV", adc_get_dvdd());
    DBGPRINTLN_CTX("EMU - DVDD Status: %s", g_ubDVDDLow ? "LOW" : "OK");
    DBGPRINTLN_CTX("EMU - IOVDD Fall Threshold: %.2f mV!", fIOVDDLowThresh * 1000);
    DBGPRINTLN_CTX("EMU - IOVDD Rise Threshold: %.2f mV!", fIOVDDHighThresh * 1000);
    DBGPRINTLN_CTX("EMU - IOVDD Voltage: %.2f mV", adc_get_iovdd());
    DBGPRINTLN_CTX("EMU - IOVDD Status: %s", g_ubIOVDDLow ? "LOW" : "OK");
    DBGPRINTLN_CTX("EMU - Core Voltage: %.2f mV", adc_get_corevdd());

    delay_ms(100);

    DBGPRINTLN_CTX("Scanning I2C bus 1...");

    for(uint8_t a = 0x08; a < 0x78; a++)
    {
        if(i2c1_write(a, 0, 0, I2C_STOP))
            DBGPRINTLN_CTX("  Address 0x%02X ACKed!", a);
    }

    if(mcp9600_init())
        DBGPRINTLN_CTX("MCP9600 init OK!");
    else
        DBGPRINTLN_CTX("MCP9600 init NOK!");

    pOvenPID = pid_init(PHASE_ANGLE_WIDTH, 0, PID_KP, PID_KI, PID_KD);

    if(pOvenPID)
        DBGPRINTLN_CTX("Oven PID init OK!");
    else
        DBGPRINTLN_CTX("Oven PID init NOK!");

    return 0;
}
int main()
{
    DBGPRINTLN_CTX("MCP9600 ID 0x%02X Revision 0x%02X", mcp9600_get_id(), mcp9600_get_revision());

    // Internal flash test
    DBGPRINTLN_CTX("Initial calibration dump:");

    for(init_calib_t *psCalibTbl = g_psInitCalibrationTable; psCalibTbl->pulRegister; psCalibTbl++)
        DBGPRINTLN_CTX("  0x%08X -> 0x%08X", psCalibTbl->ulInitialCalibration, psCalibTbl->pulRegister);

    /*
    DBGPRINTLN_CTX("Boot lock word: %08X", g_psLockBits->CLW[0]);
    DBGPRINTLN_CTX("User lock word: %08X", g_psLockBits->ULW);
    DBGPRINTLN_CTX("Mass lock word: %08X", g_psLockBits->MLW);
    msc_flash_word_write((uint32_t)&(g_psLockBits->MLW), 0xFFFFFFFD);
    DBGPRINTLN_CTX("Mass lock word: %08X", g_psLockBits->MLW);

    DBGPRINTLN_CTX("0x000FFFFC: %08X", *(volatile uint32_t *)0x000FFFFC);
    DBGPRINTLN_CTX("0x00100000: %08X", *(volatile uint32_t *)0x00100000);
    msc_flash_word_write(0x000FFFFC, 0x12344321);
    msc_flash_word_write(0x00100000, 0xABCDDCBA);
    DBGPRINTLN_CTX("0x000FFFFC: %08X", *(volatile uint32_t *)0x000FFFFC);
    DBGPRINTLN_CTX("0x00100000: %08X", *(volatile uint32_t *)0x00100000);
    msc_flash_unlock();
    MSC->WRITECMD = MSC_WRITECMD_ERASEMAIN1;
    msc_flash_lock();
    DBGPRINTLN_CTX("0x000FFFFC: %08X", *(volatile uint32_t *)0x000FFFFC);
    DBGPRINTLN_CTX("0x00100000: %08X", *(volatile uint32_t *)0x00100000);
    */

    // MCP9600 init
    mcp9600_set_sensor_config(MCP9600_TYPE_K | MCP9600_FILT_COEF_0);
    mcp9600_set_config(MCP9600_BURST_TS_1 | MCP9600_MODE_NORMAL);

    // PID initialization
    pOvenPID->fSetpoint = 40.f;

    /*
        Function description:

        ZEROCROSS = Rising edge of the zero cross detector (goes high when the wave is about to cross, not after)
        WTIMER0_OF = Goes high ZEROCROSS_DELAY microseconds after ZEROCROSS
        PRS_CH0 = Generates a pulse when WTIMER0_OF goes high
        WTIMER1_CC1 = Goes low ZEROCROSS_DELAY microseconds after ZEROCROSS, goes high when the TRIAC should be turned on
        WTIMER1_CC2 = Goes low ZEROCROSS_DELAY microseconds after ZEROCROSS, goes high when the TRIAC should be turned off
        PRS_CH1 = Follows WTIMER1_CC1 and ANDs it with PRS_CH2
        PRS_CH2 = Follows WTIMER1_CC2 and inverts it
        TRIAC_OUT = TRIAC control signal

        --------------------------

        ZEROCROSS -> Trigger WTIMER0_CC0 -> Start WTIMER0
        WTIMER0_OF -> Pulse PRS_CH0 -> Trigger WTIMER1_CC0 -> Start WTIMER1
        WTIMER1_CC1 -> Level PRS_CH1 -------->
                                            -> AND -> TRIAC_OUT
        WTIMER1_CC2 -> Level PRS_CH2 -> NOT ->
    */

    // Wide Timer 0 - Delay zero cross
    CMU->HFPERCLKEN1 |= CMU_HFPERCLKEN1_WTIMER0;

    WTIMER0->CTRL = WTIMER_CTRL_RSSCOIST | WTIMER_CTRL_PRESC_DIV1 | WTIMER_CTRL_CLKSEL_PRESCHFPERCLK | WTIMER_CTRL_FALLA_NONE | WTIMER_CTRL_RISEA_RELOADSTART | WTIMER_CTRL_OSMEN | WTIMER_CTRL_MODE_UP;
    WTIMER0->TOP = (float)ZEROCROSS_DELAY / 0.028f;
    WTIMER0->CNT = 0x00000000;
    WTIMER0->ROUTELOC0 = ((uint32_t)0 << _WTIMER_ROUTELOC0_CC0LOC_SHIFT);
    WTIMER0->ROUTEPEN |= WTIMER_ROUTEPEN_CC0PEN;

    WTIMER0->CC[0].CTRL = WTIMER_CC_CTRL_FILT_ENABLE | WTIMER_CC_CTRL_INSEL_PIN | WTIMER_CC_CTRL_CUFOA_NONE | WTIMER_CC_CTRL_COFOA_NONE | WTIMER_CC_CTRL_CMOA_NONE | WTIMER_CC_CTRL_MODE_OFF;

    WTIMER0->IFC = _WTIMER_IFC_MASK; // Clear all flags
    IRQ_CLEAR(WTIMER0_IRQn); // Clear pending vector
    IRQ_SET_PRIO(WTIMER0_IRQn, 0, 0); // Set priority 0,0 (max)
    IRQ_ENABLE(WTIMER0_IRQn); // Enable vector
    WTIMER0->IEN = WTIMER_IEN_OF; // Enable OF flag

    // Wide Timer 1 - Output phase angle control
    CMU->HFPERCLKEN1 |= CMU_HFPERCLKEN1_WTIMER1;

    WTIMER1->CTRL = WTIMER_CTRL_RSSCOIST | WTIMER_CTRL_PRESC_DIV1 | WTIMER_CTRL_CLKSEL_PRESCHFPERCLK | WTIMER_CTRL_FALLA_NONE | WTIMER_CTRL_RISEA_RELOADSTART | WTIMER_CTRL_OSMEN | WTIMER_CTRL_MODE_UP;
    WTIMER1->TOP = 0xFFFFFFFF;
    WTIMER1->CNT = 0x00000000;

    WTIMER1->CC[0].CTRL = WTIMER_CC_CTRL_INSEL_PRS | WTIMER_CC_CTRL_PRSSEL_PRSCH0 | WTIMER_CC_CTRL_CUFOA_NONE | WTIMER_CC_CTRL_COFOA_NONE | WTIMER_CC_CTRL_CMOA_NONE | WTIMER_CC_CTRL_MODE_OFF;

    WTIMER1->CC[1].CTRL = WTIMER_CC_CTRL_PRSCONF_LEVEL | WTIMER_CC_CTRL_CUFOA_NONE | WTIMER_CC_CTRL_COFOA_NONE | WTIMER_CC_CTRL_CMOA_SET | WTIMER_CC_CTRL_MODE_OUTPUTCOMPARE;
    WTIMER1->CC[1].CCV = (float)7500 / 0.028f;

    WTIMER1->CC[2].CTRL = WTIMER_CC_CTRL_PRSCONF_LEVEL | WTIMER_CC_CTRL_CUFOA_NONE | WTIMER_CC_CTRL_COFOA_NONE | WTIMER_CC_CTRL_CMOA_SET | WTIMER_CC_CTRL_MODE_OUTPUTCOMPARE;
    WTIMER1->CC[2].CCV = (float)(7500 + SSR_LATCH_OFFSET) / 0.028f;

    // PRS
    CMU->HFBUSCLKEN0 |= CMU_HFBUSCLKEN0_PRS;

    PRS->CH[0].CTRL = PRS_CH_CTRL_SOURCESEL_WTIMER0 | PRS_CH_CTRL_SIGSEL_WTIMER0OF;
    PRS->CH[1].CTRL = PRS_CH_CTRL_ANDNEXT | PRS_CH_CTRL_SOURCESEL_WTIMER1 | PRS_CH_CTRL_SIGSEL_WTIMER1CC1;
    PRS->CH[2].CTRL = PRS_CH_CTRL_INV | PRS_CH_CTRL_SOURCESEL_WTIMER1 | PRS_CH_CTRL_SIGSEL_WTIMER1CC2;

    PRS->ROUTELOC0 |= ((uint32_t)0 << _PRS_ROUTELOC0_CH1LOC_SHIFT); // Output for the SSR
    PRS->ROUTEPEN |= PRS_ROUTEPEN_CH1PEN;

    PRS->ROUTELOC0 |= PRS_ROUTELOC0_CH0LOC_LOC2; // Output Zero Cross for debug purposes
    PRS->ROUTEPEN |= PRS_ROUTEPEN_CH0PEN;

    while(1)
    {
        static uint64_t ullLastBlink = 0;
        static uint64_t ullLastSweep = 0;

        if(g_ullSystemTick > (ullLastBlink + 500))
        {
            GPIO->P[0].DOUT ^= BIT(0);

            ullLastBlink = g_ullSystemTick;
        }

        if(g_ullSystemTick > (ullLastSweep + 500))
        {
            static float on = 0.f;
            static float step = 0.005f;

            if(on >= 1.f)
            {
                on = 1.f;
                step = -step;
            }

            if(on <= 0.f)
            {
                on = 0.f;
                step = -step;
            }

            on += step;

            //fPhaseAngle = CLAMP(on, 0.f, 1.f);

            ullLastSweep = g_ullSystemTick;
        }

        uint8_t ubStatus = mcp9600_get_status();

        if(ubStatus & MCP9600_TH_UPDT)
        {
            static uint64_t ullLastPIDUpdate = 0;

            float fTemp = mcp9600_get_hj_temp();
            float fCold = mcp9600_get_cj_temp();
            float fDelta = mcp9600_get_temp_delta();

            mcp9600_set_status(0x00);
            //mcp9600_set_config(MCP9600_BURST_TS_1 | MCP9600_MODE_NORMAL);

            pOvenPID->fDeltaTime = (float)(g_ullSystemTick - ullLastPIDUpdate) * 0.001f;
            pOvenPID->fValue = fTemp;

            pid_calc(pOvenPID);

            DBGPRINTLN_CTX("PID - Last update: %llu ms ago", g_ullSystemTick - ullLastPIDUpdate);
            DBGPRINTLN_CTX("PID - MCP9600 temp %.3f C", fTemp);
            DBGPRINTLN_CTX("PID - MCP9600 cold %.3f C", fCold);
            DBGPRINTLN_CTX("PID - MCP9600 delta %.3f C", fDelta);
            DBGPRINTLN_CTX("PID - temp target %.3f C", pOvenPID->fSetpoint);
            DBGPRINTLN_CTX("PID - output %.2f / %d", pOvenPID->fOutput, PHASE_ANGLE_WIDTH);
            //DBGPRINTLN_CTX("PID - output linear compensated %d / %d", g_usPacLookup[(uint16_t)pOvenPID->fOutput], PHASE_ANGLE_WIDTH);

            ullLastPIDUpdate = g_ullSystemTick;
        }

        /*
        DBGPRINTLN_CTX("ADC Temp: %.2f", adc_get_temperature());
        DBGPRINTLN_CTX("EMU Temp: %.2f", emu_get_temperature());

        DBGPRINTLN_CTX("HFXO Startup: %.2f pF", cmu_hfxo_get_startup_cap());
        DBGPRINTLN_CTX("HFXO Startup: %.2f uA", cmu_hfxo_get_startup_current());
        DBGPRINTLN_CTX("HFXO Steady: %.2f pF", cmu_hfxo_get_steady_cap());
        DBGPRINTLN_CTX("HFXO Steady: %.2f uA", cmu_hfxo_get_steady_current());
        DBGPRINTLN_CTX("HFXO PMA [%03X]: %.2f uA", cmu_hfxo_get_pma_ibtrim(), cmu_hfxo_get_pma_current());
        DBGPRINTLN_CTX("HFXO PDA [%03X]: %.2f uA", cmu_hfxo_get_pda_ibtrim(1), cmu_hfxo_get_pda_current(0));

        //sleep();

        DBGPRINTLN_CTX("RTCC Time: %lu", rtcc_get_time());
        */
    }

    return 0;
}