/*
 * BB_USART.c
 *
 *  Created on: Oct 18, 2016
 *      Author: emit
 */

#include "BB_USART.h"

#define F_SYS 16000000UL // 16 MHz
#define USART_BAUDRATE 9600
#define UBRR_VALUE (((F_SYS / (USART_BAUDRATE * 16UL))) - 1)

void BB_USART_init(void){
	// Set baud rate
	UBRR0H = (uint8_t)(UBRR_VALUE>>8);
	UBRR0L = (uint8_t)UBRR_VALUE;
	// Set frame format to 8 data bits, no parity, 1 stop bit
	UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);
	//enable transmission
	UCSR0B |= (1<<TXEN0);
}

void BB_USART_send_byte(uint8_t u8Data){
	//wait while previous byte is completed
	while(!(UCSR0A&(1<<UDRE0))){};
	// Transmit data
	UDR0 = u8Data;
}
