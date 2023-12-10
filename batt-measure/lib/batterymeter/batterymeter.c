
#include <avr/io.h>        // AVR device-specific IO definitions
#include <avr/interrupt.h> // Interrupts standard C library for AVR-GCC
#include <stdio.h> // C library for IO operations
#include <math.h>  // C library for math operations
#include <gpio.h>  //
#include <uart.h>  // Peter Fleury's UART library
#include <oled.h> 
#include "batterymeter.h"
#include "timer.h" // Timer library for AVR-GCC



void batterymeter_init()
{
    uart_init(UART_BAUD_SELECT(115200, F_CPU));
    uart_puts("Init start\r\n");

    ADC_INT_REF
    ADC_CHANNEL_0
    ADC_ENABLE
    ADC_ISR_ENABLE
    ADC_PS_128

    // Configure 16-bit Timer/Counter1 to start ADC conversion
    // Set prescaler to 1 s and enable overflow interrupt
    TIM1_OVF_1SEC
    TIM1_OVF_ENABLE

    // Enables interrupts by setting the global interrupt mask
    sei();


}
