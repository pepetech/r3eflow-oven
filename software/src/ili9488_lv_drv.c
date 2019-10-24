#include "ili9488_lv_drv.h"

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */
static void ili9488_lv_drv_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
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

void ili9488_lv_drv_init(void)
{
    /*-------------------------
     * Initialize display
     * -----------------------*/
    ili9488_init();

    ili9488_set_rotation(1);

    ili9488_display_on();

    ili9488_lv_drv_bl_init(2000);
    ili9488_lv_drv_bl_set(0.5);

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
    static lv_color_t buf2_1[LV_HOR_RES_MAX * 60];                        /*A buffer for 10 rows*/
    static lv_color_t buf2_2[LV_HOR_RES_MAX * 60];                        /*An other buffer for 10 rows*/
    lv_disp_buf_init(&disp_buf_2, buf2_1, buf2_2, LV_HOR_RES_MAX * 10);   /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LittlevGL
     *----------------------------------*/
    lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = 480;
    disp_drv.ver_res = 320;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = ili9488_lv_drv_flush;

    /*Set a display buffer*/
    disp_drv.buffer = &disp_buf_2;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/* Initialize backlight pwm at desired frequency. */
void ili9488_lv_drv_bl_init(uint32_t ulFrequency)
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
void ili9488_lv_drv_bl_set(float fBrightness)
{
    if(fBrightness > 1.f) fBrightness = 1.f;
    if(fBrightness < 0.f) fBrightness = 0.f;

    TIMER1->CC[0].CCVB = TIMER1->TOP * fBrightness;
}