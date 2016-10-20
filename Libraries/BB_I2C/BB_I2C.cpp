/*
 * BB_I2C.cpp
 *
 *  Created on: Oct 14, 2016
 *      Author: emit
 */

#include "BB_I2C.h"


BB_I2C::BB_I2C(){
	this->_init();
}

int8_t BB_I2C::_init(){
	TWSR = 0x00;    //set Prescaler to faktor 1
	TWBR = ((F_CPU/SCL_CLOCK)-16)/2; /* must be > 10 for stable operation */
	return 0;
}

uint8_t BB_I2C::_transmit(uint8_t type){
	switch(type) {
		case I2C_START:    // Send Start Condition
		TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
		break;
		case I2C_DATA:     // Send Data
		TWCR = (1 << TWINT) | (1 << TWEN);
		break;
		case I2C_STOP:     // Send Stop Condition
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
		return 0;
	}
	// Wait for TWINT flag set in TWCR Register
	while (!(TWCR & (1 << TWINT)));
	// Return TWI Status Register, mask the prescaler bits (TWPS1,TWPS0)
	return (TWSR & 0xF8);
}

int8_t BB_I2C::writebyte(uint8_t reg_address, uint8_t dev_addr, uint8_t data){
	//unsigned char n = 0;
	unsigned char twi_status;
	char r_val = -1;

	I2C_retry:

	// Transmit Start Condition
	twi_status = this->_transmit(I2C_START);

	// Check the TWI Status
	if (twi_status == TW_MT_ARB_LOST) goto I2C_retry;
	if ((twi_status != TW_START) && (twi_status != TW_REP_START)) goto I2C_quit;
	// Send slave address (SLA_W)
	TWDR = (dev_addr << 1) | TW_WRITE;
	// Transmit I2C Data
	twi_status = this->_transmit(I2C_DATA);
	// Check the TWSR status
	if ((twi_status == TW_MT_SLA_NACK) || (twi_status == TW_MT_ARB_LOST)) goto I2C_retry;
	if (twi_status != TW_MT_SLA_ACK) goto I2C_quit;
	// Send the I2C Address
	TWDR = reg_address;
	// Transmit I2C Data
	twi_status = this->_transmit(I2C_DATA);
	// Check the TWSR status
	if (twi_status != TW_MT_DATA_ACK) goto I2C_quit;
	// Put data into data register and start transmission
	TWDR = data;
	// Transmit I2C Data
	twi_status = this->_transmit(I2C_DATA);
	// Check the TWSR status
	if (twi_status != TW_MT_DATA_ACK) goto I2C_quit;
	// TWI Transmit Ok
	r_val=1;

	I2C_quit:

	// Transmit I2C Data
	twi_status = this->_transmit(I2C_STOP);

	_delay_us(10);
	return r_val;
}

int8_t BB_I2C::readbyte(uint8_t reg_address, uint8_t dev_addr, uint8_t* data){
	//unsigned char n = 0;
	unsigned char twi_status;
	char r_val = -1;

	I2C_retry:

	// Transmit Start Condition
	twi_status = this->_transmit(I2C_START);

	// Check the TWSR status
	if (twi_status == TW_MT_ARB_LOST) goto I2C_retry;
	if ((twi_status != TW_START) && (twi_status != TW_REP_START)) goto I2C_quit;

	// Send slave address (SLA_W) 0xa0
	TWDR = (dev_addr << 1) | TW_WRITE;

	// Transmit I2C Data
	twi_status = this->_transmit(I2C_DATA);

	// Check the TWSR status
	if ((twi_status == TW_MT_SLA_NACK) || (twi_status == TW_MT_ARB_LOST)) goto I2C_retry;
	if (twi_status != TW_MT_SLA_ACK) goto I2C_quit;

	// Send I2C Address
	TWDR = reg_address;

	// Transmit I2C Data
	twi_status = this->_transmit(I2C_DATA);

	// Check the TWSR status
	if (twi_status != TW_MT_DATA_ACK) goto I2C_quit;

	// Send start Condition
	twi_status = this->_transmit(I2C_START);

	// Check the TWSR status
	if (twi_status == TW_MT_ARB_LOST) goto I2C_retry;
	if ((twi_status != TW_START) && (twi_status != TW_REP_START)) goto I2C_quit;

	// Send slave address (SLA_R)
	TWDR = (dev_addr << 1) | TW_READ;

	// Transmit I2C Data
	twi_status = this->_transmit(I2C_DATA);
	// Check the TWSR status
	if ((twi_status == TW_MR_SLA_NACK) || (twi_status == TW_MR_ARB_LOST)) goto I2C_retry;
	if (twi_status != TW_MR_SLA_ACK) goto I2C_quit;

	// Read I2C Data
	twi_status = this->_transmit(I2C_DATA);
	if (twi_status != TW_MR_DATA_NACK) goto I2C_quit;

	// Get the Data
	*data=TWDR;
	r_val=1;

	I2C_quit:

	// Send Stop Condition
	twi_status = this->_transmit(I2C_STOP);

	_delay_us(10);

	return r_val;
}
