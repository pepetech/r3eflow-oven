
#include "gpio.h"

static void gpio_isr(uint32_t ulFlags)
{
    if(ulFlags & BIT(3))
    lv_indev_ft6x36_isr();
}
void _gpio_even_isr()
{
    uint32_t ulFlags = GPIO->IF;

    gpio_isr(ulFlags & 0x55555555);

    GPIO->IFC = 0x55555555; // Clear all even flags
}
void _gpio_odd_isr()
{
    uint32_t ulFlags = GPIO->IF;

    gpio_isr(ulFlags & 0xAAAAAAAA);

    GPIO->IFC = 0xAAAAAAAA; // Clear all odd flags
}

void gpio_init()
{
    CMU->HFBUSCLKEN0 |= CMU_HFBUSCLKEN0_GPIO;

    // NC - Not Connected (not available in mcu package)
    // NR - Not routed (no routing to pin on pcb, floating)
    // NU - Not used (not currently in use)

    // Port A
    GPIO->P[0].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[0].MODEL  = GPIO_P_MODEL_MODE0_DISABLED     // NR
                      | GPIO_P_MODEL_MODE1_PUSHPULL     // USB-SUS
                      | GPIO_P_MODEL_MODE2_PUSHPULL     // USB_RST
                      | GPIO_P_MODEL_MODE3_INPUTPULLFILTER // U0TX-USB-RX - Location 2
                      | GPIO_P_MODEL_MODE4_PUSHPULL     // U0RX-USB-TX - Location 2
                      | GPIO_P_MODEL_MODE5_DISABLED     // NR
                      | GPIO_P_MODEL_MODE6_DISABLED     // NR
                      | GPIO_P_MODEL_MODE7_DISABLED;    // NR
    GPIO->P[0].MODEH  = GPIO_P_MODEH_MODE8_PUSHPULL     // TIM0-CC0-BUZZ - Location 6
                      | GPIO_P_MODEH_MODE9_DISABLED     // NR
                      | GPIO_P_MODEH_MODE10_DISABLED    // NR
                      | GPIO_P_MODEH_MODE11_DISABLED    // NR
                      | GPIO_P_MODEH_MODE12_DISABLED    // NR
                      | GPIO_P_MODEH_MODE13_DISABLED    // NR
                      | GPIO_P_MODEH_MODE14_DISABLED    // NR
                      | GPIO_P_MODEH_MODE15_DISABLED;   // NR
    GPIO->P[0].DOUT   = BIT(3) | BIT(4);
    GPIO->P[0].OVTDIS = 0;

    // Port B
    GPIO->P[1].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (7 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (7 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[1].MODEL  = GPIO_P_MODEL_MODE0_PUSHPULL     // TIM1-CC0-TFT-BL - Location 0
                      | GPIO_P_MODEL_MODE1_DISABLED     // NR
                      | GPIO_P_MODEL_MODE2_DISABLED     // NR
                      | GPIO_P_MODEL_MODE3_INPUTPULLFILTER // TFT_IRQ
                      | GPIO_P_MODEL_MODE4_DISABLED     // NR
                      | GPIO_P_MODEL_MODE5_PUSHPULL     // TFT_RESET
                      | GPIO_P_MODEL_MODE6_PUSHPULL     // TFT_RST
                      | GPIO_P_MODEL_MODE7_DISABLED;    // MAIN_LFXTAL_P
    GPIO->P[1].MODEH  = GPIO_P_MODEH_MODE8_DISABLED     // MAIN_LFXTAL_N
                      | GPIO_P_MODEH_MODE9_DISABLED     // NR
                      | GPIO_P_MODEH_MODE10_DISABLED    // NR
                      | GPIO_P_MODEH_MODE11_WIREDANDPULLUPFILTER // I2C1-SDA-TTFT - Location 1
                      | GPIO_P_MODEH_MODE12_WIREDANDPULLUPFILTER // I2C1-SCL-TTFT - Location 1
                      | GPIO_P_MODEH_MODE13_DISABLED    // MAIN_HFXTAL_N
                      | GPIO_P_MODEH_MODE14_DISABLED    // MAIN_HFXTAL_P
                      | GPIO_P_MODEH_MODE15_DISABLED;   // NC
    GPIO->P[1].DOUT   = BIT(6) | BIT(11) | BIT(12);
    GPIO->P[1].OVTDIS = 0;

    // Port C
    GPIO->P[2].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[2].MODEL  = GPIO_P_MODEL_MODE0_DISABLED     // NR
                      | GPIO_P_MODEL_MODE1_PUSHPULL     // TFT_DC
                      | GPIO_P_MODEL_MODE2_PUSHPULL     // US2-MOSI-TFT - Location 0
                      | GPIO_P_MODEL_MODE3_INPUTPULLFILTER // US2-MISO-TFT - Location 0
                      | GPIO_P_MODEL_MODE4_PUSHPULL     // US2-CLK-TFT - Location 0
                      | GPIO_P_MODEL_MODE5_PUSHPULL     // US2-CS-TFT - Location 0
                      | GPIO_P_MODEL_MODE6_DISABLED     // NR
                      | GPIO_P_MODEL_MODE7_INPUTPULLFILTER; // DOOR-SW
    GPIO->P[2].MODEH  = GPIO_P_MODEH_MODE8_PUSHPULL     // TIM5-CC0-FAN0-GND - Location 4
                      | GPIO_P_MODEH_MODE9_PUSHPULL     // TIM5-CC1-FAN0-CTRL - Location 4
                      | GPIO_P_MODEH_MODE10_INPUTPULLFILTER // TIM2-CC2FAN0-SNSE - Location 2
                      | GPIO_P_MODEH_MODE11_DISABLED    // NR
                      | GPIO_P_MODEH_MODE12_DISABLED    // NC
                      | GPIO_P_MODEH_MODE13_DISABLED    // NC
                      | GPIO_P_MODEH_MODE14_DISABLED    // NC
                      | GPIO_P_MODEH_MODE15_DISABLED;   // NC
    GPIO->P[2].DOUT   = BIT(5);
    GPIO->P[2].OVTDIS = 0;

    // Port D
    GPIO->P[3].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[3].MODEL  = _GPIO_P_MODEL_MODE0_INPUT     // VIN_SENSE
                      | GPIO_P_MODEL_MODE1_DISABLED     // NR
                      | GPIO_P_MODEL_MODE2_INPUTPULLFILTER     // 5V_ERR
                      | GPIO_P_MODEL_MODE3_PUSHPULL     // WTIM2-CC0-SERVO0-SIG - Location 5
                      | GPIO_P_MODEL_MODE4_INPUT        // 5V-SENSE
                      | GPIO_P_MODEL_MODE5_PUSHPULL     // PRS-CH11-SSR-OUT - Location 2
                      | GPIO_P_MODEL_MODE6_WIREDANDFILTER     // I2C0-SDA-SENS - Location 1
                      | GPIO_P_MODEL_MODE7_WIREDANDFILTER;    // I2C0-SCL-SENS - Location 1
    GPIO->P[3].MODEH  = GPIO_P_MODEH_MODE8_DISABLED     // NR
                      | GPIO_P_MODEH_MODE9_PUSHPULL     // QSPI0_DQ0 - Location 0
                      | GPIO_P_MODEH_MODE10_PUSHPULL    // QSPI0_DQ1 - Location 0
                      | GPIO_P_MODEH_MODE11_PUSHPULL    // QSPI0_DQ2 - Location 0
                      | GPIO_P_MODEH_MODE12_PUSHPULL    // QSPI0_DQ3 - Location 0
                      | GPIO_P_MODEH_MODE13_DISABLED    // NC
                      | GPIO_P_MODEH_MODE14_DISABLED    // NC
                      | GPIO_P_MODEH_MODE15_DISABLED;   // NC
    GPIO->P[3].DOUT   = 0;
    GPIO->P[3].OVTDIS = 0;

    // Port E
    GPIO->P[4].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[4].MODEL  = GPIO_P_MODEL_MODE0_PUSHPULL     // BLE-!RST
                      | GPIO_P_MODEL_MODE1_PUSHPULL     // BLE-KEY
                      | GPIO_P_MODEL_MODE2_INPUTPULLFILTER // U1TX-BLE-RX - Location 3
                      | GPIO_P_MODEL_MODE3_PUSHPULL     // U1RX-BLE-TX - Location 3
                      | GPIO_P_MODEL_MODE4_INPUTPULLFILTER // WTIM1-CC1-ZERO-CROSS - Location 4
                      | GPIO_P_MODEL_MODE5_PUSHPULL     // US0-LED-SCK - Location 1
                      | GPIO_P_MODEL_MODE6_DISABLED     // NR
                      | GPIO_P_MODEL_MODE7_PUSHPULL;    // US0-LED-MOSI - Location 1
    GPIO->P[4].MODEH  = GPIO_P_MODEH_MODE8_PUSHPULL     // SDIO-DAT3 - Location 0
                      | GPIO_P_MODEH_MODE9_PUSHPULL     // SDIO-DAT2 - Location 0
                      | GPIO_P_MODEH_MODE10_PUSHPULL    // SDIO-DAT1 - Location 0
                      | GPIO_P_MODEH_MODE11_INPUTPULL   // SDIO-DAT0 - Location 0
                      | GPIO_P_MODEH_MODE12_PUSHPULL    // SDIO-CMD - Location 0
                      | GPIO_P_MODEH_MODE13_PUSHPULL    // SDIO-CLK - Location 0
                      | GPIO_P_MODEH_MODE14_DISABLED    // NR
                      | GPIO_P_MODEH_MODE15_DISABLED;   // NR
    GPIO->P[4].DOUT   = BIT(2) | BIT(4);
    GPIO->P[4].OVTDIS = 0;

    // Port F
    GPIO->P[5].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[5].MODEL  = GPIO_P_MODEL_MODE0_PUSHPULL     // DBG_SWCLK - Location 0
                      | GPIO_P_MODEL_MODE1_PUSHPULL     // DBG_SWDIO - Location 0
                      | GPIO_P_MODEL_MODE2_PUSHPULL     // DBG_SWO - Location 0
                      | GPIO_P_MODEL_MODE3_DISABLED     // NC
                      | GPIO_P_MODEL_MODE4_DISABLED     // NC
                      | GPIO_P_MODEL_MODE5_DISABLED     // NR
                      | GPIO_P_MODEL_MODE6_PUSHPULL     // QSPI0_SCLK - Location 0
                      | GPIO_P_MODEL_MODE7_PUSHPULL;    // QSPI0_CS0 - Location 0
    GPIO->P[5].MODEH  = GPIO_P_MODEH_MODE8_INPUTPULLFILTER // SDIO-CD - Location 0
                      | GPIO_P_MODEH_MODE9_DISABLED     // NR
                      | GPIO_P_MODEH_MODE10_DISABLED    // NR
                      | GPIO_P_MODEH_MODE11_DISABLED    // NR
                      | GPIO_P_MODEH_MODE12_DISABLED    // NR
                      | GPIO_P_MODEH_MODE13_DISABLED    // NC
                      | GPIO_P_MODEH_MODE14_DISABLED    // NC
                      | GPIO_P_MODEH_MODE15_DISABLED;   // NC
    GPIO->P[5].DOUT   = BIT(7) | BIT(8);
    GPIO->P[5].OVTDIS = 0;

    // Debugger Route
    GPIO->ROUTEPEN &= ~(GPIO_ROUTEPEN_TDIPEN | GPIO_ROUTEPEN_TDOPEN); // Disable JTAG
    GPIO->ROUTEPEN |= GPIO_ROUTEPEN_SWVPEN; // Enable SWO
    GPIO->ROUTELOC0 = GPIO_ROUTELOC0_SWVLOC_LOC0; // SWO on PF2

    // External interrupts
    GPIO->EXTIPSELL = GPIO_EXTIPSELL_EXTIPSEL0_PORTB            // TFT_IRQ
                    | GPIO_EXTIPSELL_EXTIPSEL1_PORTB            //
                    | GPIO_EXTIPSELL_EXTIPSEL2_PORTB            //
                    | GPIO_EXTIPSELL_EXTIPSEL3_PORTB            //
                    | GPIO_EXTIPSELL_EXTIPSEL4_PORTA            //
                    | GPIO_EXTIPSELL_EXTIPSEL5_PORTA            //
                    | GPIO_EXTIPSELL_EXTIPSEL6_PORTC            //
                    | GPIO_EXTIPSELL_EXTIPSEL7_PORTC;           //
    GPIO->EXTIPSELH = GPIO_EXTIPSELH_EXTIPSEL8_PORTA            //
                    | GPIO_EXTIPSELH_EXTIPSEL9_PORTE            //
                    | GPIO_EXTIPSELH_EXTIPSEL10_PORTF           //
                    | GPIO_EXTIPSELH_EXTIPSEL11_PORTA           //
                    | GPIO_EXTIPSELH_EXTIPSEL12_PORTA           //
                    | GPIO_EXTIPSELH_EXTIPSEL13_PORTE           //
                    | GPIO_EXTIPSELH_EXTIPSEL14_PORTF           //
                    | GPIO_EXTIPSELH_EXTIPSEL15_PORTA;          //

    GPIO->EXTIPINSELL = GPIO_EXTIPINSELL_EXTIPINSEL0_PIN3       // TFT_IRQ
                      | GPIO_EXTIPINSELL_EXTIPINSEL1_PIN1       //
                      | GPIO_EXTIPINSELL_EXTIPINSEL2_PIN2       //
                      | GPIO_EXTIPINSELL_EXTIPINSEL3_PIN3       //
                      | GPIO_EXTIPINSELL_EXTIPINSEL4_PIN6       //
                      | GPIO_EXTIPINSELL_EXTIPINSEL5_PIN7       //
                      | GPIO_EXTIPINSELL_EXTIPINSEL6_PIN4       //
                      | GPIO_EXTIPINSELL_EXTIPINSEL7_PIN7;      //
    GPIO->EXTIPINSELH = GPIO_EXTIPINSELH_EXTIPINSEL8_PIN8       //
                      | GPIO_EXTIPINSELH_EXTIPINSEL9_PIN9       //
                      | GPIO_EXTIPINSELH_EXTIPINSEL10_PIN11     //
                      | GPIO_EXTIPINSELH_EXTIPINSEL11_PIN8      //
                      | GPIO_EXTIPINSELH_EXTIPINSEL12_PIN13     //
                      | GPIO_EXTIPINSELH_EXTIPINSEL13_PIN15     //
                      | GPIO_EXTIPINSELH_EXTIPINSEL14_PIN12     //
                      | GPIO_EXTIPINSELH_EXTIPINSEL15_PIN12;    //

    GPIO->EXTIRISE = 0; //
    GPIO->EXTIFALL = BIT(3); // TFT_IRQ

    GPIO->IFC = _GPIO_IFC_MASK; // Clear pending IRQs
    IRQ_CLEAR(GPIO_EVEN_IRQn); // Clear pending vector
    IRQ_CLEAR(GPIO_ODD_IRQn); // Clear pending vector
    IRQ_SET_PRIO(GPIO_EVEN_IRQn, 0, 0); // Set priority 0,0 (max)
    IRQ_SET_PRIO(GPIO_ODD_IRQn, 0, 0); // Set priority 0,0 (max)
    IRQ_ENABLE(GPIO_EVEN_IRQn); // Enable vector
    IRQ_ENABLE(GPIO_ODD_IRQn); // Enable vector
    GPIO->IEN = BIT(3); // Enable interrupts
}