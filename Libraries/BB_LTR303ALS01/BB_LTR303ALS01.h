/**
 * BB_LTR303ALS01.h - Library providing the basic functionality to control and read
 * the LTR303ALS01 light sensor.
 *
 *  Created on: Oct 18, 2016
 *      Author: E. Mittermeier, BlueberryE
 *  Released into the public domain.
 */

extern "C" {
    #include <avr/io.h>
    #include <stdint.h>
    #include <util/delay.h>
}

#include <BB_I2C.h>

#ifndef BB_LTR303ALS01_H_
#define BB_LTR303ALS01_H_

// The I2C address of the sensor
#define BB_LTR303ALS01_ADDRESS (0x29)

// light integration time settings
#define LTR303ALS01_INT_100ms 0
#define LTR303ALS01_INT_50ms  1
#define LTR303ALS01_INT_200ms 2
#define LTR303ALS01_INT_400ms 3
#define LTR303ALS01_INT_150ms 4
#define LTR303ALS01_INT_250ms 5
#define LTR303ALS01_INT_300ms 6
#define LTR303ALS01_INT_350ms 7

// measurement period settings
#define LTR303ALS01_MEAS_50ms   0
#define LTR303ALS01_MEAS_100ms  1
#define LTR303ALS01_MEAS_200ms  2
#define LTR303ALS01_MEAS_500ms  3
#define LTR303ALS01_MEAS_1000ms 4
#define LTR303ALS01_MEAS_2000ms 5

// light amplification gain settings
#define LTR303ALS01_GAIN_1X  0
#define LTR303ALS01_GAIN_2X  1
#define LTR303ALS01_GAIN_4X  2
#define LTR303ALS01_GAIN_8X  3
#define LTR303ALS01_GAIN_48X 6
#define LTR303ALS01_GAIN_96X 7

// Reset w/ or w/o running the start-up procedure
#define LTR303ALS01_RESET_WITHOUT_STARTUP 0
#define LTR303ALS01_RESET_WITH_STARTUP    1

// ALS mode
#define LTR303ALS01_MODE_STANDBY 0
#define LTR303ALS01_MODE_ACTIVE  1

// interrupt polarity
#define LTR303ALS01_INTERRUPT_ACTIVE_LOW  0
#define LTR303ALS01_INTERRUPT_ACTIVE_HIGH 1

// interrupt mode
#define LTR303ALS01_INTERRUPT_DISABLED 0
#define LTR303ALS01_INTERRUPT_ENABLED  1

/**
 * Contains the settings for the LTR303ALS01
 */
struct BB_LTR303ALS01_SETTINGS{
	uint8_t gain;               // light amplification gain
	uint8_t mode;               // active or stand-by
	uint8_t integrationTime;    // sensor current integration time
	uint8_t measurementRate;    // measurement rate
	uint8_t interruptPolarity;  // interrupt polarity
	uint8_t interruptMode;      // interrupt enabled or disabled
	uint8_t thresholdUp0;       // upper threshold for channel 0
	uint8_t thresholdUp1;       // upper threshold for channel 1
	uint8_t thresholdLow0;      // lower threshold for channel 0
	uint8_t thresholdLow1;      // lower threshold for channel 1
	uint8_t interruptPersist;	// number of occurrences until interrupt is asserted
};

//TODO future implementations
/*
struct BB_LTR303ALS01_STATUS{
	uint8_t dataValid;        // 0 : data is valid, 1 : data is invalid
	uint8_t dataGainRange;    // LTR303ALS01_GAIN_X
	uint8_t interruptStatus;  // 0 : interrupt signal inactive, 1 : interrupt signal active
	uint8_t dataStatus;       // 0 : old data, 1 : new data available
};
*/

//REGISTER ADDRESSES
enum BB_LTR303ALS01_REGISTER{
	ALS_CONTR         = 0x80,
	ALS_MEAS_RATE     = 0x85,
	PART_ID           = 0x86,
	MANUFAC_ID        = 0x87,
    ALS_DATA_CH1_0    = 0x88,
	ALS_DATA_CH1_1    = 0x89,
	ALS_DATA_CH0_0    = 0x8A,
	ALS_DATA_CH0_1    = 0x8B,
	ALS_STATUS        = 0x8C,
	INTERRUPT         = 0x8F,
	ALS_THRES_UP_0    = 0x97,
	ALS_THRES_UP_1    = 0x98,
	ALS_THRES_LOW_0   = 0x99,
	ALS_THRES_LOW_1   = 0x9A,
	INTERRUPT_PERSIST = 0x9E,
};

/**
 * Objects of this class represent a LTR303ALS01
 */
class BB_LTR303ALS01{
    public:
	    /**
	     * Initializes a LTR303ALS01 object.
	     * @param i2c a reference to a I2C object.
	     */
	    BB_LTR303ALS01(BB_I2C *i2c);

	    /**
         * Provides the manufacturer identification number, which is 0x05.
	     * This method can be used to check if the sensor is up and running.
	     * @return the manufacturer identification number 0x05.
	     */
	    uint8_t readManufacturerId(void);

	    // TODO check if this makes sense and then implement
	    //uint16_t readVisibleLight(void);
		//uint16_t readIrLight(void);

	    /**
	     * Provide the sensor value form channel 0.
	     * This channel has its maximum sensitivity at a wavelength = 450nm
	     * @return the value read from channel 0
	     */
		uint16_t readChannel0(void);

	    /**
	     * Provide the sensor value form channel 1.
	     * This channel has its maximum sensitivity at a wavelength = 770nm
	     * @return the value read from channel 1
	     */
		uint16_t readChannel1(void);

		//TODO implement methods for changing the settings

    private:
		/**
		 * the I2C object used for communication
		 */
	    BB_I2C *_i2c;

	    /**
	     * A convenience variable containing the I2C address of the BME280
	     */
	    uint8_t _i2cAddr;

	    /**
	     * Contains the current settings of the LTR303ALS01.
	     */
	    struct BB_LTR303ALS01_SETTINGS _settings;
	    //struct BB_LTR303ALS01_STATUS _status;

	    /**
	     * A convenience method used to perform a read operation on the I2C bus.
	     * This method provides a value stored in one register of the LTR303ALS01.
	     * @param reg a register address on the LTR303ALS01
	     * @return data read from the register.
	     */
	    uint8_t _i2cRead(BB_LTR303ALS01_REGISTER regAddr);

	    /**
	     * A convenience method used to perform a write operation on the I2C bus.
	     * This method pushes a value into one register of the BME280.
	     * @param reg a register address on the BME280
	     * @param value new data for the BME280 register
	     */
	    void _i2cWrite(BB_LTR303ALS01_REGISTER regAddr, uint8_t value);

	    /**
	     * Writes the settings to the sensor.
	     */
	    void _writeSettings2Sensor(void);

	    /**
	     * Docu to be done.
	     * @param regAddr
	     * @param val
	     * @param mask
	     * @param shift
	     */
        void _writeRegister(BB_LTR303ALS01_REGISTER regAddr, uint8_t val, uint8_t mask, uint8_t shift);

		//void _readStatus(void);

};



#endif /* BB_LTR303ALS01_H_ */
