include ../Makefile.inc

SRC := ../$(STM_LIB_SRC)/Src/stm32f0xx_hal_*.c
SRC += ../$(CMSIS_STM32)/Source/Templates/system_stm32f0xx.c
SRC += ../$(STM_USB_CORE)/Src/*.c
SRC += ../$(STM_USB_CDC)/Src/*.c

SRC_EXP := $(wildcard $(SRC))
OBJ := $(SRC_EXP:.c=.o)

CFLAGS = $(PRJCFLAGS) $(STM_INCLUDE) 

all : ../$(OUTPUT)/libStmCore

../$(OUTPUT)/libStmCore : $(OBJ)
	$(STM_SRC)
	$(AR) rcs ../$(OUTPUT)/libStmCore.a $?

clean :
	rm -f $(OBJ)
	rm -f ../$(OUTPUT)/libStmCore.a

.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@

#.s.o:
#	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@
