#include "lv_disp_ili9488.h"

static void lv_disp_ili9488_send_cmd(uint8_t ubCmd, uint8_t *pubParam, uint8_t ubNParam)
{
    ILI9488_SELECT();
    ILI9488_SETUP_CMD();

    usart2_spi_transfer_byte(ubCmd);

    if(pubParam && ubNParam)
    {
        ILI9488_SETUP_DAT();

        usart2_spi_write(pubParam, ubNParam, 1);
    }

    ILI9488_UNSELECT();
}
static void lv_disp_ili9488_read_data(uint8_t ubCmd, uint8_t *pubData, uint8_t ubNData)
{
    ILI9488_SELECT();
    ILI9488_SETUP_CMD();

    usart2_spi_transfer_byte(ubCmd);

    if(pubData && ubNData)
    {
        ILI9488_SETUP_DAT();

        //usart2_spi_transfer_byte(0x00); // dummy byte
        usart2_spi_read(pubData, ubNData, 0x00);

        ILI9488_UNSELECT();
    }
}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */
static void lv_disp_ili9488_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    uint8_t ubBuf[4];

    ubBuf[0] = area->x1 >> 8;
    ubBuf[1] = area->x1 & 0x00FF; // XSTART
    ubBuf[2] = area->x2 >> 8;
    ubBuf[3] = area->x2 & 0x00FF; // XEND
    lv_disp_ili9488_send_cmd(ILI9488_C_ADDR_SET, ubBuf, 4); // Column addr set

    ubBuf[0] = area->y1 >> 8;
    ubBuf[1] = area->y1 & 0x00FF; // YSTART
    ubBuf[2] = area->y2 >> 8;
    ubBuf[3] = area->y2 & 0x00FF; // YEND
    lv_disp_ili9488_send_cmd(ILI9488_P_ADDR_SET, ubBuf, 4); // Row addr set

    lv_disp_ili9488_send_cmd(ILI9488_RAM_WR, NULL, 0); // write to RAM

    uint32_t block_size = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);

    ILI9488_SELECT();
    ILI9488_SETUP_DAT();

    while(block_size--)
    {
        usart2_spi_write_byte((rgb565_t)(color_p)->ch.red << 3, 0);
        usart2_spi_write_byte((rgb565_t)(color_p)->ch.green << 2, 0);
        usart2_spi_write_byte((rgb565_t)(color_p++)->ch.blue << 3, 0);
    }

    ILI9488_UNSELECT();

    /* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}

uint8_t lv_disp_ili9488_init()
{
    // TODO: Add ili9844_read_id() and check if it is valid, otherwise return 0

    ILI9488_RESET();
    delay_ms(10);
    ILI9488_UNRESET();
    delay_ms(120);

    uint8_t ubBuf[15];

    ubBuf[0] =  0x00;
    ubBuf[1] =  0x03;
    ubBuf[2] =  0x09;
    ubBuf[3] =  0x08;
    ubBuf[4] =  0x16;
    ubBuf[5] =  0x0A;
    ubBuf[6] =  0x3F;
    ubBuf[7] =  0x78;
    ubBuf[8] =  0x4C;
    ubBuf[9] =  0x09;
    ubBuf[10] = 0x0A;
    ubBuf[11] = 0x08;
    ubBuf[12] = 0x16;
    ubBuf[13] = 0x1A;
    ubBuf[14] = 0x0F;
    lv_disp_ili9488_send_cmd(ILI9488_PGAMCTRL, ubBuf, 15); // PGAMCTRL(Positive Gamma Control)

    ubBuf[0] =  0x00;
    ubBuf[1] =  0x16;
    ubBuf[2] =  0x19;
    ubBuf[3] =  0x03;
    ubBuf[4] =  0x0F;
    ubBuf[5] =  0x05;
    ubBuf[6] =  0x32;
    ubBuf[7] =  0x45;
    ubBuf[8] =  0x46;
    ubBuf[9] =  0x04;
    ubBuf[10] = 0x0E;
    ubBuf[11] = 0x0D;
    ubBuf[12] = 0x35;
    ubBuf[13] = 0x37;
    ubBuf[14] = 0x0F;
	lv_disp_ili9488_send_cmd(ILI9488_NGAMCTRL, ubBuf, 15); // NGAMCTRL(Negative Gamma Control)

    ubBuf[0] = 0x17; // VReg1out
    ubBuf[1] = 0x15; // VReg2out
	lv_disp_ili9488_send_cmd(ILI9488_POW_CTL_1, ubBuf, 2); // Power Control 1

    ubBuf[0] = 0x41; // VGH,VGL
	lv_disp_ili9488_send_cmd(ILI9488_POW_CTL_2, ubBuf, 1); // Power Control 2

    ubBuf[0] = 0x00;
	ubBuf[0] = 0x12; // VCom
	ubBuf[0] = 0x80;
	lv_disp_ili9488_send_cmd(ILI9488_VCOM_CTL_1, ubBuf, 3); // Power Control 3

    ubBuf[0] = 0x48; // MX | BGR
	lv_disp_ili9488_send_cmd(ILI9488_MEM_A_CTL, ubBuf, 1); // Memory Access

    ubBuf[0] = 0x66; // 18 bit
	lv_disp_ili9488_send_cmd(ILI9488_PIX_FMT, ubBuf, 1); // Interface Pixel Format

    ubBuf[0] = 0x00;
	lv_disp_ili9488_send_cmd(ILI9488_IF_MD_CTL, ubBuf, 1); // Interface Mode Control

    ubBuf[0] = 0xA0; // 60Hz
	lv_disp_ili9488_send_cmd(ILI9488_FRMRT_CTL_1, ubBuf, 1); // Frame rate

    ubBuf[0] = 0x02; // 2-dot
	lv_disp_ili9488_send_cmd(ILI9488_INV_CTL, ubBuf, 1); // Display Inversion Control

    ubBuf[0] = 0x02; // MCU
    ubBuf[1] = 0x02; // Source, Gate scan direction
	lv_disp_ili9488_send_cmd(ILI9488_DISP_FUNC_CTL, ubBuf, 2); // Display Function Control RGB/MCU Interface Control

    ubBuf[0] = 0x00; // Disable 24 bit data
	lv_disp_ili9488_send_cmd(ILI9488_SET_IMG_FUNC, ubBuf, 1); // Set Image Functio

    ubBuf[0] = 0xA9;
    ubBuf[1] = 0x51;
    ubBuf[2] = 0x2C;
    ubBuf[3] = 0x82; // D7 stream, loose
	lv_disp_ili9488_send_cmd(ILI9488_ADJ_CTL_3, ubBuf, 4); // Adjust Control

	lv_disp_ili9488_wakeup(0); // Wake up controller, leave display off

    #if LV_ROTATE == ILI9488_VERTICAL
    uint8_t ubParamBuff = ILI9488_MADCTL_MX | ILI9488_MADCTL_BGR;
    lv_disp_ili9488_send_cmd(ILI9488_MEM_A_CTL, &ubParamBuff, 1);
    #elif LV_ROTATE == ILI9488_HORIZONTAL
    uint8_t ubParamBuff = ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR;
    lv_disp_ili9488_send_cmd(ILI9488_MEM_A_CTL, &ubParamBuff, 1);
    #elif LV_ROTATE == ILI9488_VERTICAL_FLIP
    uint8_t ubParamBuff = ILI9488_MADCTL_MY | ILI9488_MADCTL_BGR;
    lv_disp_ili9488_send_cmd(ILI9488_MEM_A_CTL, &ubParamBuff, 1);
    #elif LV_ROTATE == ILI9488_HORIZONTAL_FLIP
    uint8_t ubParamBuff = ILI9488_MADCTL_MX | ILI9488_MADCTL_MY | ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR;
    lv_disp_ili9488_send_cmd(ILI9488_MEM_A_CTL, &ubParamBuff, 1);
    #endif

    /*-----------------------------
     * Create a buffers for drawing
     *----------------------------*/

    /* Create TWO buffer with some rows:
     *      LittlevGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LittlevGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     */
    static lv_disp_buf_t disp_buf_2;
    static lv_color_t buf2_1[LV_HOR_RES_MAX * 10];                        /*A buffer for 10 rows*/
    static lv_color_t buf2_2[LV_HOR_RES_MAX * 10];                        /*An other buffer for 10 rows*/
    lv_disp_buf_init(&disp_buf_2, buf2_1, buf2_2, LV_HOR_RES_MAX * 10);   /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LittlevGL
     *----------------------------------*/
    lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = lv_disp_ili9488_flush;

    /*Set a display buffer*/
    disp_drv.buffer = &disp_buf_2;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);

    return 1;
}

