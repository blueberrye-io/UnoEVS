/*
 * BB_USART.h
 *
 *  Created on: Oct 18, 2016
 *      Author: emit
 */

#include <avr/io.h>
#include <stdint.h>

#ifndef BB_USART_H_
#define BB_USART_H_

void BB_USART_init(void);
void BB_USART_send_byte(uint8_t u8Data);

#endif /* BB_USART_H_ */
