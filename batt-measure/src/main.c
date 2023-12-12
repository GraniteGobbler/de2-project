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

int main(void)
{
    batterymeter_init();

    batterymeter_change_scr(1);

    // Infinite loop
    while (1)
    {
        if ((GPIO_read(&PIND, Start_button) == 0) & (isStarted == 0)) // Green Button
        {
            uart_puts("Measurement started!\r\n");

            batterymeter_change_scr(2);

            Voltage_unloaded = ADC_A0; // Snapshot of unloaded voltage of battery
            
            batterymeter_uart_puts(Voltage_unloaded, "Unloaded Voltage: %.3f V");

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
            batterymeter_write_var(0, 3, ADC_A0, "Batt voltage: %.3f V");

            if (ADC_A0 <= 2.6)
            {
                batterymeter_clear_line(7);
                batterymeter_write_line(1,7,"Voltage is too low!");

            }
            else if (ADC_A0 >= 4.1)
            {
                batterymeter_clear_line(7);
                batterymeter_write_line(0,7,"BATT is 100% charged!");
            }
            else
            {
                batterymeter_clear_line(7);
                batterymeter_write_line(6,7,"Battery OK!");
            }

        }

        if (isStarted == 1)
        {
            // Measurement logic //
            Voltage = ADC_A0 - 0.05;
            Current = Voltage / (R_circ + fabs(R_bat));
                    
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
                    
                    batterymeter_stop_measure(&isStarted);

                    if (Capacity != 0.0)
                    {
                        batterymeter_change_scr(3);

                        batterymeter_write_var(10,3,R_bat*1000,"%.3f");
                        batterymeter_write_var(10,4,Capacity,"%.1f");
                        batterymeter_write_var(10,5,Energy,"%.1f");

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
                }

                // Displaying logic, minus sign position correction //
                if (floor(Voltage) == 0.0) // Is 0 V?
                {        
                    // Don't print '-' sign
                    batterymeter_write_line(9,2," ");
                    batterymeter_write_line(9,3," ");
                }
                else if ((Voltage * -1) == fabs(Voltage)) // Is negative?
                {
                    // Print '-' sign on a specific place
                    batterymeter_write_line(9,2,"-");
                    batterymeter_write_line(9,3,"-");
                }
                else // Is positive?
                {
                    // Clear '-' sign
                    batterymeter_write_line(9,2," ");
                    batterymeter_write_line(9,3," ");
                }

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