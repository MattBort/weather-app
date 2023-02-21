/**
 * Defines the structures and function to handle the FSM
 * to cycle through each cities correctly keeping the same order
 * on button presses and executing functions based on the states
 * of the machine.
 */
#ifndef __WEATHER_H__
#define __WEATHER_H__

/**
 * Defines the different states.
 * 
 * STATE_INIT -> default state for startup.
 * STATE_X -> one state for each city.
 *
 */
typedef enum
{
    STATE_ROME, STATE_MOSCOU, STATE_NEWYORK, STATE_TOKYO, STATE_NUM
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
 * or for when no event occurred.
 */
typedef enum
{
    EVENT_NONE, BUTTON1_PRESSED, BUTTON2_PRESSED
} Event_t;

/**
 * Executed function called based on the current
 * state and events. Rome is the default city
 * after hardware initialization.
 */
void fn_ROME(void);
void fn_MOSCOU(void);
void fn_NEWYORK(void);
void fn_TOKYO(void);

/**
 * Updates the LCD display with the correct
 * city's information.
 */
void display_weather(int);

#endif
