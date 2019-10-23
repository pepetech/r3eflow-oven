#include <em_device.h>
#include <stdlib.h>
#include <math.h>
#include "debug_macros.h"
#include "utils.h"
#include "nvic.h"
#include "atomic.h"
#include "systick.h"
#include "rmu.h"
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
#include "qspi.h"
#include "usart.h"
#include "i2c.h"
#include "mcp9600.h"
#include "pid.h"
#include "pac_lookup.h"
#include "sk9822.h"
#include "ili9488.h"
#include "ft6x36.h"
#include "lvgl.h"

// Defines
// https://www.silabs.com/documents/public/application-notes/AN0030.pdf

#define ZEROCROSS_DELAY     10    // us
#define SSR_LATCH_OFFSET    10       // us
#define ZEROCROSS_DEADTIME  300     // us

#define PHASE_ANGLE_WIDTH   10000   // us
#define MAX_PHASE_ANGLE     (PHASE_ANGLE_WIDTH - ZEROCROSS_DEADTIME)
#define MIN_PHASE_ANGLE     (2 * SSR_LATCH_OFFSET)

#define PID_OPERATING_RANGE 15 // PID starts at (setpoint -+ this value)
#define PID_KP  350      // PID Proportional gain
#define PID_KI  20       // PID Integration gain
#define PID_KI_CAP  300  // PID Integration gain
#define PID_KD  10        // PID Derivative gain

// Structs
static pid_struct_t *pOvenPID = NULL;

// Forward declarations
static void reset() __attribute__((noreturn));
static void sleep();

static uint32_t get_free_ram();

void get_device_name(char *pszDeviceName, uint32_t ulDeviceNameSize);
static uint16_t get_device_revision();

//lv forwards
void lv_port_disp_init(void);
void lv_port_indev_init(void);

void disp_bl_init(uint32_t ulFrequency);
void disp_bl_set(float fBrightness);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

static void touchpad_init(void);
static bool touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

static void btn_event_cb(lv_obj_t * btn, lv_event_t event);
static void ddlist_event_cb(lv_obj_t * ddlist, lv_event_t event);

// Variables
lv_indev_t * indev_touchpad;

static lv_obj_t * slider;

