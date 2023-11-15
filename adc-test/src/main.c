/***********************************************************************
 * 
 * Use Analog-to-digital conversion to read push buttons on LCD keypad
 * shield and display it on LCD screen.
 * 
 * ATmega328P (Arduino Uno), 16 MHz, PlatformIO
 *
 * Copyright (c) 2018 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/
/* Defines -----------------------------------------------------------*/
#ifndef F_CPU
# define F_CPU 16000000  // CPU frequency in Hz required for UART_BAUD_SELECT
/** @brief ADC reference voltage, prescaler 000 --> STOP */
#define ADC_AVCC  ADMUX  &= ~(1<<REFS1); ADMUX |= (1<<REFS0);

// dopisat ADC nastavenie registru
// https://github.com/tomas-fryza/digital-electronics-2/tree/master/labs/05-adc

#endif


/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <stdlib.h>         // C library. Needed for number conversions
#include <uart.h>           // Peter Fleury's UART library
#include <oled.h>            // Peter Fleury's oled library
#include <twi.h>            // I2C/TWI library for AVR-GCC

/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Use Timer/Counter1 and start ADC conversion every 100 ms.
 *           When AD conversion ends, send converted value to LCD screen.
 * Returns:  none
 **********************************************************************/
int main(void)
{
    char string[4];  // For converting numbers by itoa()

    // Initialize display
    oled_init(OLED_DISP_ON);
    oled_clrscr();
    oled_charMode(DOUBLESIZE);
    oled_puts("OLED disp.");
    oled_charMode(NORMALSIZE);

    twi_init();

    uart_init(UART_BAUD_SELECT(115200, F_CPU));
    sei();  // Needed for UART

    uart_puts("Init\r\n");

    // oled_gotoxy(x, y)
    oled_gotoxy(0, 2);
    oled_puts("128x64, SHH1106");

    // oled_drawLine(x1, y1, x2, y2, color)
    oled_drawLine(0, 25, 120, 25, WHITE);

    oled_gotoxy(0, 4);
    oled_puts("BPC-DE2, Brno");

    // Copy buffer to display RAM
    oled_display();
    oled_clrscr();

    oled_gotoxy(1, 0); oled_puts("value:");
    oled_gotoxy(3, 1); oled_puts("key:");
    oled_gotoxy(8, 0); oled_puts("a");  // Put ADC value in decimal
    oled_gotoxy(13,0); oled_puts("b");  // Put ADC value in hexadecimal
    oled_gotoxy(8, 1); oled_puts("c");  // Put button name here
    
    oled_display();

    // Configure Analog-to-Digital Convertion unit
    // Select ADC voltage reference to "AVcc with external capacitor at AREF pin"

    // Select input channel ADC0 (voltage divider pin)

    // Enable ADC module

    // Enable conversion complete interrupt

    // Set clock prescaler to 128


    // Configure 16-bit Timer/Counter1 to start ADC conversion
    // Set prescaler to 33 ms and enable overflow interrupt

    // Timer1
    TIM1_OVF_33MS
    TIM1_OVF_ENABLE

    uart_puts("Scanning I2C... ");
    for (uint8_t sla = 8; sla < 120; sla++) {
        if (twi_test_address(sla) == 0) {  // ACK from Slave
            uart_puts("\r\n");
            itoa(sla, string, 16);
            uart_puts(string);
        }
    }


    // Infinite loop
    while (1)
    {
        /* Empty loop. All subsequent operations are performed exclusively 
         * inside interrupt service routines ISRs */
    }

    // Will never reach this
    return 0;
}


/* Interrupt service routines ----------------------------------------*/
/**********************************************************************
 * Function: Timer/Counter1 overflow interrupt
 * Purpose:  Use single conversion mode and start conversion every 100 ms.
 **********************************************************************/
ISR(TIMER1_OVF_vect)
{
    // Start ADC conversion
}

/**********************************************************************
 * Function: ADC complete interrupt
 * Purpose:  Display converted value on LCD screen.
 **********************************************************************/
ISR(ADC_vect)
{
    uint16_t value;
    char string[4];  // String for converted numbers by itoa()

    // Read converted value
    // Note that, register pair ADCH and ADCL can be read as a 16-bit value ADC
    value = ADC;
    // Convert "value" to "string" and display it
}
