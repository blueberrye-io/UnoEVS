/*
 * BB_ML8511.cpp
 *
 *  Created on: Oct 18, 2016
 *      Author: emit
 */

#include "BB_ML8511.h"

// public:

BB_ML8511::BB_ML8511(){
	this->_init();
}

uint16_t BB_ML8511::readUvLevel(void){
	uint16_t uvLevel;
	BB_ML8511_enable;
	_delay_ms(100); // TODO - skip and replace by dummy measurement
	//dummy measurement
	//_adcRead(BB_ML8511_muxChannel);
	//real measurement
	uvLevel = this->_adcRead(BB_ML8511_muxChannel);
	BB_ML8511_disable;
	return uvLevel;
}

uint16_t BB_ML8511::readUvLevel(uint8_t measurementCount){
	uint16_t uvLevel = 0;
	BB_ML8511_enable;
	_delay_ms(10); // TODO - skip and replace by dummy measurement
	for (uint8_t i = 0; i < measurementCount; i++){
		uvLevel = uvLevel + this->_adcRead(BB_ML8511_muxChannel);
	}
	BB_ML8511_disable;
	return (uvLevel / measurementCount);
}

// private:

int8_t BB_ML8511::_init(void){
	BB_ML8511_setPort2Out;
    BB_ML8511_disable; //TODO check if this improves power saving????????
	this->_adcInit();
	return 1;
}

void BB_ML8511::_adcInit(void){
	ADMUX |= (1<<REFS0); // Select Vref=AVcc
	//ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN); //set prescaller to 128 and enable ADC
	ADCSRA |= (1<<ADPS1)|(1<<ADPS0)|(1<<ADEN); //set prescaler to 128 and enable ADC
}

uint16_t BB_ML8511::_adcRead(uint8_t channel){
	//select ADC channel with safety mask
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
	//single conversion mode
	ADCSRA |= (1<<ADSC);
	// wait until ADC conversion is complete
	while( ADCSRA & (1<<ADSC) );
	return ADC;
}