// ISRs
void _wtimer0_isr()
{
    uint32_t ulFlags = WTIMER0->IFC;

    if(ulFlags & WTIMER_IF_OF)
    {
        WTIMER1->CC[1].CCV = (PHASE_ANGLE_WIDTH - CLIP(g_usPacLookup[(uint16_t)pOvenPID->fOutput], MIN_PHASE_ANGLE, MAX_PHASE_ANGLE)) / 0.028f;
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
    void *pCurrentHeap = malloc(1);

    uint32_t ulFreeRAM = (uint32_t)__get_MSP() - (uint32_t)pCurrentHeap;

    free(pCurrentHeap);

    return ulFreeRAM;
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
    rmu_init(RMU_CTRL_PINRMODE_FULL, RMU_CTRL_SYSRMODE_EXTENDED, RMU_CTRL_LOCKUPRMODE_EXTENDED, RMU_CTRL_WDOGRMODE_EXTENDED); // Init RMU and set reset modes

    emu_init(1); // Init EMU
    emu_r5v_vin_config(EMU_R5VCTRL_INPUTMODE_AUTO); // Set 5V regulator automatic input selection
    emu_r5v_vout_config(3.3f); // Set 5V regulator output voltage to 3.3V
    //emu_dcdc_init(2500.f, 200.f, 500.f, 0.f); // Init DC-DC converter (2.5 V, 200 mA active, 500 uA sleep, 0 mA reverse limit)

    cmu_hfxo_startup_calib(0x200, 0x087); // Config HFXO Startup for 1280 uA, 20.04 pF
    cmu_hfxo_steady_calib(0x006, 0x087); // Config HFXO Steady state for 12 uA, 20.04 pF

    cmu_init(); // Init Clocks

    cmu_ushfrco_calib(1, USHFRCO_CALIB_8M, 8000000); // Enable and calibrate USHFRCO for 8 MHz
    cmu_auxhfrco_calib(1, AUXHFRCO_CALIB_32M, 32000000); // Enable and calibrate AUXHFRCO for 32 MHz

    cmu_update_clocks(); // Update Clocks

    dbg_init(); // Init Debug module
    dbg_swo_config(BIT(0) | BIT(1), 6000000); // Init SWO channels 0 and 1 at 6 MHz

    msc_init(); // Init Flash, RAM and caches

    systick_init(); // Init system tick

    gpio_init(); // Init GPIOs
    ldma_init(); // Init LDMA
    rtcc_init(); // Init RTCC
    trng_init(); // Init TRNG
    crypto_init(); // Init Crypto engine
    crc_init(); // Init CRC calculation unit
    adc_init(); // Init ADCs
    qspi_init(); // Init QSPI memory

    float fAVDDHighThresh, fAVDDLowThresh;
    float fDVDDHighThresh, fDVDDLowThresh;
    float fIOVDDHighThresh, fIOVDDLowThresh;

    emu_vmon_avdd_config(1, 3.1f, &fAVDDLowThresh, 3.22f, &fAVDDHighThresh); // Enable AVDD monitor
    emu_vmon_dvdd_config(1, 2.5f, &fDVDDLowThresh); // Enable DVDD monitor
    emu_vmon_iovdd_config(1, 3.15f, &fIOVDDLowThresh); // Enable IOVDD monitor

    fDVDDHighThresh = fDVDDLowThresh + 0.026f; // Hysteresis from datasheet
    fIOVDDHighThresh = fIOVDDLowThresh + 0.026f; // Hysteresis from datasheet

    i2c0_init(I2C_NORMAL, 1, 1); // Init I2C0 at 100 kHz on location 1 Temp Sensors
    i2c1_init(I2C_NORMAL, 1, 1); // Init I2C1 at 400 kHz on location 1 Touch TFT
    usart0_init(18000000, 0, USART_SPI_MSB_FIRST, -1, 1, 1); // Init Usart 0 Mode SPI at 1MHz SK9822 LED
    usart2_init(36000000, 0, USART_SPI_MSB_FIRST, 0, 0, 0); // Init Usart 2 Mode SPI at 36MHz ILI9488 Display

    //usart0_init(115200, UART_FRAME_STOPBITS_ONE | UART_FRAME_PARITY_NONE | USART_FRAME_DATABITS_EIGHT, 4, 4, -1, -1);
    //usart0_init(1000000, 0, USART_SPI_LSB_FIRST, 0, 0, 0);
    //usart0_init(800000, 1, USART_SPI_MSB_FIRST, -1, 4, 5);
    //usart0_init(1000000, 1, USART_SPI_MSB_FIRST, 0, 0, 0);
    //i2c0_init(I2C_NORMAL, 1, 1); // Init I2C0 at 100 kHz on location 1

    char szDeviceName[32];

    get_device_name(szDeviceName, 32);

    DBGPRINTLN_CTX("Device: %s", szDeviceName);
    DBGPRINTLN_CTX("Device Revision: 0x%04X", get_device_revision());
    DBGPRINTLN_CTX("Calibration temperature: %hhu C", (DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK) >> _DEVINFO_CAL_TEMP_SHIFT);
    DBGPRINTLN_CTX("Flash Size: %hu kB", FLASH_SIZE >> 10);
    DBGPRINTLN_CTX("RAM Size: %hu kB", SRAM_SIZE >> 10);
    DBGPRINTLN_CTX("Free RAM: %lu B", get_free_ram());
    DBGPRINTLN_CTX("Unique ID: %08X-%08X", DEVINFO->UNIQUEH, DEVINFO->UNIQUEL);

    DBGPRINTLN_CTX("RMU - Reset cause: %hhu", rmu_get_reset_reason());
    DBGPRINTLN_CTX("RMU - Reset state: %hhu", rmu_get_reset_state());

    rmu_clear_reset_reason();

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
    DBGPRINTLN_CTX("CMU - QSPI Clock: %.1f MHz!", (float)QSPI_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - SDIO Clock: %.1f MHz!", (float)SDIO_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - USB Clock: %.1f MHz!", (float)USB_CLOCK_FREQ / 1000000);
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
    DBGPRINTLN_CTX("EMU - R5V VREGI Voltage: %.2f mV", adc_get_r5v_vregi());
    DBGPRINTLN_CTX("EMU - R5V VREGI Current: %.2f mA", adc_get_r5v_vregi_current());
    DBGPRINTLN_CTX("EMU - R5V VBUS Voltage: %.2f mV", adc_get_r5v_vbus());
    DBGPRINTLN_CTX("EMU - R5V VBUS Current: %.2f mA", adc_get_r5v_vbus_current());
    DBGPRINTLN_CTX("EMU - R5V VREGO Voltage: %.2f mV", adc_get_r5v_vrego());

    play_sound(1500, 500);
    delay_ms(100);


    DBGPRINTLN_CTX("Scanning I2C bus 0...");

    for(uint8_t a = 0x08; a < 0x78; a++)
    {
        if(i2c0_write(a, 0, 0, I2C_STOP))
            DBGPRINTLN_CTX("  Address 0x%02X ACKed!", a);
    }

    DBGPRINTLN_CTX("Scanning I2C bus 1...");

    for(uint8_t a = 0x08; a < 0x78; a++)
    {
        if(i2c1_write(a, 0, 0, I2C_STOP))
            DBGPRINTLN_CTX("  Address 0x%02X ACKed!", a);
    }

    if(mcp9600_init(0))
        DBGPRINTLN_CTX("MCP9600 #0 init OK!");
    else
        DBGPRINTLN_CTX("MCP9600 #0 init NOK!");

    if(mcp9600_init(7))
        DBGPRINTLN_CTX("MCP9600 #7 init OK!");
    else
        DBGPRINTLN_CTX("MCP9600 #7 init NOK!");

    if(ili9488_init())
        DBGPRINTLN_CTX("ILI9488 init OK!");
    else
        DBGPRINTLN_CTX("ILI9488 init NOK!");

    if(ft6x36_init())
        DBGPRINTLN_CTX("FT6236 init OK!");
    else
        DBGPRINTLN_CTX("FT6236 init NOK!");

    pOvenPID = pid_init(PHASE_ANGLE_WIDTH, 0, PID_OPERATING_RANGE, PID_KI_CAP, PID_KP, PID_KI, PID_KD);

    if(pOvenPID)
        DBGPRINTLN_CTX("Oven PID init OK!");
    else
        DBGPRINTLN_CTX("Oven PID init NOK!");

    return 0;
}
int main()
{
    play_sound(2000, 100);
    delay_ms(50);
    play_sound(2000, 100);

    // Internal flash test
    //DBGPRINTLN_CTX("Initial calibration dump:");

    //for(init_calib_t *psCalibTbl = g_psInitCalibrationTable; psCalibTbl->pulRegister; psCalibTbl++)
    //    DBGPRINTLN_CTX("  0x%08X -> 0x%08X", psCalibTbl->ulInitialCalibration, psCalibTbl->pulRegister);

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

    // QSPI
    DBGPRINTLN_CTX("Flash Part ID: %06X", qspi_flash_read_jedec_id());

    uint8_t ubFlashUID[8];

    qspi_flash_read_security(0x0000, ubFlashUID, 8);

    DBGPRINTLN_CTX("Flash ID: %02X%02X%02X%02X%02X%02X%02X%02X", ubFlashUID[0], ubFlashUID[1], ubFlashUID[2], ubFlashUID[3], ubFlashUID[4], ubFlashUID[5], ubFlashUID[6], ubFlashUID[7]);

    //qspi_flash_chip_erase();

    //uint8_t rd[16];

    //qspi_flash_cmd(QSPI_FLASH_CMD_READ_FAST, 0x00008000, 3, 0, 8, NULL, 0, rd, 10);
    //DBGPRINTLN_CTX("Flash RD C: %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X", rd[0], rd[1], rd[2], rd[3], rd[4], rd[5], rd[6], rd[7], rd[8], rd[9], rd[10], rd[11], rd[12], rd[13], rd[14], rd[15]);

    //DBGPRINTLN_CTX("Flash RD: %08X", *(volatile uint32_t *)0xC0000000);
    //*(volatile uint32_t *)0xC0000000 = 0xABCDEF12;

    //qspi_flash_cmd(QSPI_FLASH_CMD_READ_FAST, 0x00000000, 3, 0, 8, NULL, 0, rd, 10);
    //DBGPRINTLN_CTX("Flash RD C: %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X", rd[0], rd[1], rd[2], rd[3], rd[4], rd[5], rd[6], rd[7], rd[8], rd[9], rd[10], rd[11], rd[12], rd[13], rd[14], rd[15]);

    //uint32_t wr = 0xA2B3C4D5;
    //qspi_flash_busy_wait();
    //qspi_flash_write_enable();
    //qspi_flash_cmd(QSPI_FLASH_CMD_WRITE, 0x00000004, 3, 0, 0, (uint8_t*)&wr, 4, NULL, 0);
    //qspi_flash_busy_wait();

    //qspi_flash_cmd(QSPI_FLASH_CMD_READ_FAST, 0x00000000, 3, 0, 8, NULL, 0, rd, 10);
    //DBGPRINTLN_CTX("Flash RD C: %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X", rd[0], rd[1], rd[2], rd[3], rd[4], rd[5], rd[6], rd[7], rd[8], rd[9], rd[10], rd[11], rd[12], rd[13], rd[14], rd[15]);

    //*(volatile uint32_t *)0xC0000000 = 0x12AB34CD;

    //////// Test for page wrapping (write beyond page boundary)

    /*
    for(uint8_t i = 0; i <= 64; i++)
        *(volatile uint32_t *)(0xC0000000 + i * 4) = 0x0123ABCD;

    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC0000000); // CD
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC0000001); // AB
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC0000002); // 23
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC0000003); // 01
    DBGPRINTLN_CTX("Flash RD: %08X", *(volatile uint32_t *)0xC0000000); // 0123ABCD
    DBGPRINTLN_CTX("Flash RD: %08X", *(volatile uint32_t *)0xC0000010); // 0123ABCD
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC00000FC); // CD
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC00000FD); // AB
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC00000FE); // 23
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC00000FF); // 01
    DBGPRINTLN_CTX("Flash RD: %08X", *(volatile uint32_t *)0xC0000100); // 0123ABCD
    */

    //////// Test for code copy to QSPI flash

    /*
    for(uint32_t i = 0; i < bin_v1_test_bin_qspi_len / 4; i++)
        *(volatile uint32_t *)(0x04000000 + i * 4) = *(uint32_t *)(bin_v1_test_bin_qspi + i * 4);

    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000000);
    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000001);
    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000002);
    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000003);

    DBGPRINTLN_CTX("QSPI Dest %08X", get_family_name);
    DBGPRINTLN_CTX("Device: %s%hu", get_family_name((DEVINFO->PART & _DEVINFO_PART_DEVICE_FAMILY_MASK) >> _DEVINFO_PART_DEVICE_FAMILY_SHIFT), (DEVINFO->PART & _DEVINFO_PART_DEVICE_NUMBER_MASK) >> _DEVINFO_PART_DEVICE_NUMBER_SHIFT);
    */

    //DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000000);
    //DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000001);
    //DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000002);
    //DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000003);
    //DBGPRINTLN_CTX("Boot RD: %02X", *(volatile uint8_t *)0x0FE10000);
    //DBGPRINTLN_CTX("Data RD: %02X", *(volatile uint8_t *)0x0FE00000);

    //qspi_flash_cmd(QSPI_FLASH_CMD_READ_FAST, 0x00000000, 3, 0, 8, NULL, 0, rd, 10);
    //DBGPRINTLN_CTX("Flash RD C: %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X", rd[0], rd[1], rd[2], rd[3], rd[4], rd[5], rd[6], rd[7], rd[8], rd[9], rd[10], rd[11], rd[12], rd[13], rd[14], rd[15]);

    //DBGPRINTLN_CTX("QSPI RD: %08X", *(volatile uint32_t *)0xC0000000);
    //DBGPRINTLN_CTX("QSPI RD: %08X", *(volatile uint32_t *)0xC0000004);

    //for(uint8_t i = 0; i < 112; i++)
    //{
    //    usart0_spi_transfer_byte(0xFF); // G
    //    usart0_spi_transfer_byte(0xFF); // R
    //    usart0_spi_transfer_byte(0xFF); // B
    //}

    //delay_ms(1000);

    //for(uint8_t i = 0; i < 112; i++)
    //{
    //    usart0_spi_transfer_byte(0x00); // G
    //    usart0_spi_transfer_byte(0x00); // R
    //    usart0_spi_transfer_byte(0x00); // B
    //}

    //delay_ms(1000);

    // LEDs init
    sk9822_init();
    DBGPRINTLN_CTX("SK9822 LEDs Init!");

    // tft + LvGL init
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    // MCP9600 ID
    DBGPRINTLN_CTX("MCP9600 #0 ID 0x%02X Revision 0x%02X", mcp9600_get_id(0), mcp9600_get_revision(0));
    DBGPRINTLN_CTX("MCP9600 #7 ID 0x%02X Revision 0x%02X", mcp9600_get_id(7), mcp9600_get_revision(7));

    // MCP9600 init
    mcp9600_set_sensor_config(0, MCP9600_TYPE_K | MCP9600_FILT_COEF_0);
    mcp9600_set_config(0, MCP9600_BURST_TS_1 | MCP9600_MODE_NORMAL);

    mcp9600_set_sensor_config(7, MCP9600_TYPE_K | MCP9600_FILT_COEF_0);
    mcp9600_set_config(7, MCP9600_BURST_TS_1 | MCP9600_MODE_NORMAL);

    // PID initialization
    pOvenPID->fSetpoint = 160.f;

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

    PRS->CH[11].CTRL = PRS_CH_CTRL_ANDNEXT | PRS_CH_CTRL_SOURCESEL_WTIMER1 | PRS_CH_CTRL_SIGSEL_WTIMER1CC1;
    PRS->CH[12].CTRL = PRS_CH_CTRL_INV | PRS_CH_CTRL_SOURCESEL_WTIMER1 | PRS_CH_CTRL_SIGSEL_WTIMER1CC2;

    PRS->ROUTELOC2 |= ((uint32_t)2 << _PRS_ROUTELOC2_CH11LOC_SHIFT); // Output for the SSR
    PRS->ROUTEPEN |= PRS_ROUTEPEN_CH11PEN;

    //PRS->ROUTELOC0 |= PRS_ROUTELOC0_CH0LOC_LOC2; // Output Zero Cross for debug purposes
    //PRS->ROUTEPEN |= PRS_ROUTEPEN_CH0PEN;

    while(1)
    {
        static uint64_t ullLastBlink = 0;
        static uint64_t ullLastInput = 0;
        static uint64_t ullLastPIDUpdate = 0;
        static uint64_t ullLastTempCheck = 0;
        static uint64_t ullLastStateUpdate = 0;
        static uint64_t ullLastLvGLUpdate = 0;

        float fTemp;

        lv_task_handler();

        static uint8_t run_once = 1;

        if(run_once)
        {
            lv_theme_t * th = lv_theme_material_init(0, NULL);
            lv_theme_set_current(th);
            /********************
             * CREATE A SCREEN
             *******************/
            /* Create a new screen and load it
            * Screen can be created from any type object type
            * Now a Page is used which is an objects with scrollable content*/
            lv_obj_t * scr = lv_page_create(NULL, NULL);
            lv_disp_load_scr(scr);

            /****************
             * ADD A TITLE
             ****************/
            lv_obj_t * label = lv_label_create(scr, NULL); /*First parameters (scr) is the parent*/
            lv_label_set_text(label, "Object usage demo");  /*Set the text*/
            lv_obj_set_x(label, 50);                        /*Set the x coordinate*/

            /***********************
             * CREATE TWO BUTTONS
             ***********************/
            /*Create a button*/
            lv_obj_t * btn1 = lv_btn_create(lv_disp_get_scr_act(NULL), NULL);         /*Create a button on the currently loaded screen*/
            lv_obj_set_event_cb(btn1, btn_event_cb);                                  /*Set function to be called when the button is released*/
            lv_obj_align(btn1, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);               /*Align below the label*/

            /*Create a label on the button (the 'label' variable can be reused)*/
            label = lv_label_create(btn1, NULL);
            lv_label_set_text(label, "Button 1");

            /*Copy the previous button*/
            lv_obj_t * btn2 = lv_btn_create(scr, btn1);                 /*Second parameter is an object to copy*/
            lv_obj_align(btn2, btn1, LV_ALIGN_OUT_RIGHT_MID, 50, 0);    /*Align next to the prev. button.*/

            /*Create a label on the button*/
            label = lv_label_create(btn2, NULL);
            lv_label_set_text(label, "Button 2");

            /****************
             * ADD A SLIDER
             ****************/
            slider = lv_slider_create(scr, NULL);                            /*Create a slider*/
            lv_obj_set_size(slider, lv_obj_get_width(scr)  / 3, LV_DPI / 3);            /*Set the size*/
            lv_obj_align(slider, btn1, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);                /*Align below the first button*/
            lv_slider_set_value(slider, 30, false);                                            /*Set the current value*/

            /***********************
             * ADD A DROP DOWN LIST
             ************************/
            lv_obj_t * ddlist = lv_ddlist_create(scr, NULL);                     /*Create a drop down list*/
            lv_obj_align(ddlist, slider, LV_ALIGN_OUT_RIGHT_TOP, 50, 0);         /*Align next to the slider*/
            lv_obj_set_top(ddlist, true);                                        /*Enable to be on the top when clicked*/
            lv_ddlist_set_options(ddlist, "None\nLittle\nHalf\nA lot\nAll");     /*Set the options*/
            lv_obj_set_event_cb(ddlist, ddlist_event_cb);                        /*Set function to call on new option is chosen*/

            /****************
             * CREATE A CHART
             ****************/
            lv_obj_t * chart = lv_chart_create(scr, NULL);                         /*Create the chart*/
            lv_obj_set_size(chart, lv_obj_get_width(scr) / 2, lv_obj_get_width(scr) / 4);   /*Set the size*/
            lv_obj_align(chart, slider, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 50);                   /*Align below the slider*/
            lv_chart_set_series_width(chart, 3);                                            /*Set the line width*/

            /*Add a RED data series and set some points*/
            lv_chart_series_t * dl1 = lv_chart_add_series(chart, LV_COLOR_RED);
            lv_chart_set_next(chart, dl1, 10);
            lv_chart_set_next(chart, dl1, 25);
            lv_chart_set_next(chart, dl1, 45);
            lv_chart_set_next(chart, dl1, 80);

            /*Add a BLUE data series and set some points*/
            lv_chart_series_t * dl2 = lv_chart_add_series(chart, lv_color_make(0x40, 0x70, 0xC0));
            lv_chart_set_next(chart, dl2, 10);
            lv_chart_set_next(chart, dl2, 25);
            lv_chart_set_next(chart, dl2, 45);
            lv_chart_set_next(chart, dl2, 80);
            lv_chart_set_next(chart, dl2, 75);
            lv_chart_set_next(chart, dl2, 505);

            DBGPRINTLN_CTX("Free RAM: %lu B", get_free_ram());

            run_once = 0;
        }

        if(g_ullSystemTick > (ullLastBlink + 50))
        {
            static uint8_t ubLastLED = 7;

            sk9822_set_color(ubLastLED, 0x00, 0x00, 0x00, 0x00, 0);

            if(ubLastLED == 7)
                ubLastLED = 0;
            else
                ubLastLED += 1;

            uint32_t ulColor = trng_pop_random();

            sk9822_set_color(ubLastLED, 1, (uint8_t)((ulColor >> 16) & 0xFF), (uint8_t)((ulColor >> 8) & 0xFF), (uint8_t)(ulColor & 0xFF), 1);

            ullLastBlink = g_ullSystemTick;
        }

//        if(GPIO->P[0].DIN & BIT(2) && g_ullSystemTick > (ullLastInput + 500))
//        {
//            pOvenPID->fSetpoint += 2.f;
//
//            ullLastInput = g_ullSystemTick;
//        }

        if(g_ullSystemTick > (ullLastTempCheck + 100))
        {
            uint8_t ubStatus = mcp9600_get_status(0);

            if(ubStatus & MCP9600_TH_UPDT)
            {
                fTemp = mcp9600_get_hj_temp(0);
                //float fCold = mcp9600_get_cj_temp(MCP9600_0);
                //float fDelta = mcp9600_get_temp_delta(MCP9600_0);

                mcp9600_set_status(0, 0x00);
                //mcp9600_set_config(MCP9600_BURST_TS_1 | MCP9600_MODE_NORMAL);

                pOvenPID->fDeltaTime = (float)(g_ullSystemTick - ullLastPIDUpdate) * 0.001f;
                pOvenPID->fValue = fTemp;

                pid_calc(pOvenPID);

                DBGPRINTLN_CTX("PID - Last update: %llu ms ago", g_ullSystemTick - ullLastPIDUpdate);
                DBGPRINTLN_CTX("PID - MCP9600 temp %.3f C", fTemp);
                //DBGPRINTLN_CTX("PID - MCP9600 cold %.3f C", fCold);
                //DBGPRINTLN_CTX("PID - MCP9600 delta %.3f C", fDelta);
                DBGPRINTLN_CTX("PID - temp target %.3f C", pOvenPID->fSetpoint);
                DBGPRINTLN_CTX("PID - integral %.3f", pOvenPID->fIntegral);
                DBGPRINTLN_CTX("PID - output %.2f / %d", pOvenPID->fOutput, PHASE_ANGLE_WIDTH);
                //DBGPRINTLN_CTX("PID - output linear compensated %d / %d", g_usPacLookup[(uint16_t)pOvenPID->fOutput], PHASE_ANGLE_WIDTH);

                ullLastPIDUpdate = g_ullSystemTick;
            }

            ullLastTempCheck = g_ullSystemTick;
        }

        if(g_ullSystemTick > (ullLastStateUpdate + 500))
        {
            static uint64_t ullTimer = 0;
            static uint8_t ubState = 0;

            switch(ubState)
            {
                case 0:     // preheat
                    DBGPRINTLN_CTX("State - preheat");
                    DBGPRINTLN_CTX("State - progress - %.3f C / 160 C", fTemp);
                    pOvenPID->fSetpoint = 145;
                    if(fTemp > 145)
                    {
                        ubState = 1;
                        ullTimer = g_ullSystemTick;
                    }
                    break;

                case 1:     // soak
                    DBGPRINTLN_CTX("State - soak");
                    DBGPRINTLN_CTX("State - progress - %lu ms left", (ullTimer + 70000) - g_ullSystemTick);
                    pOvenPID->fSetpoint = 160;
                    if(g_ullSystemTick > (ullTimer + 70000))
                    {
                        ubState = 2;
                        ullTimer = g_ullSystemTick;
                    }

                    break;

                case 2:     // reflow
                    DBGPRINTLN_CTX("State - reflow");
                    DBGPRINTLN_CTX("State - progress - %.3f C / 220 C", fTemp);
                    pOvenPID->fSetpoint = 240;
                    if(fTemp > 220)
                    {
                        ubState = 3;
                        ullTimer = g_ullSystemTick;
                    }
                    break;

                case 3:     // cool
                    DBGPRINTLN_CTX("State - cool");
                    pOvenPID->fSetpoint = 0;
                    break;

                default:
                    DBGPRINTLN_CTX("State - default");
                    pOvenPID->fSetpoint = 0;
                    break;
            }

            ullLastStateUpdate = g_ullSystemTick;
        }

        /*
        DBGPRINTLN_CTX("ADC Temp: %.2f", adc_get_temperature());
        DBGPRINTLN_CTX("EMU Temp: %.2f", emu_get_temperature());

        DBGPRINTLN_CTX("HFXO Startup: %.2f pF", cmu_hfxo_get_startup_cap());
        DBGPRINTLN_CTX("HFXO Startup: %.2f uA", cmu_hfxo_get_startup_current());
        DBGPRINTLN_CTX("HFXO Steady: %.2f pF", cmu_hfxo_get_steady_cap());
        DBGPRINTLN_CTX("HFXO Steady: %.2f uA", cmu_hfxo_get_steady_current());
        DBGPRINTLN_CTX("HFXO PMA [%03X]: %.2f uA", cmu_hfxo_get_pma_ibtrim(), cmu_hfxo_get_pma_current());
        DBGPRINTLN_CTX("HFXO PDA [resistor%03X]: %.2f uA", cmu_hfxo_get_pda_ibtrim(1), cmu_hfxo_get_pda_current(0));

        //sleep();

        DBGPRINTLN_CTX("RTCC Time: %lu", rtcc_get_time());
        */
    }

    return 0;
}

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_bl_init(2000);
    disp_bl_set(0.5);

    ili9488_init();

    ili9488_set_rotation(1);

    ili9488_display_on();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /* LittlevGL requires a buffer where it draws the objects. The buffer's has to be greater than 1 display row
     *
     * There are three buffering configurations:
     * 1. Create ONE buffer with some rows: 
     *      LittlevGL will draw the display's content here and writes it to your display
     * 
     * 2. Create TWO buffer with some rows: 
     *      LittlevGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LittlevGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     * 
     * 3. Create TWO screen-sized buffer: 
     *      Similar to 2) but the buffer have to be screen sized. When LittlevGL is ready it will give the
     *      whole frame to display. This way you only need to change the frame buffer's address instead of
     *      copying the pixels.
     * */

    /* Example for 1) */
    //static lv_disp_buf_t disp_buf_1;
    //static lv_color_t buf1_1[LV_HOR_RES_MAX * 10];                      /*A buffer for 10 rows*/
    //lv_disp_buf_init(&disp_buf_1, buf1_1, NULL, LV_HOR_RES_MAX * 10);   /*Initialize the display buffer*/

    /* Example for 2) */
    static lv_disp_buf_t disp_buf_2;
    static lv_color_t buf2_1[LV_HOR_RES_MAX * 60];                        /*A buffer for 10 rows*/
    static lv_color_t buf2_2[LV_HOR_RES_MAX * 60];                        /*An other buffer for 10 rows*/
    lv_disp_buf_init(&disp_buf_2, buf2_1, buf2_2, LV_HOR_RES_MAX * 10);   /*Initialize the display buffer*/

    /* Example for 3) */
    //static lv_disp_buf_t disp_buf_3;
    //static lv_color_t buf3_1[LV_HOR_RES_MAX * LV_VER_RES_MAX];            /*A screen sized buffer*/
    //static lv_color_t buf3_2[LV_HOR_RES_MAX * LV_VER_RES_MAX];            /*An other screen sized buffer*/
    //lv_disp_buf_init(&disp_buf_3, buf3_1, buf3_2, LV_HOR_RES_MAX * LV_VER_RES_MAX);   /*Initialize the display buffer*/


    /*-----------------------------------
     * Register the display in LittlevGL
     *----------------------------------*/

    lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = 480;
    disp_drv.ver_res = 320;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.buffer = &disp_buf_2;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}
