#include "ft6x36_lv_drv.h"

/* Will be called by the library to read the touchpad */
static bool ft6x36_lv_drv_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    if(ft6x36_is_pressed()) // touchpad_is_pressed()
        data->state = LV_INDEV_STATE_PR;
    else
        data->state = LV_INDEV_STATE_REL;

    /*Set the pressed coordinates*/
    data->point.x = ft6x36_get_x_coord();
    data->point.y = ft6x36_get_y_coor();

    /*Return `false` because we are not buffering and no more data to read*/
    return false;
}

void ft6x36_lv_drv_init(void)
{
    lv_indev_drv_t indev_drv;

    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    indev_touchpad = lv_indev_drv_register(&indev_drv);
}