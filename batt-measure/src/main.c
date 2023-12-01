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

/* Includes ----------------------------------------------------------*/
#include <avr/io.h>        // AVR device-specific IO definitions
#include <avr/interrupt.h> // Interrupts standard C library for AVR-GCC
#include <stdlib.h>        // C library. Needed for number conversions
#include <stdio.h>  
#include <math.h>          // C library for math operations

#include "timer.h" // Timer library for AVR-GCC
#include <gpio.h>  // 
#include <uart.h>  // Peter Fleury's UART library
#include <oled.h>  // 
// #include <lcd.h>            // Peter Fleury's LCD library

/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Use Timer/Counter1 and start ADC conversion every 100 ms.
 *           When AD conversion ends, send converted value to LCD screen.
 * Returns:  none
 **********************************************************************/

// #define Start_button PB5   // PB5 is start button for C measurement
// #define Start_button PB0   // PB5 is start button for C measurement
// #define Gate_ON PD7  // PD7 is measurement trigger pin for C measurement
#ifndef F_CPU
#define F_CPU 16000000 // CPU frequency in Hz required for UART_BAUD_SELECT

#endif

// Global vars
float Uvalue;
float Ivalue;
float mAs;
//double tim0_ovf_count = 0;
//double stoUS_counter = 0;
uint16_t voltage, current;

int main(void)
{
    // Configure Analog-to-Digital Converter unit
    // Select ADC voltage reference to "AVcc with external capacitor at AREF pin"
    ADMUX |= (1 << REFS0);
    // Select input channel ADC0 (voltage divider pin)
    ADMUX &= ~(1 << MUX3 | 1 << MUX2 | 1 << MUX1 | 1 << MUX0); // puvodni
    // Enable ADC module
    ADCSRA |= (1 << ADEN);
    // Enable conversion complete interrupt
    ADCSRA |= (1 << ADIE);
    // Set clock prescaler to 128
    // ADCSRA = ADCSRA | (1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0);
    // Prescaler set to 16. ADPS[2:0] = 100
    ADCSRA &= ~(1 << ADPS1 | 1 << ADPS0); ADCSRA |= (1 << ADPS2); 
    // Enable ADC Auto Trigger Enable
    ADCSRA |= (1 << ADATE);
    // Start Conversion
    ADCSRA |= (1 << ADSC);
    // Set Free Running Mode as ADC Auto Trigger Source ADTS[2:0] = 000
    ADCSRB &= ~(1 << ADTS2 | 1 << ADTS1 | 1 << ADTS0);
    
    // Set GPIO input pins
    // GPIO_mode_input_pullup(&DDRB, Start_button);


    // INIT
    uart_init(UART_BAUD_SELECT(115200, F_CPU));
    uart_puts("Init start\r\n");

    // Initialize OLED
    oled_init(OLED_DISP_ON);
    oled_clrscr();
    oled_set_contrast(25); // Contrast setting
    // oled_drawLine(x1, y1, x2, y2, color)
    oled_drawLine(0, 32, 120, 32, WHITE);

    oled_charMode(DOUBLESIZE);
    oled_gotoxy(3, 0);  oled_puts("Battery");
    oled_gotoxy(5, 2);  oled_puts("Meter");

    oled_charMode(NORMALSIZE);
    oled_gotoxy(6, 5);  oled_puts("Voltage:");
    oled_gotoxy(8, 6);  oled_putc(',');
    oled_gotoxy(12, 6); oled_putc('V');

    oled_display();

    // Configure 16-bit Timer/Counter1 to start ADC conversion
    // Set prescaler to 262 ms and enable overflow interrupt
    TIM1_OVF_1SEC
    TIM1_OVF_ENABLE

    // Enables interrupts by setting the global interrupt mask
    sei();

    uart_puts("Init end\r\n");


    

    // oled_gotoxy(3, 2);
    // oled_puts("Capacitance:");
    // oled_gotoxy(8, 3);
    // oled_putc(',');
    // oled_gotoxy(12, 3);
    // oled_puts("mAh");
    // oled_display();



    // Infinite loop
    while (1)
    {
        // float tenths;
        // float hundreds;
        // float thousands;
        // char Vstring[4]; // String for converted numbers by itoa()
        // char Tstring[4];
        // char Hstring[4];
        // char Thstring[4];

        // tenths = 10.0*(Uvalue - floor(Uvalue));
        // hundreds = 10.0 * (tenths - floor(tenths));
        // thousands = 10.0 * (hundreds - floor(hundreds));

        // itoa(Uvalue, Vstring, 10);
        // itoa(tenths, Tstring, 10);
        // itoa(hundreds, Hstring, 10);
        // itoa(thousands, Thstring, 10);

        // oled_gotoxy(7,1);
        // oled_puts(Vstring);
        // oled_gotoxy(9,1);
        // oled_puts(Tstring);
        // oled_gotoxy(10,1);
        // oled_puts(Hstring);
        // oled_gotoxy(11,1);
        // oled_puts(Thstring);

        // uart_puts(Vstring);
        // uart_putc(',');
        // uart_puts(Tstring);
        // uart_puts(Hstring);
        // uart_puts(Thstring);
        // uart_puts("V\r\n");

        // oled_display();
    }
    // Will never reach this
    return 0;
}

/* Interrupt service routines ----------------------------------------*/
/**********************************************************************
 * Function: Timer/Counter1 overflow interrupt
 * Purpose:  Use Single Conversion mode and start conversion every 100 ms.
 **********************************************************************/


/**********************************************************************
 * Function: ADC complete interrupt
 * Purpose:  Display converted value on LCD screen.
 **********************************************************************/
ISR(TIMER1_OVF_vect)
{

}

ISR(ADC_vect)
{
    // Read converted value
    // Note that, register pair ADCH and ADCL can be read as a 16-bit value ADC
    //Uvalue = 5.0 * ADC / 1024.0; // Value converter for reading voltage in reference to AVCC = 5V
    // Ivalue = (5.0 * ADC / 1024.0)*0.05;
}