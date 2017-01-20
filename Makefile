# Put your STM32F4 library code directory here
CAN_DIR=Src
INC_DIR=Inc
LIB_DIR=lib

STM_LIB_SRC = Drivers/STM32F0xx_HAL_Driver
STM_USB_CORE = Middlewares/ST/STM32_USB_Device_Library/Core
STM_USB_CDC = Middlewares/ST/STM32_USB_Device_Library/Class/CDC
CMSIS_CORE = Drivers/CMSIS/Include
CMSIS_STM32 = Drivers/CMSIS/Device/ST/STM32F0xx

TARGET=stm32f0xx

# Put your source files here (or *.cflashing a hex file on a stm32f4discovery, etc)

CAN_SRC := $(CAN_DIR)/*.c
STM_SRC := $(STM_LIB_SRC)/Src/$(TARGET)*.c
STM_SRC += $(STM_USB_CORE)/Src/*.c
STM_SRC += $(STM_USB_CDC)/Src/*.c
STM_SRC += $(CMSIS_STM32)/Source/Templates/system_$(TARGET).c
STARTUP := $(CMSIS_STM32)/Source/Templates/gcc/startup_stm32f042x6.s

# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME=CanNode

# Normally you shouldn't need to change anything below this line!
#######################################################################################

CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar
OBJCOPY=arm-none-eabi-objcopy

ODIR=obj

CFLAGS += -Os -Wall -g
CFLAGS += --std=gnu11 --specs=nosys.specs -mthumb -mcpu=cortex-m0
CFLAGS += -TSTM32F042F6_FLASH.ld -fdata-sections -ffunction-sections -Wl,--gc-sections

# Include files from STM libraries
INCLUDE += -I$(INC_DIR)
INCLUDE += -I$(CMSIS_CORE)
INCLUDE += -I$(CMSIS_STM32)/Include
INCLUDE += -I$(STM_LIB_SRC)/Inc
INCLUDE += -I$(STM_USB_CORE)/Inc
INCLUDE += -I$(STM_USB_CDC)/Inc

STM_SRC_EXP := $(wildcard $(STM_SRC))
STM_OBJ := $(STM_SRC_EXP:.c=.o)
STM_OBJ += $(STARTUP:.s=.o)

CAN_SRC_EXP := $(wildcard $(CAN_SRC))
CAN_OBJ := $(CAN_SRC_EXP:.c=.o)

.PHONY: clean all size

.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@

.s.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@

all: main tags size

main: $(CAN_OBJ) $(STM_OBJ) 
	$(CC) $(CFLAGS) $(INCLUDE) main.c $(CAN_OBJ) $(STM_OBJ) -o $(PROJ_NAME).elf
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin
	
CanNode: $(CAN_OBJ) $(STM_OBJ)
	$(AR) rcs $(LIB_DIR)/libCanNode.a $(STM_OBJ) $(CAN_OBJ)

StmCore: $(STM_OBJ)
	$(AR) rcs $(LIB_DIR)/libStmCore.a $^

clean:
	rm -f *.o $(CAN_OBJ) $(STM_OBJ) $(LIB_DIR)/* $(PROJ_NAME)*.elf $(PROJ_NAME)*.bin

size: 
	arm-none-eabi-size $(PROJ_NAME)*.elf

tags: force_look
	ctags -R *

force_look:
	true

# Flash the STM32F4
flash: all
	dfu-util -d 0483:df11 -c 1 -i 0 -a 0 -s 0x08000000 -D $(PROJ_NAME).bin
	
stflash: main
	st-flash write $(PROJ_NAME).bin 0x08000000

docs: 
	doxygen Doxyfile
