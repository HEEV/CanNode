# CanNode - STM32 CAN library:

# How to use
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

