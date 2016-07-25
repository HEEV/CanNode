# Put your STM32F4 library code directory here
SRC_DIR=Src
INC_DIR=Inc
STM_LIB_SRC=Drivers/STM32F0xx_HAL_Driver
STM_USB_CORE=Middlewares/ST/STM32_USB_Device_Library/Core
STM_USB_CDC=Middlewares/ST/STM32_USB_Device_Library/Class/CDC
CMSIS_CORE=Drivers/CMSIS/Include
CMSIS_STM32=Drivers/CMSIS/Device/ST/STM32F0xx

TARGET=stm32f0xx

# Put your source files here (or *.cflashing a hex file on a stm32f4discovery, etc)

SRCS := $(SRC_DIR)/*.c
SRCS += $(STM_LIB_SRC)/Src/$(TARGET)*.c
SRCS += $(STM_USB_CORE)/Src/*.c
SRCS += $(STM_USB_CDC)/Src/*.c
SRCS += $(CMSIS_STM32)/Source/Templates/system_$(TARGET).c
ASM += $(CMSIS_STM32)/Source/Templates/gcc/startup_stm32f042x6.s

# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME=CanNode

# Normally you shouldn't need to change anything below this line!
#######################################################################################

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

ODIR=obj

CFLAGS += -Os -Wall -g
CFLAGS += --std=gnu11 --specs=nosys.specs -mthumb -mcpu=cortex-m0
CFLAGS += -fdata-sections -ffunction-sections -TSTM32F042F6_FLASH.ld -Wl,--gc-sections


# Include files from STM libraries
INCLUDE += -I$(INC_DIR)
INCLUDE += -I$(CMSIS_CORE)
INCLUDE += -I$(CMSIS_STM32)/Include
INCLUDE += -I$(STM_LIB_SRC)/Inc
INCLUDE += -I$(STM_USB_CORE)/Inc
INCLUDE += -I$(STM_USB_CDC)/Inc

#expand wildcards in sources
SRC_EXP := $(wildcard $(SRCS))

ASM_OBJ := $(ASM:.s=.o)
OBJS := $(SRC_EXP:.c=.o)

.PHONY: clean all size

.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@

.s.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@

all: pot switch

pot: pot_main.o $(OBJS) $(ASM_OBJ)
	$(CC)  $(CFLAGS) $^ -o $(PROJ_NAME)-switch.elf
	$(OBJCOPY) -O binary $(PROJ_NAME)-switch.elf $(PROJ_NAME)-switch.bin

switch: switch_main.o $(OBJS) $(ASM_OBJ)
	$(CC) $(CFLAGS) $^ -o $(PROJ_NAME)-pot.elf
	$(OBJCOPY) -O binary $(PROJ_NAME)-pot.elf $(PROJ_NAME)-pot.bin

clean:
	rm -f *.o $(OBJS) $(ASM_OBJ) $(PROJ_NAME)*.elf $(PROJ_NAME)*.bin

size: all
	size $(PROJ_NAME)*.elf

# Flash the STM32F4
flash: all
	dfu-util -d 0483:df11 -c 1 -i 0 -a 0 -s 0x08000000 -D $(PROJ_NAME).bin

stflash: all
	st-flash write $(PROJ_NAME).bin 0x08000000

docs: 
	doxygen Doxyfile
