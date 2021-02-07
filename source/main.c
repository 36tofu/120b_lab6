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

/*
unsigned char tmpA = 0x00;
unsigned char tmpD = 0x01;
unsigned char cnt = 0x00;
enum states {START, COUNT, ENTER_COUNT_HOLD, COUNT_HOLD, ENTER_COUNT} state; 
void TickFct(){
	// sample PA0
	tmpA = ~PINA & 0x01;
	switch(state){
		case START:
			state = COUNT;
			break;
		case COUNT:
			if(tmpA == 1) // button pressed
				state = ENTER_COUNT_HOLD;
			else 
				state = COUNT;
			break;
		case ENTER_COUNT_HOLD:
			if(tmpA == 0) // button release
				state = COUNT_HOLD;
			else 
				state = ENTER_COUNT_HOLD;
			break;
		case COUNT_HOLD:
			if(tmpA == 1) // button pressed again, restart blink
				state = ENTER_COUNT;
			else 
				state = COUNT_HOLD;
			break;
		case ENTER_COUNT:
			if(tmpA == 0) // button release
				state = COUNT;
			else 
				state = ENTER_COUNT;
			break;
		default:
			break;
	}

	// output pattern according to cnt state
	switch(cnt){
		case 0: tmpD = 0x01; break;  
		case 1: tmpD = 0x02; break;  
		case 2: tmpD = 0x04; break;  
		case 3: tmpD = 0x02; break;  
		default: tmpD = 0x01; break;  
	}

	// state count 0~3
	switch(state){
		case START:
			break;
		case COUNT:
			if(cnt < 3) cnt += 1; else cnt = 0;
			break;
		case ENTER_COUNT_HOLD:
			break;
		case COUNT_HOLD:
			break;
		case ENTER_COUNT:
			if(cnt < 3) cnt += 1; else cnt = 0;
			break;
		default:
			break;
	}
}
//********************************************************************
*/



enum states { start, WAITRISE, INC, DEC, WAITFALL, RESET } state;

unsigned char tmpD;
unsigned char tmpA;

void tick()
{
	tmpA = (~PINA) & 0x03;
	switch(state){
		case start:
			state = WAITRISE;
			break;
		case WAITRISE:
			if(tmpA == 0)
				state = WAITRISE;
			else if(tmpA == 1)
				state = INC;
			else if(tmpA == 2)
				state = DEC;
			break;
		case INC:
			state = WAITFALL;
			break;
		case DEC:
			state = WAITFALL;
			break;
		case WAITFALL:
			if((tmpA == 1)||(tmpA == 2))
				state = WAITFALL;
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
			state = start;
			break;
		
	}
	//state actions
	switch(state){
		case start:
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
