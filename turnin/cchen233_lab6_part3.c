/*
 *  Partner(s) Name:Christopher Chen
 *	Lab Section:21
 *	Assignment: Lab #6  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0; 
//TimerISR() sets this to 1. C programmer should clear to 0

//Internal variables for mapping AVR's ISR to our cleaner TimerISR model.

unsigned long _avr_timer_M = 1; //start count from here, down to 0. Dft 1ms
unsigned long _avr_timer_cntcurr = 0; //Current internal count of 1ms ticks

void TimerOn(){
	//AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B; //bit 3 = 0: CTC mode (clear timer on compare)
	//AVR output compare register OCR1A
	OCR1A = 125; // Timer interrupt will be generated when TCNT1 == OCR1A
	//AVR timer interrupt mask register
	TIMSK1 = 0x02; //bit1: OCIE1A -- enables compare match interrupt
	//Init avr counter
	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;
	//TimerISR will be called every _avr_timer_cntcurr ms
	
	//Enable global interrupts 
	SREG |= 0x80; //0x80: 1000000

}

void TimerOff(){
	TCCR1B = 0x00; //bit3bit1bit0 = 000: timer off
}

void TimerISR(){
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect){
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}




enum states { START, WAITRISE, INC, DEC, WAITFALL, RESET } state;

unsigned char tmpD;
unsigned char tmpA;
unsigned char count;

void tick()
{
	tmpA = (~PINA) & 0x03;
	switch(state){
		case START:
			state = WAITRISE;
			break;
		case WAITRISE:
			if(tmpA == 0)
				state = WAITRISE;
			else if(tmpA == 1)
				state = INC;
			else if(tmpA == 2)
				state = DEC;
			else if(tmpA == 3)
				state = RESET;
			break;
		case INC:
			state = WAITFALL;
			break;
		case DEC:
			state = WAITFALL;
			break;
		case WAITFALL:
			if(tmpA == 1){
				if(count == 0x0A){
					count = 0;
					state = INC;
				}
				else{ 	
					count += 1;
					state = WAITFALL;
				}
			}
			if(tmpA == 2){
				if(count == 0x0A){
					count = 0;
					state = DEC;
				}
				else{ 	
					count += 1;
					state = WAITFALL;
				}
			}
			else if(tmpA == 0)
				state = WAITRISE;
			else if(tmpA == 3)
				state = RESET;
			break;
		case RESET:
			if(tmpA == 0)
				state = WAITRISE;
			else
				state = RESET;
			break;
		default:
			state = START;
			break;
		
	}
	//state actions
	switch(state){
		case START:
			// tmpD = 0x07;
			break;
		case WAITRISE:
			break;
		case WAITFALL:
			break;
		case INC:
			if(tmpD < 0x09) tmpD = tmpD + 1;
			break;
		case DEC:
			if(tmpD > 0x00) tmpD = tmpD - 1;
			break;
		case RESET:
			tmpD = 0x00;
			break;
		default:
			break;
		
	}
	PORTD = tmpD;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00;
	DDRD = 0x0F;
	PORTA = 0x03;
	PORTD = 0x00;
	TimerSet(100);
	TimerOn();
	state = START;
	tmpD = 0x07;
    // SyncSM loop
	while (1) {
		tick();
		PORTD = tmpD;
		// Wait for timer interrupt
		while(!TimerFlag);
		TimerFlag = 0;
    }
    return 1;
}
