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
#include <batterymeter.h>

/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Use Timer/Counter1 and start ADC conversion every 100 ms.
 *           When AD conversion ends, send converted value to LCD screen.
 * Returns:  none
 **********************************************************************/

// #ifndef F_CPU
// #define F_CPU 16000000 // CPU frequency in Hz required for UART_BAUD_SELECT

// #endif

// Global vars
volatile float ADC_A0;          // Analog pin A0 voltage
volatile uint16_t TIM1_OVF_CNT; // Timer1 overflow counter


int main(void)
{
    batterymeter_init();

    batterymeter_change_scr(1);

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

    // char cVolt[8];
    // char cCurr[8];
    // char cIR[8];
    // char cCap[8];
    // char cEne[8];

    // Infinite loop
    while (1)
    {
        if ((GPIO_read(&PIND, Start_button) == 0) & (isStarted == 0)) // Green Button
        {
            uart_puts("Measurement started!\r\n");

            batterymeter_change_scr(2);

            Voltage_unloaded = ADC_A0; // Snapshot of unloaded voltage of battery
            
            batterymeter_uart_puts(Voltage_unloaded, "Unloaded Voltage: %.3f V");
            
            // sprintf(cVolt, "%.3f", Voltage_unloaded);

            // uart_puts("Unloaded Voltage: ");
            // uart_puts(cVolt);
            // uart_puts("\r\n");

            GPIO_write_low(&PORTB, Base_ON);
            TIM1_OVF_CNT = 0; // Reset timer overflow counter
            isStarted = 1;    // Set measurement start flag to 1
        }

        if ((GPIO_read(&PIND, Stop_button) == 0) & (isStarted == 1)) // Red Button
        {
            GPIO_write_high(&PORTB, Base_ON);

            uart_puts("Measurement stopped!\r\n");

            batterymeter_change_scr(1);

            isStarted = 0;
        }

        if (isStarted == 0)
        {
            // sprintf(cVolt, "Batt voltage: %.3f V", ADC_A0);
            // oled_gotoxy(0, 3);
            // oled_puts(cVolt);
            
            batterymeter_write_var(0, 3, ADC_A0, "Batt voltage: %.3f V");

            if (ADC_A0 <= 2.6)
            {
                // oled_gotoxy(0, 7);  oled_puts("                     ");
                // oled_gotoxy(1, 7);  oled_puts("Voltage is too low!");
                
                batterymeter_clear_line(7);
                batterymeter_write_line(1,7,"Voltage is too low!");

            }
            else if (ADC_A0 >= 4.1)
            {
                // oled_gotoxy(0, 7);  oled_puts("BATT is 100% charged!");

                batterymeter_clear_line(7);
                batterymeter_write_line(0,7,"BATT is 100% charged!");
            }
            else
            {
                // oled_gotoxy(0, 7);  oled_puts("                     ");
                // oled_gotoxy(6, 7);  oled_puts("Battery OK!");
                
                batterymeter_clear_line(7);
                batterymeter_write_line(6,7,"Battery OK!");
            }

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
                    uart_puts("Measurement finished!\r\n");

                    // isStarted = 0;
                    // oled_clrscr();
                    // GPIO_write_high(&PORTB, Base_ON);
                    
                    batterymeter_stop_measure(&isStarted);

                    if (Capacity != 0.0)
                    {
                        batterymeter_change_scr(3);

                        batterymeter_write_var(10,3,R_bat*1000,"%.3f");
                        batterymeter_write_var(10,4,Capacity,"%.1f");
                        batterymeter_write_var(10,5,Energy,"%.1f");

                        // oled_gotoxy(10, 3);
                        // oled_puts(cIR);
                        // oled_gotoxy(10, 4);
                        // oled_puts(cCap);
                        // oled_gotoxy(10, 5);
                        // oled_puts(cEne);

                        oled_display();

                        while (GPIO_read(&PIND, Stop_button) == 1)
                        {
                            uart_puts("Waiting for Stop button...\r\n");
                        }

                    }

                    batterymeter_change_scr(1);
                    oled_display();
                    
                    R_bat = 0.0;
                    Capacity = 0.0;
                    Energy = 0.0;
                    continue;

                    // while ((GPIO_read(&PIND, Stop_button) == 1) & (Capacity != 0.0))
                    // {
                        // oled_charMode(DOUBLESIZE);
                        // oled_gotoxy(1, 0);
                        // oled_puts("Finished!");
                        // oled_drawLine(0, 15, 128, 15, WHITE);

                        // oled_charMode(NORMALSIZE);
                        // oled_gotoxy(0, 3);
                        // oled_puts("IR:       _.___  mOhm");
                        // oled_gotoxy(0, 4);
                        // oled_puts("Capacity: _._     mAh");
                        // oled_gotoxy(0, 5);
                        // oled_puts("Energy:   _._     mWh");
                        // oled_gotoxy(0, 7);
                        // oled_puts("Press RED to return!");

                        // oled_gotoxy(10, 3);
                        // oled_puts(cIR);
                        // oled_gotoxy(10, 4);
                        // oled_puts(cCap);
                        // oled_gotoxy(10, 5);
                        // oled_puts(cEne);

                        // oled_display();
                    // }

                    // oled_clrscr();

                    // oled_charMode(DOUBLESIZE);
                    // oled_puts("BATT Meter");
                    // oled_drawLine(0, 15, 128, 15, WHITE);

                    // oled_charMode(NORMALSIZE);
                    // oled_gotoxy(0, 5);
                    // oled_puts("Press GREEN to start!");
                    // oled_gotoxy(1, 6);
                    // oled_puts("Press RED to pause!");

                    // oled_display();

                    // R_bat = 0.0;
                    // Capacity = 0.0;
                    // Energy = 0.0;
                    // continue;
                }

                // Displaying logic, minus sign position correction //
                if (floor(Voltage) == 0.0) // Is 0 V?
                {
                    // oled_gotoxy(9, 2);
                    // oled_putc(' '); // Don't print '-' sign
                    // oled_gotoxy(9, 3);
                    // oled_putc(' ');
                    
                    // Don't print '-' sign
                    batterymeter_write_line(9,2," ");
                    batterymeter_write_line(9,3," ");
                }
                else if ((Voltage * -1) == fabs(Voltage)) // Is negative?
                {
                    // oled_gotoxy(9, 2);
                    // oled_putc('-'); // Print '-' sign on a specific place
                    // oled_gotoxy(9, 3);
                    // oled_putc('-');

                    // Print '-' sign on a specific place
                    batterymeter_write_line(9,2,"-");
                    batterymeter_write_line(9,3,"-");
                }
                else // Is positive?
                {
                    // oled_gotoxy(9, 2);
                    // oled_putc(' '); // Clear '-' sign
                    // oled_gotoxy(9, 3);
                    // oled_putc(' ');

                    // Clear '-' sign
                    batterymeter_write_line(9,2," ");
                    batterymeter_write_line(9,3," ");
                }

                // sprintf(cIR, "%.3f", R_bat * 1000); // internal resistance print is in mOhm
                // sprintf(cVolt, "%.3f", fabs(Voltage));
                // sprintf(cCurr, "%.3f", fabs(Current));
                // sprintf(cCap, "%.1f", Capacity);
                // sprintf(cEne, "%.1f", Energy);

                // oled_gotoxy(10, 0);
                // oled_puts(cIR);
                // oled_gotoxy(10, 2);
                // oled_puts(cVolt);
                // oled_gotoxy(10, 3);
                // oled_puts(cCurr);
                // oled_gotoxy(10, 4);
                // oled_puts(cCap);
                // oled_gotoxy(10, 5);
                // oled_puts(cEne);

                // uart_puts("IR: ");      uart_puts(cIR);     uart_puts(" mOhm\r\n");
                // uart_puts("Voltage: "); uart_puts(cVolt);   uart_puts(" V\r\n");
                // uart_puts("Current: "); uart_puts(cCurr);   uart_puts(" A\r\n");
                // uart_puts("Capacity: ");uart_puts(cCap);    uart_puts(" mAh\r\n");
                // uart_puts("Energy: ");  uart_puts(cEne);    uart_puts(" mWh\r\n");

                batterymeter_write_var(10,0,R_bat * 1000,"%.3f");
                batterymeter_write_var(10,2,fabs(Voltage),"%.3f");
                batterymeter_write_var(10,3,fabs(Current),"%.3f");
                batterymeter_write_var(10,4,Capacity,"%.1f");
                batterymeter_write_var(10,5,Energy,"%.1f");
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