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
#include "lv_disp_ili9488.h"
#include "lv_indev_ft6x36.h"
#include "lvgl.h"
#include "oven.h"
#include "ui.h"

// Defines

// Structs

// Forward declarations
static void reset() __attribute__((noreturn));
static void sleep();

static uint32_t get_free_ram();

void get_device_name(char *pszDeviceName, uint32_t ulDeviceNameSize);
static uint16_t get_device_revision();

// lv forwards
static void btn_event_cb(lv_obj_t * btn, lv_event_t event);
static void ddlist_event_cb(lv_obj_t * ddlist, lv_event_t event);

// lv Variables
static lv_obj_t * slider;

// ISRs

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

    return 0;
}
int main()
{
    play_sound(2000, 100);
    delay_ms(50);
    play_sound(2000, 100);

    // QSPI
    DBGPRINTLN_CTX("Flash Part ID: %06X", qspi_flash_read_jedec_id());

    uint8_t ubFlashUID[8];

    qspi_flash_read_security(0x0000, ubFlashUID, 8);

    DBGPRINTLN_CTX("Flash ID: %02X%02X%02X%02X%02X%02X%02X%02X", ubFlashUID[0], ubFlashUID[1], ubFlashUID[2], ubFlashUID[3], ubFlashUID[4], ubFlashUID[5], ubFlashUID[6], ubFlashUID[7]);


    // LEDs init
    sk9822_init();
    DBGPRINTLN_CTX("SK9822 LEDs Init!");

    // tft + LvGL init
    lv_init();
    lv_disp_ili9488_init();
    DBGPRINTLN_CTX("ILI9488 init!");
    lv_indev_ft6x36_init();
    DBGPRINTLN_CTX("FT6236 init!");

    lv_disp_ili9488_display_on();
    lv_disp_ili9488_bl_init(20000);
    lv_disp_ili9488_bl_set(0.8);

    // MCP9600 ID
    DBGPRINTLN_CTX("MCP9600 #0 ID 0x%02X Revision 0x%02X", mcp9600_get_id(0), mcp9600_get_revision(0));
    DBGPRINTLN_CTX("MCP9600 #7 ID 0x%02X Revision 0x%02X", mcp9600_get_id(7), mcp9600_get_revision(7));

    // MCP9600 init
    mcp9600_set_sensor_config(0, MCP9600_TYPE_K | MCP9600_FILT_COEF_0);
    mcp9600_set_config(0, MCP9600_BURST_TS_1 | MCP9600_MODE_NORMAL);

    mcp9600_set_sensor_config(7, MCP9600_TYPE_K | MCP9600_FILT_COEF_0);
    mcp9600_set_config(7, MCP9600_BURST_TS_1 | MCP9600_MODE_NORMAL);

    oven_init();

    do
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

    } while(0);

    for(;;)
    {
        static uint64_t ullLastBlink = 0;
        static uint64_t ullLastInput = 0;
        static uint64_t ullLastLvGLUpdate = 0;

        lv_task_handler();

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

        oven_task();
    }

    return 0;
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