#include "ringled.h"

typedef enum {
    LED_OFF,
    LED_STATIC,
    LED_BLINKING,
    LED_SCROLL,
    LED_PROGRESS,
    LED_BREATHING
} ringled_mode_t;

typedef union
{
    struct
    {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
    } ch;
    uint32_t full;
} ringled_color_t;

typedef struct
{
    ringled_mode_t mode;
    ringled_color_t color;
} ringled_style_t;

ringled_style_t ringledStyle;

void ringled_task()
{
    static uint8_t ringled_pos;
    static uint8_t ringled_rate;

    static ringled_color_t colorLookup[16];

    ringled_mode_t lastLedMode = LED_OFF;

    if (ringledStyle.mode != lastLedMode)
    {
        switch(ringledStyle.mode)
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
                    sk9822_set_color(i, 1, ringledStyle.color.ch.red, ringledStyle.color.ch.green, ringledStyle.color.ch.blue, 0);
                }
                sk9822_update();

                break;
            case LED_BLINKING: // TODO

                break;
            case LED_SCROLL:

                colorLookup[0].ch.red = ringledStyle.color.ch.red;
                colorLookup[0].ch.green = ringledStyle.color.ch.green;
                colorLookup[0].ch.blue = ringledStyle.color.ch.blue;

                colorLookup[1].ch.red = ringledStyle.color.ch.red * 0.5f;
                colorLookup[1].ch.green = ringledStyle.color.ch.green * 0.5f;
                colorLookup[1].ch.blue = ringledStyle.color.ch.blue * 0.5f;

                colorLookup[2].ch.red = ringledStyle.color.ch.red * 0.1f;
                colorLookup[2].ch.green = ringledStyle.color.ch.green * 0.1f;
                colorLookup[2].ch.blue = ringledStyle.color.ch.blue * 0.1f;

                ringled_pos = 0;
                ringled_rate = 100;

                break;
            case LED_BREATHING:

                colorLookup[0].ch.red = ringledStyle.color.ch.red;
                colorLookup[0].ch.green = ringledStyle.color.ch.green;
                colorLookup[0].ch.blue = ringledStyle.color.ch.blue;

                colorLookup[1].ch.red = ringledStyle.color.ch.red * 0.7f;
                colorLookup[1].ch.green = ringledStyle.color.ch.green * 0.7f;
                colorLookup[1].ch.blue = ringledStyle.color.ch.blue * 0.7f;

                colorLookup[2].ch.red = ringledStyle.color.ch.red * 0.5f;
                colorLookup[2].ch.green = ringledStyle.color.ch.green * 0.5f;
                colorLookup[2].ch.blue = ringledStyle.color.ch.blue * 0.5f;

                colorLookup[3].ch.red = ringledStyle.color.ch.red * 0.3f;
                colorLookup[3].ch.green = ringledStyle.color.ch.green * 0.3f;
                colorLookup[3].ch.blue = ringledStyle.color.ch.blue * 0.3f;

                colorLookup[4].ch.red = ringledStyle.color.ch.red * 0.1f;
                colorLookup[4].ch.green = ringledStyle.color.ch.green * 0.1f;
                colorLookup[4].ch.blue = ringledStyle.color.ch.blue * 0.1f;

                colorLookup[5].ch.red = 0x00;
                colorLookup[5].ch.green = 0x00;
                colorLookup[5].ch.blue = 0x00;

                ringled_pos = 0;
                ringled_rate = 200;

            case LED_PROGRESS: // TODO
            default:
                ringledStyle.mode = LED_OFF;
                break;
        }

        lastLedMode = ringledStyle.mode;
    }

    switch(ringledStyle.mode)
    {
        case LED_OFF:
        case LED_STATIC:
        case LED_BLINKING: // TODO
            break;
        case LED_SCROLL:
            {
            static uint64_t ullLastScrollUpdate = 0;

            if(g_ullSystemTick > (ullLastScrollUpdate + ringled_rate))
            {
                uint8_t leds[4];

                switch(ringled_pos)
                {
                    case 0:
                        leds[0] = ringled_pos;
                        leds[1] = SK9822_LAST_LED;
                        leds[2] = SK9822_LAST_LED - 1;
                        leds[3] = SK9822_LAST_LED - 2;
                        break;
                    case 1:
                        leds[0] = ringled_pos;
                        leds[1] = 0;
                        leds[2] = SK9822_LAST_LED;
                        leds[3] = SK9822_LAST_LED - 1;
                        break;
                    case 2:
                        leds[0] = ringled_pos;
                        leds[1] = 1;
                        leds[2] = 0;
                        leds[3] = SK9822_LAST_LED;
                        break;
                    default:
                        leds[0] = ringled_pos;
                        leds[1] = ringled_pos - 1;
                        leds[2] = ringled_pos - 2;
                        leds[3] = ringled_pos - 3;
                        break;
                }

                sk9822_set_color(leds[0], 0x01, colorLookup[0].ch.red, colorLookup[0].ch.green, colorLookup[0].ch.blue, 0);
                sk9822_set_color(leds[1], 0x01, colorLookup[1].ch.red, colorLookup[1].ch.green, colorLookup[1].ch.blue, 0);
                sk9822_set_color(leds[2], 0x01, colorLookup[2].ch.red, colorLookup[2].ch.green, colorLookup[2].ch.blue, 0);
                sk9822_set_color(leds[3], 0x00, 0x00, 0x00, 0x00, 1);

                if(ringled_pos == SK9822_LAST_LED)
                    ringled_pos = 0;
                else
                    ringled_pos++;

                ullLastScrollUpdate = g_ullSystemTick;
            }
            }
            break;
        case LED_BREATHING:
            {
            static uint64_t ullLastBreathUpdate = 0;

            if(g_ullSystemTick > (ullLastBreathUpdate + ringled_rate))
            {
                for (uint8_t i = 0; i < SK9822_NUM_LEDS; i++)
                {
                    sk9822_set_color(i, 0x01, colorLookup[ringled_pos].ch.red, colorLookup[ringled_pos].ch.green, colorLookup[ringled_pos].ch.blue, 0);
                }
                sk9822_update();

                int8_t direction = 1;

                ringled_pos += direction;

                if((ringled_pos == 0) && (direction < 0))
                    direction = 1;
                else if((ringled_pos == 4) && (direction > 0))
                    direction = -1;

                ullLastBreathUpdate = g_ullSystemTick;
            }
            }
            break;
        case LED_PROGRESS: // TODO
        default:
            ringledStyle.mode = LED_OFF;
            break;
    }
}

void ringled_set_style(ringled_style_presets_t ringledStylePreset)
{
    switch(ringledStylePreset)
    {
        case STYLE_ABORT:
            ringledStyle.color.full = 0x000000FF;
            ringledStyle.mode = LED_STATIC;
            break;
        case STYLE_IDLE:
            ringledStyle.color.full = 0x00FF0000;
            ringledStyle.mode = LED_BREATHING;
            break;
        case STYLE_WORKING:
            ringledStyle.color.full = 0x0000FF00;
            ringledStyle.mode = LED_SCROLL;
            break;
    }
}