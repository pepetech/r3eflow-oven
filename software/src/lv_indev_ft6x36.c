#include "lv_indev_ft6x36.h"

lv_indev_t * indev_touchpad;

static uint8_t lv_indev_ft6x36_isPressed = 0;
static uint16_t lv_indev_ft6x36_touchXLoc = 0;
static uint16_t lv_indev_ft6x36_touchYLoc = 0;

static uint8_t lv_indev_ft6x36_read_register(uint8_t ubRegister)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        i2c1_write_byte(FT6X06_I2C_ADDR, ubRegister, I2C_RESTART);
        return i2c1_read_byte(FT6X06_I2C_ADDR, I2C_STOP);
    }
}
static void lv_indev_ft6x36_write_register(uint8_t ubRegister, uint8_t ubValue)
{
    uint8_t pubBuffer[2];

    pubBuffer[0] = ubRegister;
    pubBuffer[1] = ubValue;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        i2c1_write(FT6X06_I2C_ADDR, pubBuffer, 2, I2C_STOP);
    }
}
static void lv_indev_ft6x36_rmw_register(uint8_t ubRegister, uint8_t ubMask, uint8_t ubValue)
{
    lv_indev_ft6x36_write_register(ubRegister, (lv_indev_ft6x36_read_register(ubRegister) & ubMask) | ubValue);
}

void lv_indev_ft6x36_isr()
{
    uint8_t pubBuf[4];

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        i2c1_write_byte(FT6X06_I2C_ADDR, FT6X06_REG_P1_XH, I2C_RESTART);
        i2c1_read(FT6X06_I2C_ADDR, pubBuf, 4, I2C_STOP);
    }

    uint8_t ubEvent = pubBuf[0] & 0xC0;

    uint16_t touchXLoc = ((uint16_t)(pubBuf[0] & 0x07) << 8) | (uint16_t)pubBuf[1];
    uint16_t touchYLoc = ((uint16_t)(pubBuf[2] & 0x07) << 8) | (uint16_t)pubBuf[3];

    #if (LV_ROTATE == ILI9488_HORIZONTAL)
    lv_indev_ft6x36_touchXLoc = touchYLoc;
    lv_indev_ft6x36_touchYLoc = ILI9488_TFTWIDTH - touchXLoc;
    #elif (LV_ROTATE == ILI9488_HORIZONTAL_FLIP)
    lv_indev_ft6x36_touchXLoc = ILI9488_TFTHEIGHT - touchYLoc;
    lv_indev_ft6x36_touchYLoc = touchXLoc;
    #elif (LV_ROTATE == ILI9488_VERTICAL)
    lv_indev_ft6x36_touchXLoc = touchXLoc;
    lv_indev_ft6x36_touchYLoc = touchYLoc;
    #elif (LV_ROTATE == ILI9488_VERTICAL_FLIP)
    lv_indev_ft6x36_touchXLoc = ILI9488_TFTWIDTH - touchXLoc;
    lv_indev_ft6x36_touchYLoc = ILI9488_TFTHEIGHT - touchYLoc;
    #endif

    switch(ubEvent)
    {
        case FT6X06_REG_Pn_XH_EVENT_FLAG_PRESS_DOWN:
            lv_indev_ft6x36_isPressed = 1;
        break;

        case FT6X06_REG_Pn_XH_EVENT_FLAG_CONTACT:
            lv_indev_ft6x36_isPressed = 1;
        break;

        default:
            lv_indev_ft6x36_isPressed = 0;
        break;
    }

    return;
}

/* Will be called by the library to read the touchpad */
static bool lv_indev_ft6x36_lv_drv_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    if(lv_indev_ft6x36_isPressed) // touchpad_is_pressed()
        data->state = LV_INDEV_STATE_PR;
    else
        data->state = LV_INDEV_STATE_REL;

    /*Set the pressed coordinates*/
    data->point.x = lv_indev_ft6x36_touchXLoc;
    data->point.y = lv_indev_ft6x36_touchYLoc;

    /*Return `false` because we are not buffering and no more data to read*/
    return false;
}

uint8_t lv_indev_ft6x36_init()
{
    FT6X36_RESET();
    delay_ms(10);
    FT6X36_UNRESET();
    delay_ms(120);

    if(!i2c1_write(FT6X06_I2C_ADDR, NULL, 0, I2C_STOP)) // Check ACK from the expected address
        return 0;

    //ft6x36_write_register(FT6X06_REG_PERIODACTIVE, 0x05);
    lv_indev_ft6x36_write_register(FT6X06_REG_TH_GROUP, 40);

    delay_ms(10);

    lv_indev_ft6x36_isPressed = 0;
    lv_indev_ft6x36_touchXLoc = 0;
    lv_indev_ft6x36_touchYLoc = 0;

    lv_indev_drv_t indev_drv;

    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lv_indev_ft6x36_lv_drv_read;
    indev_touchpad = lv_indev_drv_register(&indev_drv);

    return 1;
}

uint8_t lv_indev_ft6x36_get_vendor_id()
{
    return lv_indev_ft6x36_read_register(FT6X06_REG_FOCALTECH_ID);
}
uint8_t lv_indev_ft6x36_get_chip_id()
{
    return lv_indev_ft6x36_read_register(FT6X06_REG_CIPHER);
}
uint8_t lv_indev_ft6x36_get_firmware_version()
{
    return lv_indev_ft6x36_read_register(FT6X06_REG_FIRMID);
}