#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <string.h>
#include <stdio.h>
#include "HAL/HAL_I2C.h"
#include "HAL/HAL_TMP006.h"
#include "weather.h"

/* GLOBAL VARIABLES. */

/* Stores graphic library context. */
Graphics_Context g_sContext;

/* Current state of the FSM initialized to display room values. */
State_t current_state = STATE_TEMP;

/* Stores button press events. */
Event_t event = EVENT_NONE;

/* FSM containing functions to be executed based on current_state variable. */
StateMachine_t fsm[] = { { STATE_TEMP, fn_TEMP }, { STATE_ROME, fn_ROME }, {
        STATE_MOSCOW, fn_MOSCOW },
                         { STATE_TOKYO, fn_TOKYO },
                         { STATE_NEWYORK, fn_NEWYORK } };

/* Array of structs that holds static city information. */
City_t cities[4];

/* External image declaration to display. The images are located in icons/. */
extern const Graphics_Image cloudy;
extern const Graphics_Image sunny;
extern const Graphics_Image snowy;
extern const Graphics_Image rainy;
extern const Graphics_Image home;

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
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    Graphics_clearDisplay(&g_sContext);
}

/* Stops WDT timer, enables interrupts, clocks , GPIO ports and I2C comunication for temperature reading. */
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

    /* Initialize I2C communication. */
    Init_I2C_GPIO();
    I2C_init();

    /* Initialize TMP006 temperature sensor. */
    TMP006_init();

}

/**
 *  Display room temperature using HAL abstraction level to get the temperature in
 *  Fahrenheit and then convert the value in Celsius to be printed.
 */
void display_temp()
{
    Graphics_clearDisplay(&g_sContext);

    char str[10];
    float temperature;

    temperature = TMP006_getTemp();
    temperature = (temperature - 32) / 1.8;

    sprintf(str, "%.1f", temperature);
    strcat(str, "C");

    /* Information display. */
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "CURRENT ROOM",
    AUTO_STRING_LENGTH,
                                64, 30, OPAQUE_TEXT);
    Graphics_drawImage(&g_sContext, &home, 52, 40);

    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Temperature:",
    AUTO_STRING_LENGTH,
                                64, 72, OPAQUE_TEXT);

    Graphics_drawStringCentered(&g_sContext, (int8_t*) str,
    AUTO_STRING_LENGTH,
                                64, 82, OPAQUE_TEXT);
}

/* Display weather information on the LCD screen corresponding to the city number. */
void display_weather(int city)
{
    Graphics_clearDisplay(&g_sContext);

    city--;

    /* If else statement to choose the correct icon based on weather condition. */
    if (strcmp(cities[city].weather_type, "Partly cloudy") == 0)
    {
        Graphics_drawImage(&g_sContext, &cloudy, 52, 40);
    }
    else if (strcmp(cities[city].weather_type, "Snowy") == 0)
    {
        Graphics_drawImage(&g_sContext, &snowy, 52, 40);
    }
    else if (strcmp(cities[city].weather_type, "Sunny") == 0)
    {
        Graphics_drawImage(&g_sContext, &sunny, 52, 40);
    }
    else if (strcmp(cities[city].weather_type, "Rainy") == 0)
    {
        Graphics_drawImage(&g_sContext, &rainy, 52, 40);
    }

    /* Information display. */
    Graphics_drawStringCentered(&g_sContext, (int8_t*) cities[city].name,
    AUTO_STRING_LENGTH,
                                64, 30, OPAQUE_TEXT);

    Graphics_drawStringCentered(&g_sContext, (int8_t*) cities[city].temperature,
    AUTO_STRING_LENGTH,
                                64, 72, OPAQUE_TEXT);

    Graphics_drawStringCentered(&g_sContext,
                                (int8_t*) cities[city].weather_type,
                                AUTO_STRING_LENGTH,
                                64, 82, OPAQUE_TEXT);

    Graphics_drawStringCentered(&g_sContext, (int8_t*) cities[city].humidity,
    AUTO_STRING_LENGTH,
                                64, 92, OPAQUE_TEXT);
}

/* Statically load data assuming Part 1 is completed. */
void _load_data()
{
    strcpy(cities[0].name, "ROME");
    strcpy(cities[0].temperature, "17.3C");
    strcpy(cities[0].weather_type, "Partly cloudy");
    strcpy(cities[0].humidity, "Hum: 63%");

    strcpy(cities[1].name, "MOSCOW");
    strcpy(cities[1].temperature, "-5.1C");
    strcpy(cities[1].weather_type, "Snowy");
    strcpy(cities[1].humidity, "Hum: 50%");

    strcpy(cities[2].name, "TOKYO");
    strcpy(cities[2].temperature, "12.5C");
    strcpy(cities[2].weather_type, "Sunny");
    strcpy(cities[2].humidity, "Hum: 70%");

    strcpy(cities[3].name, "NEW YORK");
    strcpy(cities[3].temperature, "6.3C");
    strcpy(cities[3].weather_type, "Rainy");
    strcpy(cities[3].humidity, "Hum: 91%");
}

/* FSM FUNCTIONS. */

/**
 *  Functions called when an event occurs. Each time a button is pressed,
 *  the board exits the low power mode loop and the ISR handler modifies
 *  the event corresponding to the button press. fn_X function is then
 *  called and based on the event received, displays the correct weather
 *  information and alters the current state of the FSM.
 *  Note that: Your room temp = 0, Rome = 1, Moscow = 2, Tokyo = 3, New York = 4
 */
void fn_TEMP()
{
    if (event == BUTTON1_PRESSED)
    {
        display_weather(1);
        current_state = STATE_ROME;
    }
    else if (event == BUTTON2_PRESSED)
    {
        display_weather(4);
        current_state = STATE_NEWYORK;
    }
}

void fn_ROME()
{
    if (event == BUTTON1_PRESSED)
    {
        display_weather(2);
        current_state = STATE_MOSCOW;
    }
    else if (event == BUTTON2_PRESSED)
    {
        display_temp();
        current_state = STATE_TEMP;
    }
}

void fn_MOSCOW()
{
    if (event == BUTTON1_PRESSED)
    {
        display_weather(3);
        current_state = STATE_TOKYO;
    }
    else if (event == BUTTON2_PRESSED)
    {
        display_weather(1);
        current_state = STATE_ROME;
    }
}

void fn_TOKYO()
{
    if (event == BUTTON1_PRESSED)
    {
        display_weather(4);
        current_state = STATE_NEWYORK;
    }
    else if (event == BUTTON2_PRESSED)
    {
        display_weather(2);
        current_state = STATE_MOSCOW;
    }
}

void fn_NEWYORK()
{
    if (event == BUTTON1_PRESSED)
    {
        display_temp();
        current_state = STATE_TEMP;
    }
    else if (event == BUTTON2_PRESSED)
    {
        display_weather(3);
        current_state = STATE_TOKYO;
    }
}

/* MAIN FUNCTION */
int main(void)
{
    _hwInit();
    _graphicsInit();
    _load_data();

    /* Display room temperature as default. */
    display_temp();

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
