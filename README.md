# Weather APP

This application displays weather information of different cities via the MSP432P401R. Weather type and temperature is fetched through an API using the CC3100 WiFi module and then displayed on the LCD screen of the BoosterPack MKII.
## Hardware requirements

| Module | Usage | Description |
| :- | :- | :- |
| MSP432P401R | Data elaboration | Microcontroller used to elaborate and display the received data. |
| CC3100Boost    | API request and response handling | Used to connect to a WiFi network and creates a request to an online API to retrieve data about weather. |
| BoosterPack MKII      | LCD display and pushbuttons | The LCD display is used to display data and the 2 pushbuttons to navigate to the different cities avaiable. |

## Software requirements

Code Composer Studio(CCS) 10.1.0 IDE is used to code, compile, debug and burn into the MSP432P401R. Texas Instruments driverlib is used for higher code abstraction.
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
1.Create 2 blank MSP432P401R projects in CCS, one for each of the 2 parts.
2. Replace every file in the project folders. 

3.a Part 1: also replace .cproject file inside the project to include dependecies.

3.b Part 2: add simplelink_msp432p4_sdk_3_40_01_02/source" directory to "Add dir to #include search path" window in CCS Build->ARM Compiler->Include options in project properties.
Then add simplelink_msp432p4_sdk_3_40_01_02/source/ti/devices/msp432p4xx/driverlib/ccs/msp432p4xx_driverlib.lib and ../source/ti/grlib/lib/css/m4f/grlib.a to "Include library file..." in CCS Build->ARM linker->File Search Path.
4. Build the project and start debugging with the MSP432 plugged in to run the app.

#### Team members
Bortolon Matteo 
Bouveret Samuele

We worked together on the code part and the video, helping each other when needed. Matteo then focused on the presentation PowerPoint and Samuele on the GitHub page.

```c
  #define 
  #define 
```



In the future I plan to add more plants .

our Readme file should include the following:
○ What are needed to run the project:
■ Hardware/software requirements
○ Project Layout
■ Source code organization
○ How to build, burn and run your project
○ Links to your powerpoint presentation and to your Youtube video
○ Team **members**:
■ explaining clearly how the members contributed to the project