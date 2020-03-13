#include "ui.h"

// vars
uint16_t touchSoundFrequency = 2000;
uint16_t touchSoundDuration = 50;
uint8_t touchSoundEnabled = 1;

lv_obj_t* mainTabStartAbortBtn;
lv_obj_t* mainTabOvenTempLabel;
lv_obj_t* mainTabOvenTargetTempLabel;
lv_obj_t* mainTabOvenModeLabel;

lv_obj_t* dataTabChart;
lv_chart_series_t* dataTabChartTemp;
lv_chart_series_t* dataTabChartTarget;


// func prototypes
void ui_abort_popup(ovenErr_t reason);

static void create_main_tab(lv_obj_t* parent);
static void create_data_tab(lv_obj_t* parent);
static void create_profile_tab(lv_obj_t* parent);
static void create_settings_tab(lv_obj_t* parent);

static void abort_popup_event(lv_obj_t* obj, lv_event_t event);
static void start_btn_event_cb(lv_obj_t* btn, lv_event_t event);
static void abort_btn_event_cb(lv_obj_t* btn, lv_event_t event);

void ui_init()
{
    lv_theme_t* th = lv_theme_material_init(210, NULL);
    lv_theme_set_current(th);
    //th = lv_theme_get_current();    /*If `LV_THEME_LIVE_UPDATE  1` `th` is not used directly so get the real theme after set*/

    /********************
     * CREATE A SCREEN
     *******************/
    /* Create a new screen and load it */
    lv_obj_t* scr = lv_cont_create(NULL, NULL);
    lv_disp_load_scr(scr);

    lv_obj_t* tv = lv_tabview_create(scr, NULL);
    lv_obj_set_size(tv, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_t* mainTab = lv_tabview_add_tab(tv, "Main");
    lv_obj_t* dataTab = lv_tabview_add_tab(tv, "Data");
    lv_obj_t* profileTab = lv_tabview_add_tab(tv, "Profile");
    lv_obj_t* settingsTab = lv_tabview_add_tab(tv, "Settings");

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
                lv_obj_set_event_cb(mainTabStartAbortBtn, start_btn_event_cb);
                lv_label_set_text(lv_obj_get_child(mainTabStartAbortBtn, NULL), "Start");
                lv_label_set_text(mainTabOvenModeLabel, "idle");
                ringled_set_style(STYLE_IDLE);
                break;
            case ABORT:
                ringled_set_style(STYLE_ABORT);
                lv_label_set_text(mainTabOvenModeLabel, "abort");
                ui_abort_popup(oven_get_err());
                break;
            case PREHEAT_RAMP:
                ringled_set_style(STYLE_WORKING);
                lv_label_set_text(mainTabOvenModeLabel, "Preheat ramp");
                break;
            case PREHEAT:
                ringled_set_style(STYLE_WORKING);
                lv_label_set_text(mainTabOvenModeLabel, "preheat");
                break;
            case SOAK_RAMP:
                ringled_set_style(STYLE_WORKING);
                lv_label_set_text(mainTabOvenModeLabel, "soak ramp");
                break;
            case SOAK:
                ringled_set_style(STYLE_WORKING);
                lv_label_set_text(mainTabOvenModeLabel, "soak");
                break;
            case REFLOW_RAMP:
                ringled_set_style(STYLE_WORKING);
                lv_label_set_text(mainTabOvenModeLabel, "reflow ramp");
                break;
            case REFLOW:
                ringled_set_style(STYLE_WORKING);
                lv_label_set_text(mainTabOvenModeLabel, "reflow");
                break;
            case COOLDOWN:
                ringled_set_style(STYLE_WORKING);
                lv_label_set_text(mainTabOvenModeLabel, "cooldown");
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

    static uint64_t lastDataUpdate = 0;
    if(g_ullSystemTick > (lastDataUpdate + 200))
    {
        lv_label_set_text_fmt(mainTabOvenTempLabel, "%.2fC", oven_get_temperature());
        lv_label_set_text_fmt(mainTabOvenTargetTempLabel, "%.2fC", oven_get_target_temperature());

        lastDataUpdate = g_ullSystemTick;
    }

    static uint64_t lastChartUpdate = 0;
    if(g_ullSystemTick > (lastChartUpdate + 1000))
    {
        lv_chart_set_next(dataTabChart, dataTabChartTemp, oven_get_temperature());
        lv_chart_set_next(dataTabChart, dataTabChartTarget, oven_get_target_temperature());

        lastChartUpdate = g_ullSystemTick;
    }
}

void ui_abort_popup(ovenErr_t reason)
{
    lv_obj_t* abrtPopup = lv_mbox_create(lv_scr_act(), NULL);
    lv_mbox_set_text(abrtPopup, ovenERR_str[reason]);
    lv_obj_set_width(abrtPopup, 200);
    static const char* btns[] = {"Ok", ""};
    lv_mbox_add_btns(abrtPopup, btns);
    lv_obj_set_event_cb(abrtPopup, abort_popup_event);
    lv_obj_align(abrtPopup, NULL, LV_ALIGN_CENTER, 0, 0);
}

static void create_main_tab(lv_obj_t * parent)
{
    lv_obj_t* ovenTempLabel = lv_label_create(parent, NULL);
    lv_obj_align(ovenTempLabel, NULL, LV_ALIGN_IN_TOP_LEFT, 60, 20);
    lv_label_set_text(ovenTempLabel, "Temperature");

    mainTabOvenTempLabel = lv_label_create(parent, NULL);
    lv_obj_align(mainTabOvenTempLabel, ovenTempLabel, LV_ALIGN_CENTER, 0, 30);

    lv_obj_t* ovenTargetTempLabel = lv_label_create(parent, NULL);
    lv_obj_align(ovenTargetTempLabel, NULL, LV_ALIGN_IN_TOP_RIGHT, -60, 20);
    lv_label_set_text(ovenTargetTempLabel, "Target");

    mainTabOvenTargetTempLabel = lv_label_create(parent, NULL);
    lv_obj_align(mainTabOvenTargetTempLabel, ovenTargetTempLabel, LV_ALIGN_CENTER, 0, 30);

    mainTabOvenModeLabel = lv_label_create(parent, NULL);
    lv_obj_align(mainTabOvenModeLabel, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);

    mainTabStartAbortBtn = lv_btn_create(parent, NULL);
    lv_obj_align(mainTabStartAbortBtn, NULL, LV_ALIGN_IN_TOP_MID, 0, 100);

    lv_obj_t* startBtnLabel = lv_label_create(mainTabStartAbortBtn, NULL);
    lv_label_set_text(startBtnLabel, "-");
}

static void create_data_tab(lv_obj_t * parent)
{
    /****************
     * CREATE A CHART
     ****************/
    dataTabChart = lv_chart_create(parent, NULL);
    lv_chart_set_type(dataTabChart, LV_CHART_TYPE_LINE);
    lv_chart_set_update_mode(dataTabChart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_point_count(dataTabChart, 60);
    lv_chart_set_range(dataTabChart, 21, 250);
    lv_chart_set_series_width(dataTabChart, 2);
    lv_obj_set_size(dataTabChart, 375, 180);
    lv_chart_set_series_opa(dataTabChart, LV_OPA_70);
    lv_chart_set_margin(dataTabChart, 75);
    lv_obj_align(dataTabChart, NULL, LV_ALIGN_CENTER, 25, -20);
    lv_chart_set_x_tick_texts(dataTabChart, "t-60\nt-50\nt-40\nt-30\nt-20\nt-10\nt[s]", 2, 0);
    lv_chart_set_y_tick_texts(dataTabChart, "250\n193\n135\n78\nT[C]", 2, 0);

    dataTabChartTemp = lv_chart_add_series(dataTabChart, LV_COLOR_RED);
    dataTabChartTarget = lv_chart_add_series(dataTabChart, LV_COLOR_GREEN);
}

static void create_profile_tab(lv_obj_t * parent)
{

}

static void create_settings_tab(lv_obj_t * parent)
{

}

static void abort_popup_event(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED)
    {
        if(touchSoundEnabled) sound_play(touchSoundFrequency, touchSoundDuration);
        oven_clr_err();
        lv_mbox_start_auto_close(obj, 0);
    }
}

static void start_btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
    if(event == LV_EVENT_RELEASED)
    {
        if(touchSoundEnabled) sound_play(touchSoundFrequency, touchSoundDuration);
        lv_obj_set_event_cb(mainTabStartAbortBtn, abort_btn_event_cb);
        lv_label_set_text(lv_obj_get_child(mainTabStartAbortBtn, NULL), "Abort");
        oven_start();
    }
}

static void abort_btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
    if(event == LV_EVENT_PRESSED)
    {
        if(touchSoundEnabled) sound_play(touchSoundFrequency, touchSoundDuration);
        oven_abort(USER_ABORT);
    }
}