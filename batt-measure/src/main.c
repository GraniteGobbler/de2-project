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

/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Use Timer/Counter1 and start ADC conversion every 100 ms.
 *           When AD conversion ends, send converted value to LCD screen.
 * Returns:  none
 **********************************************************************/

#define Start_button PD2   // PD2 is start button for C measurement
#define Stop_button PD3   // PD3 is stop button for C measurement
#define Base_ON PD7  // PD7 is measurement trigger pin for C measurement

#ifndef F_CPU
#define F_CPU 16000000 // CPU frequency in Hz required for UART_BAUD_SELECT

#endif

// Global vars
float ADC_A0; // Analog pin A0 voltage
uint16_t TIM1_OVF_CNT; // Timer1 overflow counter

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
    ADCSRA = ADCSRA | (1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0);
    // Prescaler set to 16. ADPS[2:0] = 100
    // ADCSRA &= ~(1 << ADPS1 | 1 << ADPS0); ADCSRA |= (1 << ADPS2); 
    // Enable ADC Auto Trigger Enable
    // ADCSRA |= (1 << ADATE);
    // Set Free Running Mode as ADC Auto Trigger Source ADTS[2:0] = 000
    // ADCSRB &= ~(1 << ADTS2 | 1 << ADTS1 | 1 << ADTS0);
    
    // Set GPIO input pins
    // GPIO_mode_input_pullup(&DDRD, Start_button);
    // GPIO_mode_input_pullup(&DDRD, Stop_button);


    ////  INIT  ////
    uart_init(UART_BAUD_SELECT(115200, F_CPU));
    uart_puts("Init start\r\n");

    oled_init(OLED_DISP_ON); // Initialize OLED
    oled_clrscr();
    oled_set_contrast(255); // Contrast setting
    oled_invert(1);
    
    oled_charMode(DOUBLESIZE);
    oled_puts("BATT Meter");
    // oled_gotoxy(5, 2);  oled_puts("Meter");

    // oled_drawLine(x1, y1, x2, y2, color)
    oled_drawLine(0, 15, 128, 15, WHITE);

    oled_charMode(NORMALSIZE);
    oled_gotoxy(0, 3);  oled_puts("Voltage:  _.___ V");
    oled_gotoxy(0, 4);  oled_puts("Current:  _.___ A");
    oled_gotoxy(0, 5);  oled_puts("Capacity: _.___ mAh");
    oled_gotoxy(0, 7);  oled_puts("Press GREEN to start!"); 

    oled_display();

    // Configure 16-bit Timer/Counter1 to start ADC conversion
    // Set prescaler to 262 ms and enable overflow interrupt
    TIM1_OVF_1SEC
    TIM1_OVF_ENABLE

    // Enables interrupts by setting the global interrupt mask
    sei();

    uart_puts("Init end\r\n");
    ////  END INIT  ////


    // Vars in while loop
    uint8_t isStarted = 0;
    
    float ADC_A0_old = 0.0;
    float Voltage = 0.0;
    float Voltage_unloaded = 0.0;
    float Current = 0.0;
    float R_circ = 1.11265; // Total circuit resistance
    float R_bat = 0.0; // Internal resistance of battery - !needs to be calculated after 4 sec!

    char cVolt[8];
    char cCurr[8];


    // Infinite loop
    while (1)
    {
        if ((GPIO_read(&PIND, Start_button) == 0) & (isStarted == 0))
        {
            uart_puts("Start button pressed!\r\n");
            oled_gotoxy(0, 7);  oled_puts("                     "); // Clear 7th row

            Voltage_unloaded = ADC_A0;  // Snapshot of unloaded voltage of battery
            
            sprintf(cVolt, "%.3f", Voltage_unloaded);                           
            uart_puts("Unloaded Voltage: ");
            uart_puts(cVolt);
            uart_puts("\r\n");

            isStarted = 1;
        }

        if ((GPIO_read(&PIND, Stop_button) == 0) & (isStarted == 1))
        {
            uart_puts("Stop button pressed!\r\n");
            oled_gotoxy(0, 7);  oled_puts("Press GREEN to start!");
            oled_gotoxy(9,3);  oled_puts(" _.___ V");
            oled_gotoxy(9,4);  oled_puts(" _.___ A");
            
            isStarted = 0;
        }

        if (isStarted == 1)
        {
            TIM1_OVF_CNT = 0;
            Voltage = ADC_A0;
            Current = Voltage/(R_circ + R_bat);

            if (floor(Voltage) == 0.0)  // Is 0 V?
            {
                oled_gotoxy(9,3);   oled_putc(' '); // Don't print '-' sign
                oled_gotoxy(9,4);   oled_putc(' ');
            }
            else if ((Voltage * -1) == fabs(Voltage))   // Is negative? 
            {
                oled_gotoxy(9,3);   oled_putc('-'); // Print '-' sign on a specific place
                oled_gotoxy(9,4);   oled_putc('-');
            }
            else    // Is positive?
            {
                oled_gotoxy(9,3);   oled_putc(' '); // Clear '-' sign
                oled_gotoxy(9,4);   oled_putc(' ');
            }
            
            if (ADC_A0 != ADC_A0_old)   // Only print values if there is a change
            {
                sprintf(cVolt, "%.3f", fabs(Voltage));
                sprintf(cCurr, "%.3f", fabs(Current));                 
                oled_gotoxy(10,3);   oled_puts(cVolt);
                oled_gotoxy(10,4);   oled_puts(cCurr);
            
                uart_puts("Voltage: ");
                uart_puts(cVolt);
                uart_puts("\r\n");

                uart_puts("Current: ");
                uart_puts(cCurr);
                uart_puts("\r\n");
            }
        }

        ADC_A0_old = ADC_A0;

        oled_display();        
    }

    return 0;
}

/* Interrupt service routines ----------------------------------------*/
/**********************************************************************
 * Function: Timer/Counter1 overflow interrupt
 * Purpose:  Use Single Conversion mode and start conversion every 1 s.
 **********************************************************************/
ISR(TIMER1_OVF_vect)
{
    ADCSRA |= (1 << ADSC); // Start Conversion
    TIM1_OVF_CNT++;
}

/**********************************************************************
 * Function: ADC complete interrupt
 * Purpose:  Convert ADC value to 5V AVCC reference
 **********************************************************************/
ISR(ADC_vect)
{
    ADC_A0 = 5.0 * ADC / 1023.0;   // ADC channel A0
}