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
// #include <avr/io.h>        // AVR device-specific IO definitions
// #include <avr/interrupt.h> // Interrupts standard C library for AVR-GCC
// // #include <stdlib.h>        // C library. Needed for number conversions
// #include <stdio.h> // C library for IO operations
// #include <math.h>  // C library for math operations


// #include <gpio.h>  //
// #include <uart.h>  // Peter Fleury's UART library
// #include <oled.h>  //
#include <batterymeter.h>

/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Use Timer/Counter1 and start ADC conversion every 100 ms.
 *           When AD conversion ends, send converted value to LCD screen.
 * Returns:  none
 **********************************************************************/

#define Start_button PD2 // PD2 is start button for battery measurement
#define Stop_button PD3  // PD3 is stop button for battery measurement
#define Base_ON PB0      // PB0 is measurement trigger pin for external battery load circuit

#ifndef F_CPU
#define F_CPU 16000000 // CPU frequency in Hz required for UART_BAUD_SELECT

#endif

// Global vars
volatile float ADC_A0;          // Analog pin A0 voltage
volatile uint16_t TIM1_OVF_CNT; // Timer1 overflow counter

int main(void)
{
    // // Configure Analog-to-Digital Converter unit
    // // Select ADC voltage reference to "AVcc with external capacitor at AREF pin"
    // ADMUX |= (1 << REFS0);
    // // Select input channel ADC0 (voltage divider pin)
    // ADMUX &= ~(1 << MUX3 | 1 << MUX2 | 1 << MUX1 | 1 << MUX0); // puvodni
    // // Enable ADC module
    // ADCSRA |= (1 << ADEN);
    // // Enable conversion complete interrupt
    // ADCSRA |= (1 << ADIE);
    // // Set clock prescaler to 128
    // ADCSRA |= (1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0);

    // ////  INIT  ////
    // uart_init(UART_BAUD_SELECT(115200, F_CPU));
    // uart_puts("Init start\r\n");

    // // Configure 16-bit Timer/Counter1 to start ADC conversion
    // // Set prescaler to 262 ms and enable overflow interrupt
    // TIM1_OVF_1SEC
    // TIM1_OVF_ENABLE

    // // Enables interrupts by setting the global interrupt mask
    // sei();

    // oled_init(OLED_DISP_ON); // Initialize OLED
    // oled_clrscr();
    // oled_set_contrast(255); // Contrast setting
    // oled_invert(1);

    // oled_charMode(DOUBLESIZE);
    // oled_puts("BATT Meter");
    // oled_drawLine(0, 15, 128, 15, WHITE);

    // oled_charMode(NORMALSIZE);
    // oled_gotoxy(0, 5);
    // oled_puts("Press GREEN to start!");
    // oled_gotoxy(1, 6);
    // oled_puts("Press RED to pause!");

    // GPIO_mode_output(&DDRB, Base_ON);
    // GPIO_write_high(&PORTB, Base_ON);

    // uart_puts("Init end\r\n");
    ////  END INIT  ////

    batterymeter_init();

    batterymeter_screen(1);


    //// Vars in while loop ////
    uint8_t isStarted = 0;
    // uint8_t isFinished = 0;
    uint16_t Current_Time = 0;

    // float ADC_A0_old = 0.0;
    float Voltage = 0.0;
    float Voltage_unloaded = 0.0;
    float Voltage_dropped = 0.0;

    float Current = 0.0;

    float Capacity = 0.0;           //  [mAh]
    float Capacity_increment = 0.0; //  [mAs]

    float Energy = 0.0;           // [mWh]
    float Energy_increment = 0.0; //  [mWs]

    float R_circ = 1.11265; // Total circuit resistance
    float R_bat = 0.0;      // Internal resistance of battery

    char cVolt[8];
    char cCurr[8];
    char cIR[8];
    char cCap[8];
    char cEne[8];

    // Infinite loop
    while (1)
    {
        if ((GPIO_read(&PIND, Start_button) == 0) & (isStarted == 0)) // Green Button
        {
            uart_puts("Measurement started!\r\n");

            oled_clrscr();
            oled_gotoxy(0, 0);
            oled_puts("IR:       _.___  mOhm");
            oled_gotoxy(0, 2);
            oled_puts("Voltage:  _.___   V");
            oled_gotoxy(0, 3);
            oled_puts("Current:  _.___   A");
            oled_gotoxy(0, 4);
            oled_puts("Capacity: _._     mAh");
            oled_gotoxy(0, 5);
            oled_puts("Energy:   _._     mWh");

            Voltage_unloaded = ADC_A0; // Snapshot of unloaded voltage of battery
            sprintf(cVolt, "%.3f", Voltage_unloaded);

            uart_puts("Unloaded Voltage: ");
            uart_puts(cVolt);
            uart_puts("\r\n");

            GPIO_write_low(&PORTB, Base_ON);
            TIM1_OVF_CNT = 0; // Reset timer overflow counter
            isStarted = 1;    // Set measurement start flag to 1
        }

        if ((GPIO_read(&PIND, Stop_button) == 0) & (isStarted == 1)) // Red Button
        {
            GPIO_write_high(&PORTB, Base_ON);

            uart_puts("Measurement stopped!\r\n");

            oled_clrscr();
            oled_charMode(DOUBLESIZE);
            oled_puts("BATT Meter");
            oled_drawLine(0, 15, 128, 15, WHITE); // oled_drawLine(x1, y1, x2, y2, color)

            oled_charMode(NORMALSIZE);
            oled_gotoxy(0, 5);
            oled_puts("Press GREEN to start!");
            oled_gotoxy(1, 6);
            oled_puts("Press RED to pause!");

            isStarted = 0;
        }

        if (isStarted == 0)
        {
            sprintf(cVolt, "Batt voltage: %.3f V", ADC_A0);
            oled_gotoxy(0, 3);
            oled_puts(cVolt);

            if (ADC_A0 <= 2.6)
            {
                oled_gotoxy(0, 7);  oled_puts("                     ");
                oled_gotoxy(1, 7);  oled_puts("Voltage is too low!");
            }
            else if (ADC_A0 >= 4.1)
            {
                oled_gotoxy(0, 7);  oled_puts("BATT is 100% charged!");
            }
            else
            {
                oled_gotoxy(0, 7);  oled_puts("                     ");
                oled_gotoxy(6, 7);  oled_puts("Battery OK!");
            }
            // uart_puts(cVolt);
            // uart_puts("\r\n");
        }

        if (isStarted == 1)
        {
            // Measurement logic //
            Voltage = ADC_A0 - 0.05;
            Current = Voltage / fabs(R_circ + R_bat);
                    
            // Start measuring time
            if ((TIM1_OVF_CNT == 3) & (R_bat == 0)) // If time == 4sec, measure dropped voltage
            {
                Voltage_dropped = Voltage;
                R_bat = (Voltage_unloaded - Voltage_dropped) / Current; // Calculate internal resistance (Voltage_unloaded - Voltage_dropped)/Current
            }

            // Capacity and Energy calculation
            if (Current_Time != TIM1_OVF_CNT)
            {
                Capacity_increment = (1000 * Current);             // [mAs]
                Capacity = Capacity + (Capacity_increment / 3600); // [mAh]

                Energy_increment = Capacity_increment * Voltage; // [mWs]
                Energy = Energy + (Energy_increment / 3600);     // [mWh]

                Current_Time = TIM1_OVF_CNT;

                if (Voltage <= 2.5)
                {
                    isStarted = 0;
                    uart_puts("Measurement finished!\r\n");

                    oled_clrscr();

                    GPIO_write_high(&PORTB, Base_ON);

                    while ((GPIO_read(&PIND, Stop_button) == 1) & (Capacity != 0.0))
                    {
                        oled_charMode(DOUBLESIZE);
                        oled_gotoxy(1, 0);
                        oled_puts("Finished!");
                        oled_drawLine(0, 15, 128, 15, WHITE);

                        oled_charMode(NORMALSIZE);
                        oled_gotoxy(0, 3);
                        oled_puts("IR:       _.___  mOhm");
                        oled_gotoxy(0, 4);
                        oled_puts("Capacity: _._     mAh");
                        oled_gotoxy(0, 5);
                        oled_puts("Energy:   _._     mWh");
                        oled_gotoxy(0, 7);
                        oled_puts("Press RED to return!");

                        oled_gotoxy(10, 3);
                        oled_puts(cIR);
                        oled_gotoxy(10, 4);
                        oled_puts(cCap);
                        oled_gotoxy(10, 5);
                        oled_puts(cEne);

                        oled_display();
                    }

                    oled_clrscr();

                    oled_charMode(DOUBLESIZE);
                    oled_puts("BATT Meter");
                    oled_drawLine(0, 15, 128, 15, WHITE);

                    oled_charMode(NORMALSIZE);
                    oled_gotoxy(0, 5);
                    oled_puts("Press GREEN to start!");
                    oled_gotoxy(1, 6);
                    oled_puts("Press RED to pause!");

                    oled_display();

                    R_bat = 0.0;
                    Capacity = 0.0;
                    Energy = 0.0;
                    continue;
                }

                // Displaying logic //
                if (floor(Voltage) == 0.0) // Is 0 V?
                {
                    oled_gotoxy(9, 2);
                    oled_putc(' '); // Don't print '-' sign
                    oled_gotoxy(9, 3);
                    oled_putc(' ');
                }
                else if ((Voltage * -1) == fabs(Voltage)) // Is negative?
                {
                    oled_gotoxy(9, 2);
                    oled_putc('-'); // Print '-' sign on a specific place
                    oled_gotoxy(9, 3);
                    oled_putc('-');
                }
                else // Is positive?
                {
                    oled_gotoxy(9, 2);
                    oled_putc(' '); // Clear '-' sign
                    oled_gotoxy(9, 3);
                    oled_putc(' ');
                }

                sprintf(cIR, "%.3f", R_bat * 1000); // internal resistance print is in mOhm
                sprintf(cVolt, "%.3f", fabs(Voltage));
                sprintf(cCurr, "%.3f", fabs(Current));
                sprintf(cCap, "%.1f", Capacity);
                sprintf(cEne, "%.1f", Energy);

                oled_gotoxy(10, 0);
                oled_puts(cIR);
                oled_gotoxy(10, 2);
                oled_puts(cVolt);
                oled_gotoxy(10, 3);
                oled_puts(cCurr);
                oled_gotoxy(10, 4);
                oled_puts(cCap);
                oled_gotoxy(10, 5);
                oled_puts(cEne);

                uart_puts("IR: ");
                uart_puts(cIR);
                uart_puts(" mOhm\r\n");
                uart_puts("Voltage: ");
                uart_puts(cVolt);
                uart_puts(" V\r\n");
                uart_puts("Current: ");
                uart_puts(cCurr);
                uart_puts(" A\r\n");
                uart_puts("Capacity: ");
                uart_puts(cCap);
                uart_puts(" mAh\r\n");
                uart_puts("Energy: ");
                uart_puts(cEne);
                uart_puts(" mWh\r\n");
            }
        }

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
    TIM1_OVF_CNT++;        // TIM1_OVF_1SEC
}

/**********************************************************************
 * Function: ADC complete interrupt
 * Purpose:  Convert ADC value to 5V AVCC reference
 **********************************************************************/
ISR(ADC_vect)
{
    static uint8_t Vref = 5;      // Voltage reference for normalization in ADC ISR
    ADC_A0 = Vref * ADC / 1023.0; // ADC channel A0
}