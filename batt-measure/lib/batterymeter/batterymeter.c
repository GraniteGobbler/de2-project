#include "batterymeter.h"

// Funcs
void batterymeter_init()
{
    isStarted = 0;
    Current_Time = 0;

    Voltage = 0.0;
    Voltage_unloaded = 0.0;
    Voltage_dropped = 0.0;

    Current = 0.0;

    Capacity = 0.0;           //  [mAh]
    Capacity_increment = 0.0; //  [mAs]

    Energy = 0.0;           // [mWh]
    Energy_increment = 0.0; //  [mWs]

    R_circ = 1.11265; // Total circuit resistance
    R_bat = 0.0;      // Internal resistance of battery

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
        
        uart_puts("Screen 1\r\n");  

        oled_clrscr();

        oled_charMode(DOUBLESIZE);
        oled_puts("BATT Meter");
        oled_drawLine(0, 15, 128, 15, WHITE);

        oled_charMode(NORMALSIZE);
        oled_gotoxy(0, 5);  oled_puts("Press GREEN to start!");
        oled_gotoxy(1, 6);  oled_puts("Press RED to pause!");
        
        break;

    case 2:

        uart_puts("Screen 2\r\n");

        oled_clrscr();

        oled_charMode(NORMALSIZE);
        oled_gotoxy(0, 0);  oled_puts("IR:       _.___  mOhm");
        oled_gotoxy(0, 2);  oled_puts("Voltage:  _.___   V");
        oled_gotoxy(0, 3);  oled_puts("Current:  _.___   A");
        oled_gotoxy(0, 4);  oled_puts("Capacity: _._     mAh");
        oled_gotoxy(0, 5);  oled_puts("Energy:   _._     mWh");

        break;

    case 3:

        uart_puts("Screen 3\r\n");

        oled_clrscr();
        
        oled_charMode(DOUBLESIZE);
        oled_gotoxy(1, 0);
        oled_puts("Finished!");
        oled_drawLine(0, 15, 128, 15, WHITE);

        oled_charMode(NORMALSIZE);
        oled_gotoxy(0, 3);  oled_puts("IR:       _.___  mOhm");
        oled_gotoxy(0, 4);  oled_puts("Capacity: _._     mAh");
        oled_gotoxy(0, 5);  oled_puts("Energy:   _._     mWh");
        oled_gotoxy(0, 7);  oled_puts("Press RED to return!");

        break;

    default:

        uart_puts("Screen 1\r\n");  

        oled_clrscr();

        oled_charMode(DOUBLESIZE);
        oled_puts("BATT Meter");
        oled_drawLine(0, 15, 128, 15, WHITE);

        oled_charMode(NORMALSIZE);
        oled_gotoxy(0, 5);  oled_puts("Press GREEN to start!");
        oled_gotoxy(1, 6);  oled_puts("Press RED to pause!");
        
        break;
    } 

}

void batterymeter_write_var(unsigned int x, unsigned int y, float value, char* string)
{
    char strOut[32];

    sprintf(strOut, string, value);
    oled_gotoxy(x,y);   oled_puts(strOut);

    uart_puts(strOut);
    uart_puts("\r\n");

}

void batterymeter_write_line(unsigned int x, unsigned int y, char* string)
{
    oled_gotoxy(x,y);   oled_puts(string);

    uart_puts(string);
    uart_puts("\r\n");
}

void batterymeter_uart_puts(float value, char* string)
{
    char strOut[32];
    
    sprintf(strOut, string, value);
    uart_puts(strOut);

}

void batterymeter_clear_line(unsigned int y)
{
    oled_gotoxy(0,y);  oled_puts("                     ");
}

void batterymeter_stop_measure(volatile uint8_t* flag)
{
    *flag = 0;
    oled_clrscr();
    GPIO_write_high(&PORTB, Base_ON);
}