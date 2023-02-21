# Weather APP
This application displays weather information of different cities via the MSP432P401R. Weather type, temperature and humidity is fetched through an API using the CC3100 WiFi module and then displayed on the LCD screen of the BoosterPack MKII.
## Hardware requirements
| Module | Usage | Description |
| :- | :- | :- |
| MSP432P401R | Data elaboration | Microcontroller used to elaborate and display the received data. |
| CC3100Boost | API request and response handling | Used to connect to a WiFi network and creates a request to an online API to retrieve data about weather. |
| BoosterPack MKII      | LCD display and pushbuttons | The LCD display is used to display data and the 2 pushbuttons to navigate through the different cities avaiable. |

## Software requirements
Code Composer Studio(CCS) 10.1.0 IDE is used to code, compile, debug and burn into the MSP432P401R. Texas Instruments driverlib is used for higher code abstraction and the simplelink library for the WiFi communication part.
## Project structure
The `wifi-part1` folder contains the code regarding the retrival part of the information. It uses the MSP432P401R combined with the CC3100Boost module to connect and fetch the data. The `lcd-part2` folder contains the LCD and pusbuttons code to display the information.

```text
weather app  
├── wifi-part1
│   ├── board
|   |   ├── board.c [enables CC3100]
|   |   └── board.h [board.c header file]
│   ├── cli_uart
|   |   ├── cli_uart.c [cli used for debugging]
|   |   └── cli_uart.h [cli_uart.c header file]
│   ├── drivelib  [TI library folder for the MSP432]
│   ├── simplelink  [CC3100 general library folder]
│   ├── spi_cc3100
|   |   ├── spi_cc3100.c [serial pheriferal interface functions for the CC3100]
|   |   └── spi_cc3100.h [spi_cc3100.c header file]
│   ├── uart_cc3100
|   |   ├── uart_cc3100.c [uart communication functions for the CC3100]
|   |   └── uart_cc3100.h [uart_cc3100.c header file]
│   ├── .cproject  [copy of include settings for CCS project building]
│   ├── main  [main C file]
│   ├── msp432p401r.cmd  [CCS generated file to setup the MSP432]
│   ├── sl_common.h  [library that provides string manipulation and wifi related functions/variables]
│   ├── startup_msp432p401r_css.c  [CCS startup code for MSP432 interrupts]
│   └── system_msp432p401r.c  [MSP432 system initialization]
└──lcd-part2
   ├── LcdDriver  [driver for lcd screen usage]
   ├── main.c  [main C file]
   ├── msp432p401r.cmd 
   ├── startup_msp432p401r_css.c 
   ├── system_msp432p401r.c  
   └── weather.h  [custom header file containing data structures and functions used in main file]
   
```

## Build, burn and run
1 Create 2 blank MSP432P401R projects in CCS, one for each of the 2 parts.

2 Replace every file in the project folders. 

3a Part 1: also replace .cproject file inside the project to include dependecies.

3b Part 2: add simplelink_msp432p4_sdk_3_40_01_02/source" directory to "Add dir to #include search path" window in CCS Build->ARM Compiler->Include options in project properties. Then add simplelink_msp432p4_sdk_3_40_01_02/source/ti/devices/msp432p4xx/driverlib/ccs/msp432p4xx_driverlib.lib and ../source/ti/grlib/lib/css/m4f/grlib.a to "Include library file..." in CCS Build->ARM linker->File Search Path.

4 Build the project and start debugging with the MSP432 plugged in to run the app.

# Part 1: code analysis
This part of the project allows comunication with an API through a WiFi connection. We used our personal WiFi router to connect to the internet and send a request. The credentials are defined as follows:

```c
    #define SSID_NAME "YOUR_AP_SSID"
    #define PASSKEY "YOUR_WIFI_PASSWORD"   
```
Then the request, response information(server and GET request) and lenght is defined. We used a mock API to test our application.

