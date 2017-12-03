#include <p18f452.h>
#include <timers.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <delays.h>
#include <pwm.h>
#include <capture.h>
#include <adc.h>
#include <xlcd_grp.h>
#include <ow.h>
#include <math.h>
#include <mainSystem.h>
#include <variableDef.h>



/* Set configuration bits for use with ICD2 / PICDEM2 PLUS Demo Board:
 * - set HS oscillator
 * - disable watchdog timer
 * - disable low voltage programming
 */
#pragma config OSC = HS
#pragma config WDT = OFF
#pragma config LVP = OFF

// CONFIG3H
#pragma config CCP2MUX = OFF    // CCP2 Mux bit (CCP2 input/output is multiplexed with RB3)

void tmr0Handler()
{
	tenCount = tenCount + 1;
	if (tenCount == 10) {
		CloseTimer0();
		bpmObtain = 1;
		INTCONbits.INT0E = 0; // disables the INT0 external interrupt
		INTCONbits.TMR0IF = 0;
	} else {
		INTCONbits.TMR0IF = 0;
		WriteTimer0(0xBDC);
	}
}

void externHandler()
{
	LATAbits.LATA1 = 1;
	risEdg = risEdg + 1;
	INTCONbits.INT0F = 0;
}

void tmr1Handler()
{
	PIR1bits.TMR1IF = 0;
    ovrFlwCount++;
}

void captureHandler()
{
	float tempVal= 0;
	LATAbits.LATA0 = 1;
	PIR1bits.CCP1IF = 0;
	if (peak == 0) {
		if (indicator == 0) {
			cptOut1 = ReadCapture1();
		} else {
			cptOut2 = ReadCapture1();
		}
	} else if ((peak == 1) && (indicator == 0)) {
		cptOut2 = ReadCapture1();
	}
	peak++;
	if ((peak > 1) || (indicator == 1)) {
		indicator = 1;
		interval = maxTmr1Val * ovrFlwCount + cptOut2 - cptOut1;
		nn++;
		prevTime = cptOut2;
		cptOut1 = prevTime;
		tempVal = (float) interval / (float) 1000;
		if ((float) tempVal > (float) 50) {
			nn_50++;
		}
		peak = 0;
		if (nn > 15) {
			hrv = (float) nn_50 / (float) 15;
			hrv = hrv * 100;
			CloseCapture1();
			CloseTimer1();
		}
	}
}

void adc_handler()
{
	PIR1bits.ADIF = 0;
        
	if(adcMode1 == '0' && adcMode2 == '1') {
		LATAbits.LATA0 = 1;

		result = ReadADC(); // Read result
		voltage = ((result * 5) / (float) 1023);
		if(voltage >3) {
			voltage = voltage / (float) 2.1;
			glseLevel = (log(voltage));
			value = (3377 * 0.00001)*(2817 * 0.0001);
			glseLevel = glseLevel / (float) (value);
			integerPart = (int) glseLevel;
			decimalPart = (glseLevel - integerPart) * (float) 10;
			sprintf(adcBuffer, (far char*) "Level:%d mg/dL", integerPart);
			prtStrLCD(3, adcBuffer);
		}            
	}
	if(adcMode1 == '1' && adcMode2 == '0'){
		
		result = ReadADC(); // Read result
		voltage = ((result * 5) / (float) 1023);
		integerPart = (int) voltage;
		decimalPart = (voltage - integerPart) * (float) 10;
		if(integerPart < 4){
			prtStrLitLCD(3,"Saving Data....");
			prtStrLitLCD(4,"Entering Rest....");

			LATAbits.LATA0 = 1;
			LATAbits.LATA1 = 1;
			
			Delay1KTCYx(100);
		   
			LATAbits.LATA0 = 0;
			
			CloseADC();
			PIE1bits.CCP1IE = 0; // CCP1 interrupt disable 
			INTCONbits.INT0E = 0; // disable the INT0 external interrupt
			INTCONbits.GIEH = 0;
			INTCONbits.GIE = 0; // disable interrupts 
			
			Sleep();
		}
	}
}
	
#pragma code
/*****************High priority ISR **************************/
#pragma interrupt highISR

void highISR(void) {

    if (INTCONbits.TMR0IF == 1) { // Interrupt Check 
		tmr0Handler();
    }
    if (INTCONbits.INT0F == 1) {
		externHandler();
    }

    if (PIR1bits.TMR1IF == 1) {
		tmr1Handler();
    }
    if (PIR1bits.CCP1IF == 1) {
		captureHandler();
    }
    if(PIR1bits.ADIF == 1){
        adc_handler();
    }
}

/*****************High priority interrupt vector **************************/
#pragma code high_vector=0x08

void interrupt_at_high_vector(void) {
    _asm
    GOTO highISR
    _endasm
}

void DelayFor18TCY(void) {
    Delay10TCYx(20); //delays 20 cycles
    return;
}

void DelayPORXLCD(void) // minimum 15ms
{
    Delay1KTCYx(15);
    return;
}

