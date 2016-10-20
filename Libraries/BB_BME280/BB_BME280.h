/**
 * BB_BME280.h - Library providing the basic functionality to control and read
 * the BME280 sensor (humidity, temperature, pressure).
 *
 *  Created on: Oct 14, 2016
 *      Author: E. Mittermeier, BlueberryE
 *  Released into the public domain.
 */

extern "C" {
	#include <avr/io.h>
    #include <stdint.h>
}

#ifndef BB_BME280_H_
#define BB_BME280_H_

#include <BB_I2C.h>

// The I2C address of the sensor:
#define BB_BME280_ADDRESS (0x76)

// Humidity oversampling osrs_h settings:
#define BME280_osrs_h_SKIPPED	0
#define BME280_osrs_h_x1		1
#define BME280_osrs_h_x2		2
#define BME280_osrs_h_x4		3
#define BME280_osrs_h_x8		4
#define BME280_osrs_h_x16		5

// ctrl_meas osrs_t settings:
#define BME280_osrs_t_SKIPPED	0
#define BME280_osrs_t_x1		1
#define BME280_osrs_t_x2		2
#define BME280_osrs_t_x4		3
#define BME280_osrs_t_x8		4
#define BME280_osrs_t_x16		5

// ctrl_meas osrs_p settings:
#define BME280_osrs_p_SKIPPED	0
#define BME280_osrs_p_x1		1
#define BME280_osrs_p_x2		2
#define BME280_osrs_p_x4		3
#define BME280_osrs_p_x8		4
#define BME280_osrs_p_x16		(0x05)

// ctrl_meas mode settings:
#define BME280_MODE_SLEEP	0
#define BME280_MODE_FORCED	1
#define BME280_MODE_NORMAL	(0x03)

//SPI3w settings:
#define BME280_SPI3w_disable	0
#define BME280_SPI3w_enable		1

// stand by time settings:
#define BME280_StandbyTime_500us		0
#define BME280_StandbyTime_62500us		1
#define BME280_StandbyTime_125ms		2
#define BME280_StandbyTime_250ms		3
#define BME280_StandbyTime_500ms		(0x04)
#define BME280_StandbyTime_1000ms		5
#define BME280_StandbyTime_10ms			6
#define BME280_StandbyTime_20ms			7

//filter settings:
#define BME280_Filter_off	0
#define BME280_Filter_2		1
#define BME280_Filter_4		2
#define BME280_Filter_8		3
#define BME280_Filter_16	(0x04)

/**
 * Contains the all settings for the BME280
 */
struct BB_BME280_SETTINGS{
    uint8_t StandbyTime;		// sets standby time
    uint8_t Filter;				// sets filter coefficient
    uint8_t SPI3w_en;			// 1: enables 3-wire SPI Interface
    uint8_t osrs_t;				// temperature oversampling
    uint8_t osrs_p;				// pressure oversampling
    uint8_t MODE;				// sleep/forced/normal mode
    uint8_t osrs_h;				// humidity oversampling
};

// TODO future implementation
/*
struct BB_BME280_STATUS{
    uint8_t measuring;		// automatically set to 1 when conversation is running;
							// back to 0 when results have been transferred.
    uint8_t im_update;		// automatically set to 1 when NVM data are being copied to image registers;
							// back to 0 when copying is done.
};
*/

//CALIBRATION REGISTER ADDRESS DEFINITIONS
enum BB_BME280_REGISTER{
    CALIB_DIG_T1_LSB = 0x88,
    CALIB_DIG_T2_LSB = 0x8A,
    CALIB_DIG_T3_LSB = 0x8C,

    CALIB_DIG_P1_LSB = 0x8E,
    CALIB_DIG_P2_LSB = 0x90,
    CALIB_DIG_P3_LSB = 0x92,
    CALIB_DIG_P4_LSB = 0x94,
    CALIB_DIG_P5_LSB = 0x96,
    CALIB_DIG_P6_LSB = 0x98,
    CALIB_DIG_P7_LSB = 0x9A,
    CALIB_DIG_P8_LSB = 0x9C,
    CALIB_DIG_P9_LSB = 0x9E,

    CALIB_DIG_H1 = 0xA1,
    CALIB_DIG_H2_LSB = 0xE1,
    CALIB_DIG_H3 = 0xE3,
    CALIB_DIG_H4_MSB = 0xE4,
    CALIB_DIG_H5_MSB = 0xE6,
    CALIB_DIG_H6 = 0xE7,

    CHIPID = 0xD0,
    VERSION = 0xD1,
    SOFTRESET = 0xE0,

