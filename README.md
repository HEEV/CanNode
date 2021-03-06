# CanNode - STM32 CAN library:

## How to use
This repository should be used as a submodule in another project. For examples of projects using this library see 
[HEEV/CanNodeProjects](https://github.com/HEEV/CanNodeProjects). 

To use the library the file CanNode.h needs to be included `#include <CanNode.h>` Also the stm32 hardware abstraction layer
must be started before using any functions in the library. 

```cpp
void main(void) {
  // Reset of all peripherals, Initializes the Flash interface and the Systick.                   
  HAL_Init();                                                                                     
  // Configure the system clock                                                                   
  SystemClock_Config();  
  
  //create an example CanNode
  CanNode node(THROTTLE, throttleRTR);
  // do whatever
  // ...
}
```
## Common tasks
1) Creating a CanNode
```cpp
//outside of main
CanNode* nodePtr;

//handler function: returns void and takes a CanMessage pointer as an argument.
void throttleRTR(CanMessage* msg);

void main(void) {
  // stuff
  // ...
  
  // use standard C++ object initilization of an object
  // function parameters are the device type (This is an enum type defined in CanTypes.h)
  // the second parameter is the name of a handler function for a RTR message.
  CanNode node(THROTTLE, throttleRTR);
  
  // you can also cast an int to a CanNodeType.
  // CanNode node((CanNodeType) 350, throttleRTR);
  
  //make a pointer to the node so that other functions can access it.
  nodePtr = &node;
}
```