void lv_port_indev_init(void)
{
    /* Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */

    lv_indev_drv_t indev_drv;

    /*------------------
     * Touchpad
     * -----------------*/

    /*Initialize your touchpad if you have*/
    touchpad_init();

    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    indev_touchpad = lv_indev_drv_register(&indev_drv);
}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

    ili9488_set_window(area->x1, area->y1, area->x2, area->y2);

    uint32_t block_size = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);

    while(block_size--)
    {
        ili9488_send_pixel_data((rgb565_t)(color_p++)->full);
    }


    /* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}

/* Initialize your display and the required peripherals. */
void disp_bl_init(uint32_t ulFrequency)
{
    CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_TIMER1;

    TIMER1->CTRL = TIMER_CTRL_RSSCOIST | TIMER_CTRL_PRESC_DIV1 | TIMER_CTRL_CLKSEL_PRESCHFPERCLK | TIMER_CTRL_FALLA_NONE | TIMER_CTRL_RISEA_NONE | TIMER_CTRL_MODE_UP;
    TIMER1->TOP = (HFPER_CLOCK_FREQ / ulFrequency) - 1;
    TIMER1->CNT = 0x0000;

    TIMER1->CC[0].CTRL = TIMER_CC_CTRL_PRSCONF_LEVEL | TIMER_CC_CTRL_CUFOA_NONE | TIMER_CC_CTRL_COFOA_SET | TIMER_CC_CTRL_CMOA_CLEAR | TIMER_CC_CTRL_MODE_PWM;
    TIMER1->CC[0].CCV = 0x0000;

    TIMER1->ROUTELOC0 = TIMER_ROUTELOC0_CC0LOC_LOC2;
    TIMER1->ROUTEPEN |= TIMER_ROUTEPEN_CC0PEN;

    TIMER1->CMD = TIMER_CMD_START;

    TIMER1->CC[0].CCVB = 0;
}

