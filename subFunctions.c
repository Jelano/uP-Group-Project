#include <mainSystem.h>
#include "xlcd_grp.h"
#include <timers.h>
#include <adc.h>
#include <delays.h>
#include <pwm.h>


void initLCD(void) {
    OpenXLCD(FOUR_BIT & LINES_5X7);
    while (BusyXLCD());
    WriteCmdXLCD(FOUR_BIT & LINES_5X7);
    while (BusyXLCD());
    WriteCmdXLCD(BLINK_ON);
    while (BusyXLCD());
    WriteCmdXLCD(CLR_DISP);
    while (BusyXLCD());
}

//Run initLCD(); before running this 

void prtStrLitLCD(int lineNum, rom const char * string) {
    if (lineNum == 1) {
        SetDDRamAddr(0x00);
        while (BusyXLCD());
        putrsXLCD(string);
        while (BusyXLCD());
    } else if (lineNum == 2) {
        SetDDRamAddr(0x40);
        while (BusyXLCD());
        putrsXLCD(string);
        while (BusyXLCD());
    } else if (lineNum == 3) {
        SetDDRamAddr(0x10);
        while (BusyXLCD());
        putrsXLCD(string);
        while (BusyXLCD());
    } else if (lineNum == 4) {
        SetDDRamAddr(0x50);
        while (BusyXLCD());
        putrsXLCD(string);
        while (BusyXLCD());
    } else {
        SetDDRamAddr(0x00);
        while (BusyXLCD());
        putrsXLCD("ERROR: line #");
        while (BusyXLCD());
    }
}

//Run initLCD(); before running this 

void prtStrLCD(int lineNum, char * string) {
    if (lineNum == 1) {
        SetDDRamAddr(0x00);
        while (BusyXLCD());
        putsXLCD(string);
        while (BusyXLCD());
    } else if (lineNum == 2) {
        SetDDRamAddr(0x40);
        while (BusyXLCD());
        putsXLCD(string);
        while (BusyXLCD());
    } else if (lineNum == 3) {
        SetDDRamAddr(0x10);
        while (BusyXLCD());
        putsXLCD(string);
        while (BusyXLCD());
    } else if (lineNum == 4) {
        SetDDRamAddr(0x50);
        while (BusyXLCD());
        putsXLCD(string);
        while (BusyXLCD());
    } else {
        SetDDRamAddr(0x00);
        while (BusyXLCD());
        putrsXLCD("ERROR: line #");
        while (BusyXLCD());
    }
}

void initPorts() {
    PORTC = 0x00;
    LATC = 0x00;
    TRISC = 0xF4;
    TRISAbits.RA0 = 0;
    TRISAbits.RA1   = 0;
    TRISBbits.RB0 = 1;
    TRISBbits.CCP2 = 0;
	TRISBbits.RB2=0; //set RB2 as output to transistor
    PORTD = 0x00;
    TRISD = 0x00;
}

void initTimers() {
    INTCONbits.TMR0IE = 1; //Enable Timer0 Interrupt
    INTCONbits.TMR0IF = 0; //Clear Timer0 Interrupt Flag
    INTCON2bits.TMR0IP = 1; //Enable Priority Levels
    TMR1H = 0x00; // clear timer1 
    TMR1L = 0x00;
    T1CON = 0x81; // 10000001; Timer1 enabled; prescale 1:1 
    OpenTimer0(TIMER_INT_ON & T0_SOURCE_INT & T0_PS_1_16 & T0_16BIT);
}

void initInterrupts() {
    INTCONbits.INT0E = 1; // Enables the INT0 external interrupt
    INTCONbits.INT0F = 0; //The INT0 external interrupt occurred (must be cleared in software)   
    INTCON2bits.INTEDG0 = 1; //Interrupt on rising edge 
    PIE1bits.CCP1IE = 1; // CCP1 interrupt enabled 
    RCONbits.IPEN = 1; //Enable Priority Levels
    INTCONbits.GIEH = 1;
    INTCONbits.GIE = 1; // enable interrupts 
    INTCONbits.PEIE = 1;
    PIR1 = 0x00; // clear the interrupt flags 
    RCONbits.IPEN = 1; // interrupt priority enabled
    CCP1CON = 0x05; // 00000101; CCP1 as capture mode, every rising edge  
    IPR1 = 0x04; // 00000100; CCP1 interrupt set to high priority 
    PIE1bits.ADIE = 1; // ADC interrupt enabled 
    IPR1bits.ADIP = 1; //ADC interrupt set to high priority 
    PIR1bits.ADIF = 0;////Clear ADC Interrupt Flag
}

void intitPWM(){
    TRISBbits.CCP2 = 0; //Make CCP2 pin as output for RB3
    OpenTimer2(TIMER_INT_OFF & T0_PS_1_4 & T2_POST_1_1 );
    SetDCPWM2(30);
}

void intitADC(){
    OpenADC(ADC_FOSC_RC&ADC_RIGHT_JUST&ADC_8ANA_0REF,ADC_CH3&ADC_INT_ON);
}

