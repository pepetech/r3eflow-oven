#include "ui.h"

// func prototypes
void ui_abort_popup(ovenErr_t reason);

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
}

void ui_task()
{
    static ovenMode_t lastMode = ABORT;
    ovenMode_t currMode = oven_get_mode();

    if(lastMode != currMode)
    {
        switch(currMode)
        {
            case IDLE:
                ringled_set_style(STYLE_IDLE);
                break;
            case ABORT:
                //ringled_set_style(STYLE_ABORT);
                ui_abort_popup(oven_get_err());
                break;
            case PREHEAT_RAMP:
            case PREHEAT:
            case SOAK_RAMP:
            case SOAK:
            case REFLOW_RAMP:
            case REFLOW:
            case COOLDOWN:
                //ringled_set_style(STYLE_WORKING);
                break;
        }

        lastMode = currMode;
    }

    switch(currMode)
    {
        case IDLE:
        case ABORT:
        case PREHEAT_RAMP:
        case PREHEAT:
        case SOAK_RAMP:
        case SOAK:
        case REFLOW_RAMP:
        case REFLOW:
        case COOLDOWN:
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