void disp_bl_set(float fBrightness)
{
    if(fBrightness > 1.f)
        fBrightness = 1.f;
    if(fBrightness < 0.f)
        fBrightness = 0.f;

    TIMER1->CC[0].CCVB = TIMER1->TOP * fBrightness;
}

/*Initialize your touchpad*/
static void touchpad_init(void)
{
    ft6x36_isPressed = 0;
    ft6x36_touchXLoc = 0;
    ft6x36_touchYLoc = 0;
    /*Your code comes here*/
}
/* Will be called by the library to read the touchpad */
static bool touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    if(ft6x36_isPressed) // touchpad_is_pressed()
        data->state = LV_INDEV_STATE_PR;
    else
        data->state = LV_INDEV_STATE_REL;

    /*Set the pressed coordinates*/
    data->point.x = ft6x36_touchXLoc;
    data->point.y = ft6x36_touchYLoc;

    /*Return `false` because we are not buffering and no more data to read*/
    return false;
}




static void btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
    if(event == LV_EVENT_RELEASED) {
        /*Increase the button width*/
        lv_coord_t width = lv_obj_get_width(btn);
        lv_obj_set_width(btn, width + 20);
    }
}

static  void ddlist_event_cb(lv_obj_t * ddlist, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        uint16_t opt = lv_ddlist_get_selected(ddlist);            /*Get the id of selected option*/

        lv_slider_set_value(slider, (opt * 100) / 4, true);       /*Modify the slider value according to the selection*/
    }

}