/*	Author: Ali Noor
 *  Partner(s) Name: 
 *	Lab Section: 21
 *	Assignment: Lab #6  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
*/

#include <avr/interrupt.h>
#include "io.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum States{start,init, increase, decrease, waitIncrease, waitDecrease, waitReset, reset} state;
volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;

	TIMSK1 = 0x02; 
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet (unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M; 
}

unsigned char cnt = 0x00;
unsigned char tmp = 0x00; 
unsigned char tmp1 = 0x00; 

void Tick() {
	
	tmp = ~PINA & 0x01;
	tmp1 = ~PINA & 0x02;

	switch(state) {
		case start :
			state = init; 
			break;
		case init :
			if(tmp && tmp1) {
				state = waitReset;
			}		
			else if(tmp) {
				state = increase;
			}
			else if(tmp1) { 
				state = decrease;
			}
			else {
				state = init;
			}
			break;
		case increase :
			state = init;
			break;
		case decrease : 
			state = init;
			break;
		case waitIncrease :
			if(tmp) {
				state = waitIncrease;
			}
			else {
				state = increase;
			}
			break;
		case waitDecrease :
			if(tmp1) {
				state = waitDecrease;
				break;
			}
			else {
				state = decrease;
			}
		case waitReset :
			if(tmp || !tmp) {
				state = reset;
			}
			else {
				state = waitReset;
			}
			break;
		case reset :
			state = init;
			break;
		default : 
			state = start;
			break;
	}
	
	switch(state) {
		case start :
			break;
		case init : 
			break;
		case increase :
			if(cnt < 0x09) {
				++cnt;
			}
			break;
		case decrease :
			if(cnt > 0x00) {
				--cnt;
			}
			break;
		case reset :
			cnt = 0x00;
			break;
		case waitIncrease :
			break;
		case waitDecrease :
			break;
		case waitReset :
			break;
		default : 
			break;
	}
	PORTC= cnt;
	LCD_WriteData(PORTC + '0');

}	

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00; 
	DDRD = 0xFF; PORTD = 0x00;
	
	TimerSet(10);
	TimerOn();

	state = start;
	LCD_init();
	LCD_Cursor(1);
		
	while(1) {
		LCD_ClearScreen();
		Tick();
	    	while(!TimerFlag) {};
	    	TimerFlag = 0;
    	}

}