void DelayXLCD(void) // minimum 5ms
{
    Delay1KTCYx(5);
    return;
}

int getKeyPress() {
    // This routine returns the first key found to be pressed during the scan.
    char key = 0, row;
    for (row = 0b00000001; row < 0b00010000; row <<= 1) {
        { // turn on row output
            row1Port = (row & 0x0001) >> 0;
            row2Port = (row & 0x0002) >> 1;
            row3Port = (row & 0x0004) >> 2;
            row4Port = (row & 0x0008) >> 3;
            Delay100TCYx(0xF6);
        }
        // read colums - break when key press detected
        if (col1Port)break;
        key++;
        if (col2Port)break;
        key++;
        if (col3Port)break;
        key++;
        if (col4Port)break;
        key++;
    }
    row1Port = 0;
    row2Port = 0;
    row3Port = 0;
    row4Port = 0;
    if (key != old_key) {
        old_key = key;
        return keyPadMatrix[ key ];
    } else {
        return keyPadMatrix[ 0x10 ];
    }
}

int getBpmValue() {
    int val = 0;
    val = heartBeat * 6;
    return val;
}

int getHrvValue() {
    int hrvValue = 0;
    hrvValue = hrv;
    return hrvValue;
}

void displayBpmValue(int bpmVal) {
    char buffer[80];
    sprintf(buffer, "HR: %d bpm", bpmVal);
    prtStrLCD(1, buffer);
}

void displayHrvValue(int hrvVal) {
    char buffer[16];
    sprintf(buffer, "HRV: %d%", hrvVal);
    prtStrLCD(2, buffer);
}

void restValues(){
    cptOut1=0,cptOut2=0,peak=0,interval=0,
	ovrFlwCount=0,prevTime=0,indicator=0,nn=0,nn_50=0;
    tenCount = 0,heartBeat = 0,bpmObtain = 0,risEdg = 0;
	msbTmpy = 0,lsbTmpy = 0;intPart = 0;
	fFracPart = 0.0000;
	sign = 0;
	iFracPart =0;
	cnvCnt = 0;
}

void setTone(int i){
    OpenPWM2(song[i]);;//set PWM frequency according to entries in song array
    Delay1KTCYx(400*length[i]); //each note is played for 400ms*relative length
    OpenPWM2(1); //the PWM frequency set beyond audible range                         
    //in order to create a short silence between notes
    Delay10KTCYx(25);   //the silence is played for 50 ms

}

void readDallas(){
	//Read Dallas 1822P
	ow_reset(); //reset device
	ow_write_byte(0xCC); //skip ROM check
	ow_write_byte(0xBE); //Send read scratchpad on 1822P
	tmpyLSB = ow_read_byte(); //Read first byte, LS and store in tmpyLSB
	tmpyMSB = ow_read_byte(); //Read first byte, MS and store in tmpyMSB
}

void acqInt(){
	
	int tempV;
	//Acquire Integer
	lsbTmpy = tmpyLSB >> 4; 
	msbTmpy = tmpyMSB << 4;
    //tempV = intPart;
	intPart = msbTmpy | lsbTmpy;
    if(intPart > 30 || intPart < 20 )
        intPart = tempV;
}

void acqFract(){
	//Acquire Fraction
	if(tmpyLSB & 0x01){fFracPart += 0.0625;}
	if(tmpyLSB & 0x02){fFracPart += 0.125;}
	if(tmpyLSB & 0x04){fFracPart += 0.25;}
	if(tmpyLSB & 0x08){fFracPart += 0.5;}
	iFracPart =fFracPart*1000;
}

void dspTemp(){

	sprintf(tempResult,"Temp: +%d.%03d%cC",intPart,iFracPart,degree);
	//intPart = 0;
	iFracPart= 0;
	fFracPart =0.0;

}

void getTempVal(){
	int sumIntPart = 0;
	int sumFraPart = 0;
	int i;
	for(i = 0;i < 16;i++){ 
		ow_reset();  //reset 1822P
		ow_write_byte(0xCC); // Skip ROM Check
		ow_write_byte(0x44); //Temperature conversion 
		
		PORTBbits.RB2 = 1;  //Strong pullup to provide current that parasitic capacitance can't provide
		
		for(cnvCnt = 1; cnvCnt<=8;cnvCnt++){ //800ms (750ms is recommended conversion time))
		   Delay1KTCYx(100);
		}
		PORTBbits.RB2 = 0; //Turn off strong pullup
		
		readDallas();
		acqInt();
		sumIntPart = sumIntPart + intPart;
		acqFract();
		sumFraPart = sumFraPart + iFracPart;
	}
	intPart = sumIntPart/16;
	iFracPart = sumFraPart/16;
	dspTemp();
}


