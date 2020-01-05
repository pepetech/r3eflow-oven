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
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    } ch;
    uint32_t full;
} ringled_color_t;

typedef struct
{
    ringled_mode_t mode;
    ringled_color_t color;
} ringled_style_t;

#define LED_COLOR_MAKE(r, g, b) ((ringled_color_t){{r, g, b}})

#define LED_COLOR_WHITE LED_COLOR_MAKE(0xFF, 0xFF, 0xFF)
#define LED_COLOR_SILVER LED_COLOR_MAKE(0xC0, 0xC0, 0xC0)
#define LED_COLOR_GRAY LED_COLOR_MAKE(0x80, 0x80, 0x80)
#define LED_COLOR_BLACK LED_COLOR_MAKE(0x00, 0x00, 0x00)
#define LED_COLOR_RED LED_COLOR_MAKE(0xFF, 0x00, 0x00)
#define LED_COLOR_MAROON LED_COLOR_MAKE(0x80, 0x00, 0x00)
#define LED_COLOR_YELLOW LED_COLOR_MAKE(0xFF, 0xFF, 0x00)
#define LED_COLOR_OLIVE LED_COLOR_MAKE(0x80, 0x80, 0x00)
#define LED_COLOR_LIME LED_COLOR_MAKE(0x00, 0xFF, 0x00)
#define LED_COLOR_GREEN LED_COLOR_MAKE(0x00, 0x80, 0x00)
#define LED_COLOR_CYAN LED_COLOR_MAKE(0x00, 0xFF, 0xFF)
#define LED_COLOR_AQUA LED_COLOR_CYAN
#define LED_COLOR_TEAL LED_COLOR_MAKE(0x00, 0x80, 0x80)
#define LED_COLOR_BLUE LED_COLOR_MAKE(0x00, 0x00, 0xFF)
#define LED_COLOR_NAVY LED_COLOR_MAKE(0x00, 0x00, 0x80)
#define LED_COLOR_MAGENTA LED_COLOR_MAKE(0xFF, 0x00, 0xFF)
#define LED_COLOR_PURPLE LED_COLOR_MAKE(0x80, 0x00, 0x80)
#define LED_COLOR_ORANGE LED_COLOR_MAKE(0xFF, 0xA5, 0x00)


ringled_style_t ringledStyle;

void ringled_task()
{
    const float coeficients[8] = {1.f, 0.9f, 0.8f, 0.7f, 0.5f, 0.3f, 0.1f, 0.0f};
    
    static uint8_t ringled_pos;
    static uint8_t ringled_rate;

    static ringled_color_t colorLookup[16];

    static ringled_mode_t lastLedMode = LED_OFF;

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

                colorLookup[1].ch.red = ringledStyle.color.ch.red * coeficients[4];
                colorLookup[1].ch.green = ringledStyle.color.ch.green * coeficients[4];
                colorLookup[1].ch.blue = ringledStyle.color.ch.blue * coeficients[4];

                colorLookup[2].ch.red = ringledStyle.color.ch.red * coeficients[6];
                colorLookup[2].ch.green = ringledStyle.color.ch.green * coeficients[6];
                colorLookup[2].ch.blue = ringledStyle.color.ch.blue * coeficients[6];

                ringled_pos = 0;
                ringled_rate = 100;

                break;
            case LED_BREATHING:

                for (uint8_t i = 0; i < 8; i++)
                {
                    colorLookup[i].ch.red = ringledStyle.color.ch.red * coeficients[i];
                    colorLookup[i].ch.green = ringledStyle.color.ch.green * coeficients[i];
                    colorLookup[i].ch.blue = ringledStyle.color.ch.blue * coeficients[i];
                }

                ringled_pos = 0;
                ringled_rate = 200;

                break;
            case LED_PROGRESS: // TODO
            default:
                ringledStyle.mode = LED_OFF;
                break;
        }

        lastLedMode = ringledStyle.mode;
    }

    static uint64_t ullLastScrollUpdate = 0;
    static uint64_t ullLastBreathUpdate = 0;

    switch(ringledStyle.mode)
    {
        case LED_OFF:
        case LED_STATIC:
        case LED_BLINKING: // TODO
            break;
        case LED_SCROLL:
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
            break;
        case LED_BREATHING:
            if(g_ullSystemTick > (ullLastBreathUpdate + ringled_rate))
            {
                for (uint8_t i = 0; i < SK9822_NUM_LEDS; i++)
                {
                    sk9822_set_color(i, 0x01, colorLookup[ringled_pos].ch.red, colorLookup[ringled_pos].ch.green, colorLookup[ringled_pos].ch.blue, 0);
                }
                sk9822_update();

                static int8_t direction = 1;

                if((ringled_pos == 0) && (direction < 0))
                {
                    direction = 1;
                    ringled_pos += direction;
                }
                else if((ringled_pos == 7) && (direction > 0))
                {
                    direction = -1;
                    ringled_pos += direction;
                }
                else
                {
                    ringled_pos += direction;
                }
                
                    

                ullLastBreathUpdate = g_ullSystemTick;
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
            ringledStyle.color = LED_COLOR_RED;
            ringledStyle.mode = LED_STATIC;
            break;
        case STYLE_IDLE:
            ringledStyle.color = LED_COLOR_CYAN;
            ringledStyle.mode = LED_BREATHING;
            break;
        case STYLE_WORKING:
            ringledStyle.color = LED_COLOR_YELLOW;
            ringledStyle.mode = LED_SCROLL;
            break;
    }
}