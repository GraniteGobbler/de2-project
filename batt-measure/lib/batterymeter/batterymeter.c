
#include <avr/io.h>        // AVR device-specific IO definitions
#include <avr/interrupt.h> // Interrupts standard C library for AVR-GCC
#include <stdio.h> // C library for IO operations
#include <math.h>  // C library for math operations
#include <gpio.h>  //
#include <uart.h>  // Peter Fleury's UART library
#include <oled.h> 
#include "batterymeter.h"

// Global vars
volatile float ADC_A0;          // Analog pin A0 voltage
volatile uint16_t TIM1_OVF_CNT; // Timer1 overflow counter


// Funcs
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

    GPIO_mode_output(&DDRB, Base_ON);
    GPIO_write_high(&PORTB, Base_ON);
    
    oled_init(OLED_DISP_ON); // Initialize OLED
    oled_set_contrast(255); // Contrast setting
    oled_invert(1);
    oled_clrscr();
    
    uart_puts("Init end\r\n");

}

void batterymeter_change_scr(unsigned int screenID)
{
    switch (screenID)
    {
    case 1:
        
        uart_puts("Screen 1 \r\n");  
        oled_charMode(DOUBLESIZE);
        oled_puts("BATT Meter");
        oled_drawLine(0, 15, 128, 15, WHITE);

        oled_charMode(NORMALSIZE);
        oled_gotoxy(0, 5);  oled_puts("Press GREEN to start!");
        oled_gotoxy(1, 6);  oled_puts("Press RED to pause!");
        
        break;
    
    default:
        break;
    } 

}
