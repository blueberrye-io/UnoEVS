/**
 * BB_I2C.h - A library providing I2C functionality for I2C masters.
 *
 *  Created on: Oct 14, 2016
 *      Author: E. Mittermeier, BlueberryE
 *  Released into the public domain.
 */

extern "C" {
    #include <avr/io.h>
    #include <stdint.h>
    #include <util/twi.h>
    #include <util/delay.h>
}

#ifndef BB_I2C_H_
#define BB_I2C_H_

#ifndef F_CPU
    #define F_CPU 8000000UL
#endif

#ifndef SCL_CLOCK
    #define SCL_CLOCK 100000L
#endif

#define I2C_START 0
#define I2C_DATA  1
#define I2C_STOP  2

/**
 * Objects of this class are used for communication using the I2C protocol.
 * This is for a I2C master. This class provides the methods to read /
 * write one value into / from a register of a I2C slave.
 */
class BB_I2C{

	public:
	    /**
	     * Initiates a new I2C object.
	     */
	    BB_I2C();
	    //int8_t init(); //TODO remove

	    /**
	     * Read one byte of data from a register of one I2C slave.
	     * @param reg_address the address of the register on the I2C slave
	     * @param dev_addr the I2C address of the slave
	     * @param data contains the data after I2C communication
	     * @return 1 if I2C communication was successful
	     */
	    int8_t readbyte(uint8_t reg_address, uint8_t dev_addr, uint8_t* data);

	    /**
	     * Writes one byte of data into a register of one I2C slave.
	     * @param reg_address reg_address the address of the register on the I2C slave
	     * @param dev_addr dev_addr the I2C address of the slave
	     * @param data the data which will be written to the slave
	     * @return 1 if I2C communication was successful
	     */
	    int8_t writebyte(uint8_t reg_address, uint8_t dev_addr, uint8_t data);

	private:
	    /**
	     * Initiates the I2C settings.
	     * @return
	     */
	    int8_t _init();

	    /**
	     * Controls the data transmission
	     * @param type one I2C condition (Start, send, stop)
	     * @return return TWI Status Register without the prescaler bits (TWPS1,TWPS0)
	     */
	    uint8_t _transmit(uint8_t type);
};

#endif /* BB_I2C_H_ */
