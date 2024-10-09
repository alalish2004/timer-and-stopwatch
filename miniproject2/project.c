/*
 * project.c
 *
 *  Created on: Sep 9, 2024
 *      Author: ALI
 */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

unsigned char seconds =0;

unsigned char minutes =0;

unsigned char hours =0;

unsigned char inc_dec=0;

unsigned char pressing_flag=0;

unsigned char hoursdecrement =0;
unsigned char hoursincrement =0;
unsigned char minutesdecrement =0;
unsigned char minutesincrement =0;
unsigned char secondsdecrement =0;
unsigned char secondsincrement =0;


void timer1_compare(void){
	TCCR1A |= (1<<FOC1A) | (1<<FOC1B);
	TCCR1B |= (1<<WGM12) | (1<<CS10) | (1<<CS12);
	TCNT1 = 0;
	OCR1A = 15624;
	TIMSK |= (1<<OCIE1A);
	SREG |= (1<<7);


}

void int0_init(void){
	GICR |= (1<<INT0);
	MCUCR |= (1<<ISC01) ;
	MCUCR &= ~(1<<ISC00);
	SREG |= (1<<7);
	// Clear the INT0 interrupt flag manually neacause its taking action on starting the simulation
	GIFR |= (1<<INTF0);

}

void int1_init(void){
	DDRD &= ~(1<<PD3); // input pin at interrupt 1
	GICR |= (1<<INT1);
	MCUCR |= (1<<ISC11) | (1<<ISC10);
	SREG |= (1<<7);
}

void int2_init(void){


	PORTB |= (1<<PB2);// set internal pull up resistor

	GICR |= (1<<INT2);
	MCUCSR &= ~(1<<ISC2);
	SREG |= (1<<7);
}

ISR(TIMER1_COMPA_vect){
	if(!inc_dec){

		seconds++;
		if(seconds ==60){
			seconds=0 ;
			minutes++;
		}
		if (minutes == 60){
			minutes =0;
			hours++;

		}
	}
	else {
		if(seconds != 0 || minutes != 0 || hours != 0  )
		{
			seconds--;
			if(seconds == 255)
			{
				minutes--;
				seconds = 59;
			}
			if(minutes == 255)
			{
				hours--;
				minutes = 59;
			}


		}
		else{
			// to pause the clock when the timer hits zero
			TCCR1B &= ~(1<<CS12);
			TCCR1B &= ~(1<<CS10);
			//to set the pin controlling the buzzer

		}
	}
}
//PORTA = 0x01;  // Toggle PORTA pin 0 to see if ISR is firing


ISR(INT0_vect){ // reset button
	seconds= 0;
	minutes= 0;
	hours = 0;

}

ISR(INT1_vect){ // pause clock
	TCCR1B &= ~(1<<CS12);
	TCCR1B &= ~(1<<CS10);
}

ISR(INT2_vect){//resume clock
	TCCR1B |= (1<<CS10) | (1<<CS12)| (1<<WGM12);

}