void mainSystem(char keyPress) {
    int bpmVal = 0;
    int hrvVal = 0;
    int i = 0;

    restValues();
    
    initTimers();
    initInterrupts();
    intitADC();

    WriteCmdXLCD(CLR_DISP);
    while (1) {
        Delay1KTCYx(100);     // Delay for 50TCY
        ConvertADC();
        SetDCPWM2(0);
        LATAbits.LATA0 = 0;
        LATAbits.LATA1 = 0;
                
		getTempVal();
        prtStrLCD(4,tempResult);//Write top line value
        sprintf(tempResult,"                   ");	
		
        if (bpmObtain == 0) {
            heartBeat = risEdg;
            displayBpmValue(0);
            displayHrvValue(0);

        }
        else 
        {
            CloseTimer0();
            bpmVal = getBpmValue();
            if(bpmVal > 120 || bpmVal  < 60){
                if(i == 2){
                    i = 0;
                }
                CloseADC();
                intitPWM();
                setTone(i);
                i++;
            }
            hrvVal = getHrvValue();
            displayBpmValue(bpmVal);
            displayHrvValue(hrvVal);

        }
        keyPress = getKeyPress();
        if(keyPress == 'B'){
            SetDCPWM2(0);
            PIE1bits.CCP1IE = 0; // CCP1 interrupt disable 
            INTCONbits.INT0E = 0; // disable the INT0 external interrupt
            INTCONbits.GIEH = 0;
            INTCONbits.GIE = 0; // disable interrupts 
            break;
        }
    }
}

void heathSystem(char keyPress) {
        
    while (1) {
        adcMode1 = '0';
        adcMode2 = '1';
        
        keyPress = getKeyPress();
        if (keyPress != 0xFF){
            switch(keyPress){
                case 'B':
                    mainSystem(keyPress);
                    break; 
            }
        }else{
            WriteCmdXLCD(CLR_DISP);
            prtStrLitLCD(1,"To Start");
            prtStrLitLCD(2,"Press B");
            prtStrLitLCD(3,"To Exit");
            prtStrLitLCD(4,"Press F");
        }
        keyPress = getKeyPress();
        if(keyPress == 'F'){
            break;
        }
    }
}

void intervalSys(char keyPress){
    char keypress;
    char typedKey[17];
    int k =0;
    memset(typedKey, ' ', 16);
    typedKey[16] = '\0';
    
    while (1) {
        keypress = getKeyPress();
        if (keypress != 0xFF && k <17) {
            if((keypress>'9')||(keypress<'0')){
                if(keypress == 'A')
                    break;
            }else{
                typedKey[k] = keypress;
                k++;
            }
        }else if(k ==17){
            WriteCmdXLCD(CLR_DISP);
            memset(typedKey, ' ', 16                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           );
            typedKey[16] = '\0';
            k=0;
        } else {
            WriteCmdXLCD(CLR_DISP);
            prtStrLitLCD(1,"Enter Interval");
            prtStrLitLCD(2,"Press A to save");
            prtStrLCD(3,typedKey);
        }
    }

}

void brownOutTest(char keyPress){

    int bpmVal = 0;
    int hrvVal = 0;
    int i = 0;

    
    restValues();
    
    initTimers();
    initInterrupts();
    
    LATAbits.LATA1 = 0;
    
    OpenADC(ADC_FOSC_RC&ADC_RIGHT_JUST&ADC_8ANA_0REF,ADC_CH4&ADC_INT_ON);
    WriteCmdXLCD(CLR_DISP);
    while(1){
        adcMode1 = '1';
        adcMode2 = '0';
        
        Delay1KTCYx(100);     // Delay for 50TCY
        ConvertADC();
        LATAbits.LATA0 = 0;
        if (bpmObtain == 0) {
            heartBeat = risEdg;
            displayBpmValue(0);
            displayHrvValue(0);
        }
        else 
        {
            CloseTimer0();
            bpmVal = getBpmValue();
            if(bpmVal > 120 || bpmVal  < 60){
                if(i == 2){
                    i = 0;
                }
                CloseADC();
                intitPWM();
                setTone(i);
                i++;
            }
            hrvVal = getHrvValue();
            displayBpmValue(bpmVal);
            displayHrvValue(hrvVal);

        }
        keyPress = getKeyPress();
        if(keyPress == 'C'){
            SetDCPWM2(0);
            break;
        }
        
    }

}

void main() {
    char keyPress;
    char typedKey[17];
    memset(typedKey, ' ', 16);
    typedKey[16] = '\0';
    
    initPorts();
    initLCD();
    
    while (1) {
        keyPress = getKeyPress();
        if (keyPress != 0xFF){
            switch(keyPress){
                case 'A':
                    heathSystem(keyPress);
                    break;
                case 'B':
                    intervalSys(keyPress);
                    break;
                case 'C':
                    brownOutTest(keyPress);
                    break;
            }
        }else{
            WriteCmdXLCD(CLR_DISP);
            prtStrLitLCD(1,"Grp Prj Push & Pop");                    
            prtStrLitLCD(2,"A->Health System");
            prtStrLitLCD(3,"B->Entr Intervls");
            prtStrLitLCD(4,"C->BrownOut Test");

        }
    }
    Sleep();
}