    CAL26 = 0xE1,  // R calibration stored in 0xE1-0xF0

    CONTROLHUMID = 0xF2,
    CONTROL = 0xF4,
    CONFIG = 0xF5,

    PRESSUREDATA = 0xF7,
    TEMPERATUREDATA = 0xFA,
    HUMIDITYDATA = 0xFD
};

/**
 * Contains the calibration values of the BME280
 */
struct BB_BME280_CALIBRATION{
    uint16_t dig_T1;		//temperature
    int16_t dig_T2;			//temperature
    int16_t dig_T3;			//temperature

    uint16_t dig_P1;		//pressure
    int16_t dig_P2;			//pressure
    int16_t dig_P3;			//pressure
    int16_t dig_P4;			//pressure
    int16_t dig_P5;			//pressure
    int16_t dig_P6;			//pressure
    int16_t dig_P7;			//pressure
    int16_t dig_P8;			//pressure
    int16_t dig_P9;			//pressure

    uint8_t dig_H1;			//humidity
    int16_t dig_H2;			//humidity
    uint8_t dig_H3;			//humidity
    int16_t dig_H4;			//humidity
    int16_t dig_H5;			//humidity
    int8_t dig_H6;			//humidity
};

/**
 * Objects of this class represent a BME280
 */
class BB_BME280{
    public:
	    /**
	     * Initializes a BME280 object.
	     * @param i2c a reference to a I2C object.
	     */
	    BB_BME280(BB_I2C *i2c);

	    /**
	     * Provides the chip identification number, which is 0x60.
	     * This method can be used to check if the sensor is up and running.
	     * @return the chip identification number 0x60.
	     */
        uint8_t readChipId(void);

        /**
         * Provides the temperature value in degC * 100.
         * @return the temperature value in degC * 100
         */
        int32_t readTemperature(void);

        /**
         * Provides the pressure value in hPa * 100.
         * @return the pressure value in hPa * 100
         */
        uint32_t readPressure(void);

        /**
         * Provides the humidity value in % * 1024.
         * @return the humidity value in % * 1024
         */
        uint32_t readHumidity(void);

        // TODO implement methods for changing the settings
        // TODO implement methods for reading settings
        // TODO implement methods for reading status

        // TODO implement improved method for reading calibration
        //      the new version should have a struc as argument -> only one methode(!)
        // no docu, because this preliminary code
        uint16_t getCalibT1(void);
        int16_t getCalibT2(void);
        int16_t getCalibT3(void);
        uint16_t getCalibP1(void);
        int16_t getCalibP2(void);
        int16_t getCalibP3(void);
        int16_t getCalibP4(void);
        int16_t getCalibP5(void);
        int16_t getCalibP6(void);
        int16_t getCalibP7(void);
        int16_t getCalibP8(void);
        int16_t getCalibP9(void);
        uint16_t getCalibH1(void);
        int16_t getCalibH2(void);
        uint16_t getCalibH3(void);
        int16_t getCalibH4(void);
        int16_t getCalibH5(void);
        int16_t getCalibH6(void);

    private:
        /**
         * the I2C object used for communicaion
         */
	    BB_I2C *_i2c;

	    /**
	     * A convenience variable containing the I2C address of the BME280
	     */
	    uint8_t _i2cAddr;

	    /**
	     * A convenience method used to perform a read operation on the I2C bus.
	     * This method provides a value stored in one register of the BME280.
	     * @param reg a register address on the BME280
	     * @return data read from the register.
	     */
	    uint8_t _i2cRead(BB_BME280_REGISTER reg);

	    /**
	     * A convenience method used to perform a write operation on the I2C bus.
	     * This method pushes a value into one register of the BME280.
	     * @param reg a register address on the BME280
	     * @param value new data for the BME280 register
	     */
	    void _i2cWrite(BB_BME280_REGISTER reg, uint8_t value);

	    /**
	     * Contains the current settings of the BME280.
	     */
	    struct BB_BME280_SETTINGS _settings;

	    // TODO future implemenation
	    //struct BB_BME280_STATUS _status;

	    /**
	     * Contains the calibration values of the BME280.
	     */
	    struct BB_BME280_CALIBRATION _calibration;

	    /**
	     * Reads the calibration values from the BME280 calibration register into
	     * this->_calibration.
	     */
	    void _readCalibration(void);

	    /**
	     * A variable needed to calculate calibrated values for humidity, temperature, pressure.
	     */
	    int32_t _t_fine;

};
#endif /* BB_BME280_H_ */
