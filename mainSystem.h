/* 
 * File:   mainSystem.h
 * Author: jcalb
 *
 * Created on December 3, 2017, 3:30 AM
 */

#ifndef MAINSYSTEM_H
#define	MAINSYSTEM_H

#ifdef	__cplusplus
extern "C" {
#endif

#define _XTAL_FREQ 4000000UL
#define CLR_DISP 0b00000001 //Clear LCD display

#define freqA 196
#define freqB 196

#define x 2 //total number of notes in song to be played - modify for specific song

#define maxTmr1Val 65535
    
//#define set rows and columns ports 
#define row1Port LATCbits.LATC0
#define row2Port LATCbits.LATC1
#define row3Port LATDbits.LATD3
#define row4Port LATCbits.LATC3
#define col1Port PORTCbits.RC4
#define col2Port PORTCbits.RC5
#define col3Port PORTCbits.RC6
#define col4Port PORTCbits.RC7   //if a 4x4 keypad is used
    
 /*Function Prototypes*/

void highISR(void);
void initLCD(void);
void prtStrLitLCD(int lineNum, rom const char * string);
void prtStrLCD(int lineNum, char * string);
void initPorts(void);
void initTimers(void);
void initInterrupts(void);
void initInterrupts(void);
void intitPWM(void);
void intitADC(void);
int getKeyPress(void);

/*Function Prototypes*/   
    
    
#ifdef	__cplusplus
}
#endif

#endif	/* MAINSYSTEM_H */

