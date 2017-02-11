# README #
Currently the latest code is in the can-testing branch, you should be using it
instead of the master branch. One of these days I will merge them.

### What is this repository for? ###

Code for the a sensor module that communicates over a CAN bus.
Currently it is targeted for the stm32f042C6.

### Description of code ###

#### main.c ####
main.c contains a basic test program which uses the CanNode library. The code
implements everything necessary for a wheel speed sensor and a pitot sensor.
Its kind of a hack to have them together on the same board, but it works for now.

The Tachometer part of the code is broken into two separate addresses. The first
part is located at address 1050 it gives the number of wheel revolutions
per second or RPS. This would be useful for finding the distance traveled by the
vehicle. The second function is at address 1054 it provides the ammount of time
between pulses. By dividing this data by the circumfrence of the wheel, the
velocity of the vehicle can be found.

#### Inc/CanNode.h and Src/CanNode.c ####
These files comprise the high level functions for the CanNode library they are 
designed to be written in portable C code that could be transfered to a PC application.
The functions and structs used provide a easy way to tranmit and recieve data over the
CANBus.

#### Inc/CanTypes.h ####
Definitions for all of the constants for the CanNode library. This file also
includes a enum type that has all of base addresses for the planned CAN system
in the car.

#### Inc/can.h and Src/can.c ####
Low level CANBus functions, interface with the hardware to provide can support.

### How do I get set up? ###

make sure that gcc-arm-none-eabi is installed.

for a debian based disto
sudo apt-get install gcc-arm-none-eabi

type make in the root directory to build the code.
type make flash to upload to the board over the dfu bootloader (when board is in dfu mode).
type make docs to build the doxygen documentation for the project. Look at the
doc/html/index.html file for the documentation.
