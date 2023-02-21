#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <stdio.h>
#include "weather.h"

/* GLOBAL VARIABLES. */

/* Stores graphic library context. */
Graphics_Context g_sContext;

/* Current state of the FSM initialized to display Rome values. */
State_t current_state = STATE_ROME;

/* Stores button press events. */
Event_t event = EVENT_NONE;

/* FSM containing functions to be executed based on current_state variable. */
StateMachine_t fsm[] = { { STATE_ROME, fn_ROME }, { STATE_MOSCOU, fn_MOSCOU }, {
        STATE_NEWYORK, fn_NEWYORK },
                         { STATE_TOKYO, fn_TOKYO } };

//TODO
void display_weather(int city)
{
    switch (city)
    {
    case 1:
        Graphics_clearDisplay(&g_sContext);
        Graphics_drawStringCentered(&g_sContext, (int8_t*) "1",
        AUTO_STRING_LENGTH,
                                    64, 30, OPAQUE_TEXT);
        break;
    case 2:
        Graphics_clearDisplay(&g_sContext);
        Graphics_drawStringCentered(&g_sContext, (int8_t*) "2",
        AUTO_STRING_LENGTH,
                                    64, 30, OPAQUE_TEXT);
        break;
    case 3:
        Graphics_clearDisplay(&g_sContext);
        Graphics_drawStringCentered(&g_sContext, (int8_t*) "3",
        AUTO_STRING_LENGTH,
                                    64, 30, OPAQUE_TEXT);
        break;
    case 4:
        Graphics_clearDisplay(&g_sContext);
        Graphics_drawStringCentered(&g_sContext, (int8_t*) "4",
        AUTO_STRING_LENGTH,
                                    64, 30, OPAQUE_TEXT);
        break;
    }
}

/* FSM FUNCTIONS. */

/**
 *  Functions called when an event occurs. Each time a button is pressed,
 *  the board exits the low power mode loop and the ISR handler modifies
 *  the event corresponding to the button press. fn_X function is then
 *  called and based on the event received, displays the correct weather
 *  information and alters the current state of the FSM.
 */
void fn_ROME()
{
    if (event == BUTTON1_PRESSED)
    {
        display_weather(2);
        current_state = STATE_MOSCOU;
    }
    else if (event == BUTTON2_PRESSED)
    {
        display_weather(4);
        current_state = STATE_TOKYO;
    }
}

void fn_MOSCOU()
{
    if (event == BUTTON1_PRESSED)
    {
        display_weather(3);
        current_state = STATE_NEWYORK;
    }
    else if (event == BUTTON2_PRESSED)
    {
        display_weather(1);
        current_state = STATE_ROME;
    }
}

void fn_NEWYORK()
{
    if (event == BUTTON1_PRESSED)
    {
        display_weather(4);
        current_state = STATE_TOKYO;
    }
    else if (event == BUTTON2_PRESSED)
    {
        display_weather(2);
        current_state = STATE_MOSCOU;
    }
}

void fn_TOKYO()
{
    if (event == BUTTON1_PRESSED)
    {
        display_weather(1);
        current_state = STATE_ROME;
    }
    else if (event == BUTTON2_PRESSED)
    {
        display_weather(3);
        current_state = STATE_NEWYORK;
    }
}

/* Initialize graphics settings for the LCD screen. */
void _graphicsInit()
{
    /* Initializes display. */
    Crystalfontz128x128_Init();

    /* Set default screen orientation. */
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    /* Initializes graphics context and background/text color. */
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128,
                         &g_sCrystalfontz128x128_funcs);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_CHARTREUSE);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    Graphics_clearDisplay(&g_sContext);
}

void _hwInit()
{
    /* Halting WDT and enabling master interrupts. */
    WDT_A_holdTimer();
    Interrupt_enableMaster();

    /* LCD related settings. */
    PCM_setCoreVoltageLevel(PCM_VCORE1);
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Initializes Clock System. */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    /* Define buttons as input. */
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3, GPIO_PIN5);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN1);

    /* Clear the interrupt flags to avoid instant interrupt handling. */
    GPIO_clearInterruptFlag(GPIO_PORT_P3, GPIO_PIN5);
    GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN1);

    /* Enable interrupt on port and pin. */
    GPIO_enableInterrupt(GPIO_PORT_P3, GPIO_PIN5);
    GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN1);

    /* Enable interrupt on ports. */
    Interrupt_enableInterrupt(INT_PORT3);
    Interrupt_enableInterrupt(INT_PORT5);
}

/* MAIN FUNCTION */
int main(void)
{
    _hwInit();
    _graphicsInit();

    //STATE ROME INIT
    //MODIFY
    display_weather(1);

    while (1)
    {
        PCM_gotoLPM0();

        if (current_state < STATE_NUM)
        {
            /* Executes current state function after low power mode is interrupted. */
            (*fsm[current_state].state_function)();
        }
    }
}

/* Port 5 handler for the top button switch on the boosterpack. Pin:33 -> Port:5 Pin:1 */
void PORT5_IRQHandler(void)
{
    if ((GPIO_getEnabledInterruptStatus(GPIO_PORT_P5) & GPIO_PIN1))
    {
        /* Clear interrupt flag (to clear pending interrupt indicator. */
        GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN1);

        /* Set button pressed event to alter the current state of the FSM. */
        event = BUTTON1_PRESSED;
    }
}

/* Port 3 handler for the bottom button switch on the boosterpack. Pin:32 -> Port:3 Pin:5 */
void PORT3_IRQHandler(void)
{
    if ((GPIO_getEnabledInterruptStatus(GPIO_PORT_P3) & GPIO_PIN5))
    {
        GPIO_clearInterruptFlag(GPIO_PORT_P3, GPIO_PIN5);
        event = BUTTON2_PRESSED;
    }
}