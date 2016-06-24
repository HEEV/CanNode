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
SRCS = $(SRC_DIR)/main.c

SRCS += $(SRC_DIR)/*.c
SRCS += $(STM_LIB_SRC)/Src/$(TARGET)*.c
#SRCS += $(STM_USB_CORE)/Src/*.c
#SRCS += $(STM_USB_CDC)/Src/*.c
SRCS += $(CMSIS_STM32)/Source/Templates/system_$(TARGET).c
SRCS += $(CMSIS_STM32)/Source/Templates/gcc/startup_stm32f042x6.s


# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME=CanNode

# Normally you shouldn't need to change anything below this line!
#######################################################################################

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

CFLAGS  = -Os -Wall  -TSTM32F042F6_FLASH.ld
CFLAGS += --specs=nosys.specs -mthumb -mcpu=cortex-m0 
CFLAGS += -I. --std=gnu11
CFLAGS += -fdata-sections -ffunction-sections -Wl,--gc-sections

# Include files from STM libraries
CFLAGS += -I$(INC_DIR)
CFLAGS += -I$(CMSIS_CORE)
CFLAGS += -I$(CMSIS_STM32)/Include
CFLAGS += -I$(STM_LIB_SRC)/Inc
#CFLAGS += -I$(STM_USB_CORE)/Inc
#CFLAGS += -I$(STM_USB_CDC)/Inc

OBJS = $(SRCS:.c=.o)


.PHONY: proj

all: proj

proj: $(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ 
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

clean:
	rm -f *.o $(PROJ_NAME).elf $(PROJ_NAME).hex $(PROJ_NAME).bin

# Flash the STM32F4
flash: all
	dfu-util -d 0483:df11 -c 1 -i 0 -a 0 -s 0x08000000 -D $(PROJ_NAME).bin

stflash: all
	st-flash write $(PROJ_NAME).bin 0x08000000
