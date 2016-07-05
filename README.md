# README #


### What is this repository for? ###

Code for the a sensor module that communicates over a CAN bus. 
Currently it is targeted for the stm32f042F6 uc, but will probably move to the stm32f042C6.

### Description of code ###

#### Src/main.c ####
main.c contains a basic test program which uses the CanNode library. It reads
a value from a ADC connected to a potentiometer then sends it over the CANbus.
A second CanNode has a filter to recieve values from the first CanNode an blinks
its light at the same rate. This is handled through callback functions.

#### Inc/CanNode.h and Src/CanNode.c ####
These files comprise the high level functions for the CanNode library they are 
designed to be written in portable C code that could be transfered to a PC application.
The functions and structs used provide a easy way to tranmit and recieve data over the
CANBus.

#### Inc/can.h and Src/can.c ####
Low level CANBus functions, interface with the hardware to provide can support.

### How do I get set up? ###

make sure that gcc-arm-none-eabi is installed.

for a debian based disto
sudo apt-get install gcc-arm-none-eabi

type make in the root directory to build the code.
type make flash to upload to the board over the dfu bootloader (when board is in dfu mode).
