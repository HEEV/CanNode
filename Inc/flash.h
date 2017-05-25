/**
 * \file flash.h
 * \brief Functions for writing to flash in application.
 *
 * This compilation unit provides functions for the in-application writing of
 * flash memory. Specifically for, but not limited to, the stm32f04x line
 * of microcontrollers. 
 *
 * The process for using these functions is as follows
 * -# call flashUnlock() - This function will unlock the flash memory of the chip
 * for in application writing
 * -# call flashErasePage() to set a page of memory to all 0xffff's. Do you 
 * want to live dangerously? If you know that the memory that you are 
 * trying to write has already been erased, then this step can be skipped.  
 * -# call one of the flashWrite functions (flashWrite_16() or flashWrite_32())
 * these functions do the actuall dirty work of writing to the flash memory
 * -# clean up after yourself! call flashLock()
 *
 * \author Samuel Ellicott
 * \date 7-15-16
 */
#ifndef _FLASH_H_
#define _FLASH_H_

#include <stm32f0xx.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * \enum FlashError
 * \brief Error states for writing to flash
 */
typedef enum {
	FLASH_OK,        ///< "Good, code with all of your anger"
	FLASH_PRG_ERROR, ///< Programming error, you didn't call flashErasePage() did you?
	FLASH_WRT_ERROR  ///< Write protection error, this section of flash is write protected.
} FlashError;

/// Key to unlokck stm32 flash memory
#define FLASH_FKEY1 0x45670123
/// Key to unlokck stm32 flash memory
#define FLASH_FKEY2 0xCDEF89AB

/// \brief Unlock flash memory
void flashUnlock();
/// \brief Lock flash memory
void flashLock();

/**
 * \name Flash writing functions
 *
 */
//@{
/// \brief Erase a page of memory
FlashError flashErasePage(uint32_t addr);
/// \brief Copy a page (1k) of flash.
FlashError flashCopyPage(uint32_t src_addr, uint32_t dest_addr, bool erase);

/// \brief Write a piece of 16-bit data
FlashError flashWrite_16(uint32_t addr, uint16_t data);
/// \brief Write a piece of 32-bit data
FlashError flashWrite_32(uint32_t addr, uint32_t data);
/// \brief Write a block of memory to flash
void flashWriteMemBlock(uint32_t addr, uint8_t* data, uint16_t len);
//@}

#endif //_FLASH_H_