```c
    #define MOCK_SERVER  "cctest.free.beeceptor.com"

    #define PREFIX_BUFFER   "GET /my/api/path"
    #define POST_BUFFER     " HTTP/1.1\r\nHost:cctest.free.beeceptor.com\r\nAccept: */"
    #define POST_BUFFER2    "*\r\n\r\n"

    #define SMALL_BUF           32 //For the hostname
    #define MAX_SEND_BUF_SIZE   512
    #define MAX_SEND_RCV_SIZE   300
```
The necessary variables for the communication are instantiated with a struct:

```c
    struct
    {
        _u8 Recvbuff[MAX_SEND_RCV_SIZE];
        _u8 SendBuff[MAX_SEND_BUF_SIZE];
        _u8 HostName[SMALL_BUF];
        _u32 DestinationIP;
        _i16 SockID;
    } g_AppData;
```

A connection with the WiFi is then created if the credentials are correct; the application receives data by creating the request with the values already defined and communicating with UDP sockets. The response from the server is saved in the `Recvbuff` via the `getResponse` function. Note that the response is received if each step of the communication is succesful, otherwise the application loops indefinitely and gives errors.

Afterwards, the microcontroller is disconnected from the WiFi `disconnectFromAP` and the execution ends. For part 2 we assume a correct JSON response has been sent to display information.

# Part 2: code analysis
In the second part the LCD screen and pushbuttons are used to display the data using an FSM. Each state is triggered by the next/previous pushbuttons of the BoosterPack MKII. We defined 4 states(one for each city) with the correspondent functions in the `weather.h` header file and the button press events that are used to trigger the corresponding interrupt to display the city.

In the main file we instantiate the FSM and the `current_state` and `event` variable. The current state is set to the first city to be displayed which is Rome and the event to None:

```c
    State_t current_state = STATE_ROME;
    Event_t event = EVENT_NONE;
    StateMachine_t fsm[] = { { STATE_ROME, fn_ROME }, { STATE_MOSCOU, fn_MOSCOU }, {STATE_NEWYORK, fn_NEWYORK }, { STATE_TOKYO, fn_TOKYO } };
```
In the main function, the hardware, graphics, internal clocks and interrupts are initialized via the `_hwInit` and `_graphicsInit` functions and the first city is displayed. The microcontrollers then goes into low power mode and waits for an interrupt.

The interrupts are enabled in the hardware initialization function as follow:
```c
    GPIO_clearInterruptFlag(GPIO_PORT_P3, GPIO_PIN5);
    GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN1);

    GPIO_enableInterrupt(GPIO_PORT_P3, GPIO_PIN5);
    GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN1);

    Interrupt_enableInterrupt(INT_PORT3);
    Interrupt_enableInterrupt(INT_PORT5);
```
Note that the flag is cleared to avoid a wrongful trigger before a button is pressed. `GPIO_PORT_P3, GPIO_PIN5` refers the the top pushbutton and `GPIO_PORT_P5, GPIO_PIN1` to the bottom one. `Interrupt_enableInterrupt` enables the port interrupt function to be called.

This is one of the two interrupts function called:

```c
    void PORT5_IRQHandler(void)
    {
        if ((GPIO_getEnabledInterruptStatus(GPIO_PORT_P5) & GPIO_PIN1))
        {
            GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN1);
            event = BUTTON1_PRESSED;
        }
    }
```
In this case, the top pushbutton is pressed and changes the event variable. The main function checks the state based on the event and calls the corresponding `fn_CITY_NAME` funtions to update the current state of the FSM and display weather information. Each fn_CITY_NAME function is defined as follows: 

```c
    void fn_CITY_NAME()
    {
        if (event == BUTTON1_PRESSED)
        {
            display_weather();
            current_state = NEXT_CITY;
        }
        else if (event == BUTTON2_PRESSED)
        {
            display_weather();
            current_state = PREVIOUS_CITY;
        }
    }
```
Considering that the order is: 

    ROME ---  MOSCOU
    |           |
    |           |
    TOKYO --- NEW YORK

With `BUTTON1_PRESSED` going clockwise and `BUTTON2_PRESSED` going counter-clockwise in the order given starting from Rome.

The application goes on indefinitely.

## Team members
Bortolon Matteo 
Bouveret Samuele

We worked together on the code part and the video, helping each other when needed. Matteo then focused on the presentation PowerPoint and Samuele on the GitHub page.