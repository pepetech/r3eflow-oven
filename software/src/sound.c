#include "sound.h"

typedef struct sound_queue_element sound_queue_element_t;

struct sound_queue_element {
    uint16_t frequency;
    uint32_t duration;
    sound_queue_element_t* next_element;
};

static sound_queue_element_t* soundQueueHead = NULL;

void _timer2_isr()
{
    uint32_t flags = TIMER2->IFC;

    if(flags & TIMER_IF_OF)
    {
        if(soundQueueHead)
        {
            if(soundQueueHead->frequency)
            {
                TIMER0->CC[0].CTRL |= TIMER_CC_CTRL_MODE_OUTPUTCOMPARE;
                TIMER0->TOP = (HFPERB_CLOCK_FREQ / (soundQueueHead->frequency << 1)) - 1; // Double the frequency
                TIMER0->CMD = TIMER_CMD_START;
            }
            else
            {
                TIMER0->CC[0].CTRL &= ~_TIMER_CC_CTRL_MODE_MASK;
            }

            TIMER2->TOP = (soundQueueHead->duration * (HFPER_CLOCK_FREQ / 1024 / 1000));
            TIMER2->CMD = TIMER_CMD_START;

            sound_queue_element_t* ptr = soundQueueHead->next_element;
            free(soundQueueHead);
            soundQueueHead = ptr;
        }
        else
        {
            TIMER0->CMD = TIMER_CMD_STOP;

            return;
        }
    }
}

void sound_init()
{
    // timer 0 - generate oscilattion
    CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_TIMER0;

    TIMER0->CTRL = TIMER_CTRL_RSSCOIST | TIMER_CTRL_PRESC_DIV1 | TIMER_CTRL_CLKSEL_PRESCHFPERCLK | TIMER_CTRL_FALLA_NONE | TIMER_CTRL_RISEA_NONE | TIMER_CTRL_MODE_UP;
    TIMER0->CNT = 0x0000;

    TIMER0->CC[0].CTRL = TIMER_CC_CTRL_PRSCONF_LEVEL | TIMER_CC_CTRL_CUFOA_NONE | TIMER_CC_CTRL_COFOA_TOGGLE | TIMER_CC_CTRL_CMOA_NONE | TIMER_CC_CTRL_MODE_OUTPUTCOMPARE;

    TIMER0->ROUTELOC0 = TIMER_ROUTELOC0_CC0LOC_LOC6;
    TIMER0->ROUTEPEN |= TIMER_ROUTEPEN_CC0PEN;

    // timer 2 - control duration
    CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_TIMER2;

    TIMER2->CTRL = TIMER_CTRL_RSSCOIST | TIMER_CTRL_PRESC_DIV1024 | TIMER_CTRL_CLKSEL_PRESCHFPERCLK | TIMER_CTRL_OSMEN | TIMER_CTRL_MODE_UP;
    TIMER2->CNT = 0x0000;

    TIMER2->IFC = _TIMER_IFC_MASK;
    IRQ_CLEAR(TIMER2_IRQn); // Clear pending vector
    IRQ_SET_PRIO(TIMER2_IRQn, 0, 0); // Set priority 0,0 (max)
    IRQ_ENABLE(TIMER2_IRQn); // Enable vector
    TIMER2->IEN = TIMER_IEN_OF;
}

void sound_play(uint16_t frequency, uint32_t duration)
{
    if(TIMER2->STATUS && TIMER_STATUS_RUNNING)
    {
        sound_queue_add(frequency, duration);
    }
    else
    {
        if(frequency)
        {
            TIMER0->CC[0].CTRL |= TIMER_CC_CTRL_MODE_OUTPUTCOMPARE;
            TIMER0->TOP = (HFPERB_CLOCK_FREQ / (frequency << 1)) - 1; // Double the frequency
            TIMER0->CMD = TIMER_CMD_START;
        }
        else
        {
            TIMER0->CC[0].CTRL &= ~_TIMER_CC_CTRL_MODE_MASK;
        }

        TIMER2->TOP = (duration * (HFPER_CLOCK_FREQ / 1024 / 1000));
        TIMER2->CMD = TIMER_CMD_START;
    }
}

void sound_queue_add(uint16_t frequency, uint32_t duration)
{
    sound_queue_element_t* element = (sound_queue_element_t*)malloc(sizeof(sound_queue_element_t));

    element->frequency = frequency;
    element->duration = duration;
    element->next_element = NULL;

    if(soundQueueHead)
    {
        sound_queue_element_t* ptr = soundQueueHead;
        while(ptr->next_element)
        {
            ptr = ptr->next_element;
        }

        ptr->next_element = element;
    }
    else
    {
        soundQueueHead = element;
    }
}