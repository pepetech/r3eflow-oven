#include "ui.h"

typedef enum {
    LED_OFF,
    LED_STATIC,
    LED_SCROLL,
    LED_PROGRESS,
    LED_BREATHING
} led_mode_t;

typedef union
{
    struct
    {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
    } ch;
    uint32_t full;
} led_color_t;

typedef struct
{
    led_mode_t mode;
    led_color_t color;
} led_style_t;

led_style_t xLedStyle;

// func prototypes
static void create_main_tab(lv_obj_t * parent);
static void create_data_tab(lv_obj_t * parent);
static void create_profile_tab(lv_obj_t * parent);
static void create_settings_tab(lv_obj_t * parent);

static void abort_popup_event(lv_obj_t * obj, lv_event_t event);
static void start_btn_event_cb(lv_obj_t * btn, lv_event_t event);
static void abort_btn_event_cb(lv_obj_t * btn, lv_event_t event);

void ui_init()
{
    lv_theme_t * th = lv_theme_material_init(210, NULL);
    lv_theme_set_current(th);
    //th = lv_theme_get_current();    /*If `LV_THEME_LIVE_UPDATE  1` `th` is not used directly so get the real theme after set*/

    /********************
     * CREATE A SCREEN
     *******************/
    /* Create a new screen and load it */
    lv_obj_t * scr = lv_cont_create(NULL, NULL);
    lv_disp_load_scr(scr);

    lv_obj_t * tv = lv_tabview_create(scr, NULL);
    lv_obj_set_size(tv, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_t * mainTab = lv_tabview_add_tab(tv, "Main");
    lv_obj_t * dataTab = lv_tabview_add_tab(tv, "Data");
    lv_obj_t * profileTab = lv_tabview_add_tab(tv, "Profile");
    lv_obj_t * settingsTab = lv_tabview_add_tab(tv, "Settings");

    create_main_tab(mainTab);
    create_data_tab(dataTab);
    create_profile_tab(profileTab);
    create_settings_tab(settingsTab);

    ui_set_led_style(STYLE_IDLE);
}

void ui_set_led_style(led_style_presets_t ledStyle)
{
    switch(ledStyle)
    {
        case STYLE_ABORT:
            xLedStyle.color.full = 0x000000FF;
            xLedStyle.mode = LED_STATIC;
            break;
        case STYLE_IDLE:
            xLedStyle.color.full = 0x00FF0000;
            xLedStyle.mode = LED_STATIC;
            break;
        case STYLE_WORKING:
            xLedStyle.color.full = 0x0000FF00;
            xLedStyle.mode = LED_SCROLL;
            break;
    }
}

void ui_abort_popup(ovenErr_t reason)
{
    static const char * btns[] ={"", "Ok", "", ""};

    lv_obj_t * abrtPopup = lv_mbox_create(lv_layer_top(), NULL);
    lv_mbox_set_text(abrtPopup, ovenERR_str[reason]);
    lv_mbox_add_btns(abrtPopup, btns);
    lv_obj_set_width(abrtPopup, 200);
    lv_obj_set_event_cb(abrtPopup, abort_popup_event);
    lv_obj_align(abrtPopup, NULL, LV_ALIGN_CENTER, 0, 0); /*Align to the corner*/
}

void led_task()
{
    static uint8_t led_scroll_pos;
    static uint8_t led_scroll_rate;

    led_mode_t lastLedMode = LED_OFF;

    if (xLedStyle.mode != lastLedMode)
    {
        switch(xLedStyle.mode)
        {
            case LED_OFF:

                for (uint8_t i = 0; i < SK9822_NUM_LEDS; i++)
                {
                    sk9822_set_color(i, 0x00, 0x00, 0x00, 0x00, 0);
                }
                sk9822_update();

                break;
            case LED_STATIC:

                for (uint8_t i = 0; i < SK9822_NUM_LEDS; i++)
                {
                    sk9822_set_color(i, 1, xLedStyle.color.ch.red, xLedStyle.color.ch.green, xLedStyle.color.ch.blue, 0);
                }
                sk9822_update();

                break;
            case LED_SCROLL:

                led_scroll_pos = 0;
                led_scroll_rate = 100;

                break;

            case LED_PROGRESS: // TODO
            case LED_BREATHING: // TODO
            default:
                xLedStyle.mode = LED_OFF;
                break;
        }

        lastLedMode = xLedStyle.mode;
    }

    switch(xLedStyle.mode)
    {
        case LED_OFF:
        case LED_STATIC:
            break;
        case LED_SCROLL:
            {
            static uint64_t ullLastScrollUpdate = 0;

            if(g_ullSystemTick > (ullLastScrollUpdate + led_scroll_rate))
            {
                uint8_t leds[4];

                switch(led_scroll_pos)
                {
                    case 0:
                        leds[0] = led_scroll_pos;
                        leds[1] = SK9822_LAST_LED;
                        leds[2] = SK9822_LAST_LED - 1;
                        leds[3] = SK9822_LAST_LED - 2;
                        break;
                    case 1:
                        leds[0] = led_scroll_pos;
                        leds[1] = 0;
                        leds[2] = SK9822_LAST_LED;
                        leds[3] = SK9822_LAST_LED - 1;
                        break;
                    case 2:
                        leds[0] = led_scroll_pos;
                        leds[1] = 1;
                        leds[2] = 0;
                        leds[3] = SK9822_LAST_LED;
                        break;
                    default:
                        leds[0] = led_scroll_pos;
                        leds[1] = led_scroll_pos - 1;
                        leds[2] = led_scroll_pos - 2;
                        leds[3] = led_scroll_pos - 3;
                        break;
                }

                sk9822_set_color(leds[0], SK9822_MAX_BRIGHTNESS, xLedStyle.color.ch.red, xLedStyle.color.ch.green, xLedStyle.color.ch.blue, 0);
                sk9822_set_color(leds[1], SK9822_TWOTHIRD_BRIGHTNESS, xLedStyle.color.ch.red, xLedStyle.color.ch.green, xLedStyle.color.ch.blue, 0);
                sk9822_set_color(leds[2], SK9822_THIRD_BRIGHTNESS, xLedStyle.color.ch.red, xLedStyle.color.ch.green, xLedStyle.color.ch.blue, 0);
                sk9822_set_color(leds[3], 0x00, 0x00, 0x00, 0x00, 1);

                if(led_scroll_pos == SK9822_LAST_LED)
                    led_scroll_pos = 0;
                else
                    led_scroll_pos++;

                ullLastScrollUpdate = g_ullSystemTick;
            }
            }
            break;
        case LED_PROGRESS: // TODO
        case LED_BREATHING: // TODO
        default:
            xLedStyle.mode = LED_OFF;
            break;
    }
}

void ui_task()
{
    // TODO

    led_task();
}

static void create_main_tab(lv_obj_t * parent)
{

}

static void create_data_tab(lv_obj_t * parent)
{
    /****************
     * CREATE A CHART
     ****************/
    lv_obj_t * chart = lv_chart_create(parent, NULL);                         /*Create the chart*/
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_obj_set_size(chart, lv_obj_get_width(parent) / 2, lv_obj_get_width(parent) / 4);   /*Set the size*/
    lv_obj_align(chart, NULL, LV_ALIGN_CENTER, 0, 0);                   /*Align below the slider*/
    lv_chart_set_series_width(chart, 3);                                            /*Set the line width*/

    /*Add a RED data series and set some points*/
    lv_chart_series_t * target = lv_chart_add_series(chart, LV_COLOR_RED);
    lv_chart_set_next(chart, target, 10);
    lv_chart_set_next(chart, target, 25);
    lv_chart_set_next(chart, target, 45);
    lv_chart_set_next(chart, target, 80);

    /*Add a BLUE data series and set some points*/
    lv_chart_series_t * temp = lv_chart_add_series(chart, lv_color_make(0x40, 0x70, 0xC0));
    lv_chart_set_next(chart, temp, 10);
    lv_chart_set_next(chart, temp, 25);
    lv_chart_set_next(chart, temp, 45);
    lv_chart_set_next(chart, temp, 80);
    lv_chart_set_next(chart, temp, 75);
    lv_chart_set_next(chart, temp, 505);
}

static void create_profile_tab(lv_obj_t * parent)
{

}

static void create_settings_tab(lv_obj_t * parent)
{

}

static void abort_popup_event(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        oven_clr_err();
    }
}

static void start_btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
    if(event == LV_EVENT_RELEASED)
    {
        oven_start();
    }
}

static void abort_btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
    if(event == LV_EVENT_PRESSED)
    {
        oven_abort(USER_ABORT);
    }
}