/*
 * BB_LTR303ALS01.cpp
 *
 *  Created on: Oct 18, 2016
 *      Author: E. Mittermeier, BlueberryE
 */


#include "BB_LTR303ALS01.h"

// public:
BB_LTR303ALS01::BB_LTR303ALS01(BB_I2C *i2c){
	this->_i2c = i2c;
    this->_i2cAddr = BB_LTR303ALS01_ADDRESS;
    this->_settings = {
    		LTR303ALS01_GAIN_8X,
    		LTR303ALS01_MODE_ACTIVE,
    		LTR303ALS01_INT_100ms,
    		LTR303ALS01_MEAS_100ms,
    		LTR303ALS01_INTERRUPT_ACTIVE_HIGH,
    		LTR303ALS01_INTERRUPT_DISABLED,
    		0xFF,
    		0xFF,
    		0x00,
    		0x00,
    		0
    };

    _delay_ms(100);  // see application note
    this->_writeSettings2Sensor();
}

uint8_t BB_LTR303ALS01::readManufacturerId(void){
	return this->_i2cRead((BB_LTR303ALS01_REGISTER) MANUFAC_ID);
}

uint16_t BB_LTR303ALS01::readChannel1(void){
	uint16_t ch0 = 0x00; // visible   (peak @ 450nm)
	uint16_t ch1 = 0x00; // infra-red (peak @ 770nm)
	uint8_t msb0, lsb0, msb1, lsb1;

	//TODO implement a check if the data is valid
	//_readStatus();

	// read always both data registers as a block (see application note)
	// TODO check if this is really necessary
	lsb1 = this->_i2cRead((BB_LTR303ALS01_REGISTER) ALS_DATA_CH1_0);
	msb1 = this->_i2cRead((BB_LTR303ALS01_REGISTER) ALS_DATA_CH1_1);

	lsb0 = this->_i2cRead((BB_LTR303ALS01_REGISTER) ALS_DATA_CH0_0);
	msb0 = this->_i2cRead((BB_LTR303ALS01_REGISTER) ALS_DATA_CH0_1);

	ch1 = (uint16_t) (((uint16_t) msb1 << 8) | lsb1);
	ch0 = (uint16_t) (((uint16_t) msb0 << 8) | lsb0);


	return ch1;
}

uint16_t BB_LTR303ALS01::readChannel0(void){
	uint16_t ch0 = 0x00; // visible   (peak @ 450nm)
	uint16_t ch1 = 0x00; // infra-red (peak @ 770nm)
	uint8_t msb0, lsb0, msb1, lsb1;

	//TODO implement a check if the data is valid
	//_readStatus();

	// read always both data registers as a block (see application note)
	// TODO check if this is really necessary
	lsb1 = this->_i2cRead((BB_LTR303ALS01_REGISTER) ALS_DATA_CH1_0);
	msb1 = this->_i2cRead((BB_LTR303ALS01_REGISTER) ALS_DATA_CH1_1);

	lsb0 = this->_i2cRead((BB_LTR303ALS01_REGISTER) ALS_DATA_CH0_0);
	msb0 = this->_i2cRead((BB_LTR303ALS01_REGISTER) ALS_DATA_CH0_1);

	ch1 = (uint16_t) (((uint16_t) msb1 << 8) | lsb1);
	ch0 = (uint16_t) (((uint16_t) msb0 << 8) | lsb0);


	return ch0;

}

// private:
void BB_LTR303ALS01::_writeSettings2Sensor(void){

	this->_writeRegister((BB_LTR303ALS01_REGISTER) ALS_CONTR, 0x00, 0x07, 5 );
	this->_writeRegister((BB_LTR303ALS01_REGISTER) ALS_CONTR, this->_settings.gain, 0x07, 2 );
	this->_writeRegister((BB_LTR303ALS01_REGISTER) ALS_CONTR, this->_settings.mode, 0x01, 0 );
	this->_writeRegister((BB_LTR303ALS01_REGISTER) ALS_MEAS_RATE, this->_settings.integrationTime, 0x07, 3 );
	this->_writeRegister((BB_LTR303ALS01_REGISTER) ALS_MEAS_RATE, this->_settings.measurementRate, 0x07, 0 );
	this->_writeRegister((BB_LTR303ALS01_REGISTER) INTERRUPT, this->_settings.interruptPolarity, 0x01, 2 );
	this->_writeRegister((BB_LTR303ALS01_REGISTER) INTERRUPT, this->_settings.interruptMode, 0x01, 1 );

	this->_i2cWrite((BB_LTR303ALS01_REGISTER) ALS_THRES_UP_0, this->_settings.thresholdUp0);
	this->_i2cWrite((BB_LTR303ALS01_REGISTER) ALS_THRES_UP_1, this->_settings.thresholdUp1);
	this->_i2cWrite((BB_LTR303ALS01_REGISTER) ALS_THRES_LOW_0, this->_settings.thresholdLow0);
	this->_i2cWrite((BB_LTR303ALS01_REGISTER) ALS_THRES_LOW_1, this->_settings.thresholdLow1);

	this->_writeRegister((BB_LTR303ALS01_REGISTER) INTERRUPT_PERSIST, this->_settings.interruptMode, 0x0F, 0 );
}

void BB_LTR303ALS01::_writeRegister (BB_LTR303ALS01_REGISTER regAddr, uint8_t val, uint8_t mask, uint8_t shift){
	uint8_t data;

	data = this->_i2cRead(regAddr);

	data = (data & ~(mask << shift)) | ((val << shift) & (val << shift));

	this->_i2cWrite(regAddr, data);
}

uint8_t BB_LTR303ALS01::_i2cRead(BB_LTR303ALS01_REGISTER registerAddr){
  uint8_t value;
  this->_i2c->readbyte(registerAddr, this->_i2cAddr, &value);
  return value;
}

void BB_LTR303ALS01::_i2cWrite(BB_LTR303ALS01_REGISTER registerAddr, uint8_t value){
	this->_i2c->writebyte(registerAddr, this->_i2cAddr, value);
}

