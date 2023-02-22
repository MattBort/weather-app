/**
 * Defines the structures and function to handle the FSM
 * to cycle through each cities correctly keeping the same order
 * on button presses and executing functions based on the states
 * of the machine.
 */
#ifndef __WEATHER_H__
#define __WEATHER_H__

/* Constants for string length for city data. */
#define NAME_LENGTH 50
#define TEMP_LENGTH 7
#define WEATHER_LENGTH 100
#define HUMIDITY_LENGTH 20

/**
 * Defines the different states.
 * 
 * STATE_TEMP -> default state to display room temperature.
 * STATE_X -> one state for each city.
 * STATE_NUM -> used for control purposes.
 *
 */
typedef enum
{
    STATE_TEMP, STATE_ROME, STATE_MOSCOW, STATE_TOKYO, STATE_NEWYORK, STATE_NUM
} State_t;

/**
 * Defines the state machine struct.
 *
 * state -> State_t enum corresponding to the current city.
 * void (*state_function) -> function to be called based on current state.
 */
typedef struct
{
    State_t state;
    void (*state_function)(void);
} StateMachine_t;

/**
 * Defines the enum for button presses
 * or none as event.
 */
typedef enum
{
    EVENT_NONE, BUTTON1_PRESSED, BUTTON2_PRESSED
} Event_t;

/**
 * Struct that holds information for a specific city.
 */
typedef struct
{
    char name[NAME_LENGTH];
    char temperature[TEMP_LENGTH];
    char weather_type[WEATHER_LENGTH];
    char humidity[HUMIDITY_LENGTH];
} City_t;

/**
 * Executed function called based on the current
 * state and events. TEMP is the default displayed
 * data after hardware initialization.
 */
void fn_TEMP(void);
void fn_ROME(void);
void fn_MOSCOW(void);
void fn_TOKYO(void);
void fn_NEWYORK(void);

/**
 * Updates the LCD display with the correct
 * city's or room information.
 */
void display_temp();
void display_weather(int);

#endif
