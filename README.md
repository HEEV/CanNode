# README #


### What is this repository for? ###

Code for the a sensor module that communicates over a CAN bus. 
Currently it is targeted for the stm32f042F6 uc, but will probably move to the stm32f042C6.
For more information go to the [wiki](./wiki/Home)

### Description of code ###

Currently all my code is in Src/main.c, this will probably change as time goes by.

### How do I get set up? ###

make sure that gcc-arm-none-eabi is installed.

for a debian based disto
sudo apt-get install gcc-arm-none-eabi

type make in the root directory to build the code.
type make flash to upload to the board over the dfu bootloader (when board is in dfu mode).