int main(void){
	timer1_compare();
	int0_init();// reset button
	int1_init(); // pause button
	int2_init(); //resume button


	/*  the pins going to the decoder of the 7 segment  */
	DDRC |= 0x0f;/*set as output pins*/
	//PORTC |= 0x07;/* just to try to show a number will delete later*/

	/* the pins who are responsible of enabling the wanted 7 segment*/
	DDRA |= 0x3F;
	PORTA |= 0x3F;/*to enable all just to try will delete later */

	DDRD &= ~(1<<PD2); // input pin at interrupt 0
	PORTD |= (1<<PD2); // to activate internal pull up



	DDRB &= ~(1<<PB2);// set int 2 as input pin
	PORTB &= ~(1<<PB2);
	PORTB |= (1<<PB2);// set internal pull up resistor
	//PORTB |= (0xFB);
	//TCCR1B = (1<<CS12) | (1<<CS10) | (1<<WGM12);
	//PORTB |= (1<<PB2);

	DDRB &= ~(1<<PB7);// set the toogle button as input pin
	PORTB |= (1<<PB7);//set the internal pull


	DDRD |= 0x30; // led pins set as output

	// hours increment
	DDRB &= ~(1<<PB1); // to make all the increment and decrement button input pins
	PORTB |= (1<<PB1); // to use internal pull up resistor in them

	//hours decrement
	DDRB = 0x00;
	PORTB = 0xFF;

	//for the led were using instead of buzzer
	DDRD |= (1<<PD0);


	TCCR1B = (1<<CS12) | (1<<CS10) | (1<<WGM12);//to prevent starting from pause condition

	while(1){

		PORTA = (PORTA & 0xC0) | (1<<PA5);
		PORTC = (PORTC & 0xF0) | (seconds % 10);
		_delay_ms(3);

		PORTA = (PORTA & 0xC0) | (1<<PA4);
		PORTC = (PORTC & 0xF0) | (seconds / (10));
		_delay_ms(3);

		PORTA = (PORTA & 0xC0) | (1<<PA3);
		PORTC = (PORTC & 0xF0) | (minutes % 10);
		_delay_ms(3);

		PORTA = (PORTA & 0xC0) | (1<<PA2);
		PORTC = (PORTC & 0xF0) | (minutes / 10);
		_delay_ms(3);

		PORTA = (PORTA & 0xC0) | (1<<PA1);
		PORTC = (PORTC & 0xF0) | (hours % 10);
		_delay_ms(3);

		PORTA = (PORTA & 0xC0) | (1<<PA0);
		PORTC = (PORTC & 0xF0) | (hours / 10);
		_delay_ms(3);

		// for the toggle between increment and decrement mode
		if(!(PINB & (1<<PB7))){
			if(pressing_flag == 0){
				inc_dec ^= 1;
				pressing_flag = 1;
			}
		}
		else{
			pressing_flag=0;
		}

		// for the 2 leds
		if (inc_dec == 0){
			PORTD &= ~(1<<PD5);
			PORTD |= (1<<PD4);
		}
		else{
			PORTD |= (1<<PD5);
			PORTD &= ~(1<<PD4);
		}

		//hours decrement
		if(!(PINB & (1<<PB0))){
			if(hoursdecrement ==0){
				if(hours){
					hours--;
					hoursdecrement =1;
				}
			}
		}
		else{
			hoursdecrement=0;
		}

		//increment hours
		if(!(PINB & (1<<PB1))){
			if(hoursincrement == 0 ){
				hours++;
				hoursincrement = 1;
			}
		}
		else{
			hoursincrement = 0;
		}
		//minutes decrement
		if(!(PINB & (1<<PB3))){
			if(minutesdecrement == 0){
				if(minutes){
					minutes--;
					minutesdecrement=1;
				}
			}
		}
		else{
			minutesdecrement=0;
		}


		//minutes increment
		if(!(PINB & (1<<PB4))){
			if(minutesincrement == 0){
				if(minutes != 59){
					minutes++;
					minutesincrement = 1;
				}
			}
		}
		else{
			minutesincrement=0;
		}

		//seconds decrement
		if(!(PINB & (1<<PB5))){
			if(secondsdecrement == 0){
				if(seconds){
					seconds--;
					secondsdecrement = 1;
				}
			}
		}
		else{
			secondsdecrement = 0;
		}

		//seconds increment
		if(!(PINB & (1<<PB6))){
			if(secondsincrement == 0){
				if(seconds != 59){
					seconds++;
					secondsincrement =1;
				}
			}
		}
		else{
			secondsincrement =0;
		}

		if(seconds == 0 && minutes ==0 && hours ==0 && inc_dec){
			PORTD |= (1<<PA0);
		}
		else{
			PORTD &= ~(1<<PA0);
		}


		/*
		PORTA =0;s
		PORTA |= (1<<PA0);
		PORTC = (seconds % 10);
		_delay_ms(3);
		 */
	}




}