uint32_t lv_disp_ili9488_read_id() // FIXME: returns 000000
{
    uint8_t ubBuf[3];

    lv_disp_ili9488_read_data(ILI9488_RD_DISP_ID, ubBuf, 3);

    return ((uint32_t)ubBuf[0] << 16) | ((uint32_t)ubBuf[1] << 8) | (uint32_t)ubBuf[2];
}

void lv_disp_ili9488_sleep()
{
    lv_disp_ili9488_display_off();
    delay_ms(10);
    lv_disp_ili9488_send_cmd(ILI9488_SLP_IN, NULL, 0);  // Internal oscillator will be stopped
    delay_ms(120);
}
void lv_disp_ili9488_wakeup(uint8_t ubDisplayOn)
{
    lv_disp_ili9488_send_cmd(ILI9488_SLP_OUT, NULL, 0); // Sleep out
	delay_ms(120);

    if(ubDisplayOn)
        lv_disp_ili9488_display_on();
}

void lv_disp_ili9488_display_on()
{
    lv_disp_ili9488_send_cmd(ILI9488_DISP_ON, NULL, 0); //Display on
}
void lv_disp_ili9488_display_off()
{
    lv_disp_ili9488_send_cmd(ILI9488_DISP_OFF, NULL, 0); //Display off
}

void lv_disp_ili9488_set_invert(uint8_t ubOnOff)
{
    lv_disp_ili9488_send_cmd((ubOnOff ? ILI9488_INV_ON : ILI9488_INV_OFF), NULL, 0);
}

/* Initialize backlight pwm at desired frequency. */
void lv_disp_ili9488_bl_init(uint32_t ulFrequency)
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

/* Initialize backlight pwm output compare value (0 <= fBrightness <= 1). */
void lv_disp_ili9488_bl_set(float fBrightness)
{
    if(fBrightness > 1.f) fBrightness = 1.f;
    if(fBrightness < 0.f) fBrightness = 0.f;

    TIMER1->CC[0].CCVB = TIMER1->TOP * fBrightness;
}