/* 
 * File:   variableDef.h
 * Author: jcalb
 *
 * Created on December 3, 2017, 4:01 AM
 */

#ifndef VARIABLEDEF_H
#define	VARIABLEDEF_H

#ifdef	__cplusplus
extern "C" {
#endif

 
 /*Variables Definitions*/
char key, old_key;

char const keyPadMatrix[] ={ 
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '0', 'F', 'E', 'D',
    0xFF
};//matrix for the keypad

/*HR & HRV*/
int tenCount = 0, heartBeat = 0, bpmObtain = 0, risEdg = 0;
unsigned int cptOut1 = 0, cptOut2 = 0, peak = 0, interval = 0, 
        ovrFlwCount = 0, prevTime = 0, 
        indicator = 0, fin = 0, nn = 0, nn_50 = 0;
float hrv = 0;

/*Alert Related*/
int song[x]={freqA, freqB}; //insert notes of song in array
int length[x]={1, 1}; //relative length of each note
int i;

/*ADC Related*/
float result;
float voltage,adcRlts;
int integerPart, decimalPart;
char adcBuffer[8];
float glseLevel ;//glucose level
float value;

/*Temp sensor Related*/
unsigned char tmpyMSB,tmpyLSB,degree = 0xDF;
unsigned int msbTmpy = 0,lsbTmpy = 0,intPart = 0;
float fFracPart = 0.0000;
int sign = 0;
int iFracPart =0;
int cnvCnt = 0;
char tempResult[20];

/*Brownout Testing*/
char adcMode1;
char adcMode2;

/*Variables Definitions*/ 


#ifdef	__cplusplus
}
#endif

#endif	/* VARIABLEDEF_H */

