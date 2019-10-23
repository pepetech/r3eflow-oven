
#include "ft6x36.h"

static volatile ft6x36_event_t pxTouchFIFO[FT6X36_TOUCH_FIFO_SIZE];
static volatile uint8_t ubTouchFIFORd = 0;
static volatile uint8_t ubTouchFIFOWr = 0;
static ft6x36_event_callback_fn_t pfEventCallback = NULL;

volatile uint8_t ft6x36_isPressed = 0;
volatile uint16_t ft6x36_touchXLoc = 0;
volatile uint16_t ft6x36_touchYLoc = 0;

static uint8_t ft6x36_read_register(uint8_t ubRegister)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        i2c1_write_byte(FT6X06_I2C_ADDR, ubRegister, I2C_RESTART);
        return i2c1_read_byte(FT6X06_I2C_ADDR, I2C_STOP);
    }
}
static void ft6x36_write_register(uint8_t ubRegister, uint8_t ubValue)
{
    uint8_t pubBuffer[2];

    pubBuffer[0] = ubRegister;
    pubBuffer[1] = ubValue;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        i2c1_write(FT6X06_I2C_ADDR, pubBuffer, 2, I2C_STOP);
    }
}
static void ft6x36_rmw_register(uint8_t ubRegister, uint8_t ubMask, uint8_t ubValue)
{
    ft6x36_write_register(ubRegister, (ft6x36_read_register(ubRegister) & ubMask) | ubValue);
}

uint8_t ft6x36_init()
{
    FT6X36_RESET();
    delay_ms(10);
    FT6X36_UNRESET();
    delay_ms(120);

    if(!i2c1_write(FT6X06_I2C_ADDR, NULL, 0, I2C_STOP)) // Check ACK from the expected address
        return 0;

    //ft6x36_write_register(FT6X06_REG_PERIODACTIVE, 0x05);
    ft6x36_write_register(FT6X06_REG_TH_GROUP, 40);

    return 1;
}
void ft6x36_isr()
{
    uint8_t pubBuf[4];

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        i2c1_write_byte(FT6X06_I2C_ADDR, FT6X06_REG_P1_XH, I2C_RESTART);
        i2c1_read(FT6X06_I2C_ADDR, pubBuf, 4, I2C_STOP);
    }

    uint8_t ubEvent = pubBuf[0] & 0xC0;

    ft6x36_touchXLoc = ((uint16_t)(pubBuf[0] & 0x07) << 8) | (uint16_t)pubBuf[1];
    ft6x36_touchYLoc = ((uint16_t)(pubBuf[2] & 0x07) << 8) | (uint16_t)pubBuf[3];

    //ft6x36_touchXLoc = (ft6x36_touchXLoc * 480) / 320;
    //ft6x36_touchYLoc = (ft6x36_touchYLoc * 320) / 480;

    switch(ubEvent)
    {
        case FT6X06_REG_Pn_XH_EVENT_FLAG_PRESS_DOWN:
            ft6x36_isPressed = 1;
        break;

        case FT6X06_REG_Pn_XH_EVENT_FLAG_CONTACT:
            ft6x36_isPressed = 1;
        break;

        default:
            ft6x36_isPressed = 0;
        break;
    }

    return;
}
void ft6x36_tick()
{
    while((FT6X36_TOUCH_FIFO_SIZE + ubTouchFIFOWr - ubTouchFIFORd) % FT6X36_TOUCH_FIFO_SIZE)
    {
        if(pfEventCallback)
            pfEventCallback(pxTouchFIFO[ubTouchFIFORd].ubEvent, pxTouchFIFO[ubTouchFIFORd].usX, pxTouchFIFO[ubTouchFIFORd].usY);

        ubTouchFIFORd++;
        if(ubTouchFIFORd >= FT6X36_TOUCH_FIFO_SIZE)
            ubTouchFIFORd = 0;
    }
}

void ft6x36_set_event_callback(ft6x36_event_callback_fn_t pfFunc)
{
    pfEventCallback = pfFunc;
}

uint8_t ft6x36_get_vendor_id()
{
    return ft6x36_read_register(FT6X06_REG_FOCALTECH_ID);
}
uint8_t ft6x36_get_chip_id()
{
    return ft6x36_read_register(FT6X06_REG_CIPHER);
}
uint8_t ft6x36_get_firmware_version()
{
    return ft6x36_read_register(FT6X06_REG_FIRMID);
}