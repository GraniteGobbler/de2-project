#ifndef batterymeter
#define batterymeter
/***********************************************************************
Title: Battery meter library for AVR-GCC
  *
  * ATmega328P (Arduino Uno), 16 MHz, PlatformIO
  *
Authors: Jan Čipl, Vojtěch Drtina, Jan Gadas, Christopher Koiš
  *
  * Dept. of Radio Electronics, Brno University of Technology, Czechia
  *
License: MIT License

 **********************************************************************/

/**
 * @file 
 * @defgroup batterymeter Battery Meter library <batterymeter.h>
 * @code #include <batterymeter.h> @endcode
 *
 * @brief Battery Meter Library for AVR-GCC.
 *
 * The library contains macros for initialization and data displaying using an OLED display
 *
 * @note Based on Microchip Atmel ATmega328P manual, OLED and Timer libaries. 
 * @author Jan Čipl, Vojtěch Drtina, Jan Gadas, Christopher Koiš
 * @copyright This work is licensed under the terms of the MIT license
 */


/* Includes ----------------------------------------------------------*/
#include <avr/io.h>        // AVR device-specific IO definitions
#include <avr/interrupt.h> // Interrupts standard C library for AVR-GCC
#include <stdio.h> // C library for IO operations
#include <math.h>  // C library for math operations
#include <gpio.h>  //
#include <uart.h>  // Peter Fleury's UART library
#include <oled.h> 
#include "..\..\include\timer.h" // Timer library for AVR-GCC


/* Defines -----------------------------------------------------------*/
/**
 * @name  Battery Meter initialization definitions
 */
/** @brief Select ADC voltage reference to "AVcc with external capacitor at AREF pin" */
#define ADC_INT_REF         ADMUX |= (1 << REFS0);
/** @brief Select input channel ADC0 (voltage divider pin) */
#define ADC_CHANNEL_0       ADMUX &= ~(1 << MUX3 | 1 << MUX2 | 1 << MUX1 | 1 << MUX0);
/** @brief Enable ADC module */
#define ADC_ENABLE          ADCSRA |= (1 << ADEN);
/** @brief Enable conversion complete interrupt */
#define ADC_ISR_ENABLE      ADCSRA |= (1 << ADIE);
/** @brief Set clock prescaler to 128 → F_CPU/PS = 125 kHz ADC clock @16 MHz */
#define ADC_PS_128          ADCSRA |= (1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0);

/**
 * @name  
 * @note  
 */


/* Functions -----------------------------------------------------------*/
/**
 * @name  Function prototypes for Battery Meter
 */
/**
   @brief   Initialize UART and set baudrate 
   @param   none
   @return  none 
*/
extern void batterymeter_init();


/** @} */

#endif