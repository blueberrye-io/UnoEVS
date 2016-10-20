/*
 * BB_BME280.cpp
 *
 *  Created on: Oct 14, 2016
 *      Author: emit
 */

#include "BB_BME280.h"

// public:

BB_BME280::BB_BME280(BB_I2C *i2c){
	this->_i2c = i2c;
	this->_i2cAddr = BB_BME280_ADDRESS;

    this->_settings = {
	    BME280_StandbyTime_500ms,
	    BME280_Filter_16,
	    BME280_SPI3w_disable,
	    BME280_osrs_t_x16,
	    BME280_osrs_p_x16,
	    BME280_MODE_NORMAL,          // normal mode
	    BME280_osrs_h_x16
    };

    this->_t_fine = 0;

    this->_readCalibration();

    this->_i2cWrite((BB_BME280_REGISTER) CONTROLHUMID, (uint8_t) 0x03); // Set before CONTROL (DS 5.4.3)
    this->_i2cWrite((BB_BME280_REGISTER) CONTROL, (uint8_t) 0x3F);
}

uint8_t BB_BME280::readChipId(void){
	return this->_i2cRead((BB_BME280_REGISTER) CHIPID);
}

int32_t BB_BME280::readTemperature(void){
    int32_t adc_T = (int32_t) ((((uint32_t) this->_i2cRead((BB_BME280_REGISTER) TEMPERATUREDATA)) << 12) |
                               (((uint32_t) this->_i2cRead((BB_BME280_REGISTER) (TEMPERATUREDATA + 1))) << 4) |
                               (((uint32_t) this->_i2cRead((BB_BME280_REGISTER) (TEMPERATUREDATA + 2))) >> 4)
                              );
	int32_t x1_t = ((((adc_T >> 3) - ((int32_t) this->_calibration.dig_T1 << 1)))
					* ((int32_t) this->_calibration.dig_T2)) >> 11;

	int32_t x2_t = (((((adc_T >> 4) - (this->_calibration.dig_T1)) * ((adc_T >> 4) - this->_calibration.dig_T1)) >> 12) * this->_calibration.dig_T3) >> 14;

	this->_t_fine = x1_t + x2_t;

	int32_t temperature = (this->_t_fine * 5 + 128) >> 8;
	return temperature;
}

uint32_t BB_BME280::readPressure(void){
	int32_t adc_P = (int32_t)((((uint32_t) this->_i2cRead((BB_BME280_REGISTER) PRESSUREDATA)) << 12) |       // PRESS_MSB = 0xF7
			                  (((uint32_t) this->_i2cRead((BB_BME280_REGISTER) (PRESSUREDATA + 1))) << 4) |  // PRESS_LSB = 0xF8
			                  (((uint32_t) this->_i2cRead((BB_BME280_REGISTER) (PRESSUREDATA + 2))) >> 4)    // PRESS_XLSB = 0xF9
							 );

	int32_t x1_p = (((int32_t) this->_t_fine) >> 1) - (int32_t)64000;

	int32_t x2_p = (((x1_p >> 2) * (x1_p >> 2)) >> 11) * ((int32_t) this->_calibration.dig_P6);
	x2_p = x2_p + ((x1_p * ((int32_t) this->_calibration.dig_P5)) << 1);
	x2_p = (x2_p >> 2) + (((int32_t) this->_calibration.dig_P4) << 16);

	x1_p = (((this->_calibration.dig_P3 * (((x1_p >> 2) * (x1_p >> 2)) >> 13)) >> 3) +
	        ((((int32_t) this->_calibration.dig_P2) * x1_p) >> 1)) >> 18;

	x1_p = ((((32768 + x1_p)) * ((int32_t) this->_calibration.dig_P1)) >> 15);

	uint32_t pressure = (((uint32_t)(((int32_t)1048576) - adc_P) - (x2_p >> 12))) * 3125;
	if (pressure < 0x80000000){
		/* Avoid exception caused by division by zero */
		if (x1_p != 0){
			pressure = (pressure << 1) / ((uint32_t) x1_p);
		} else {
			return 0;
		}
	} else {
		if (x1_p != 0){
			pressure = (pressure << 1 / (uint32_t) x1_p) * 2;
		} else {
			return 0;
		}
	}

	x1_p = (((int32_t) this->_calibration.dig_P9) *
		    ((int32_t)(((pressure >> 3) * ( pressure >> 3)) >> 13))) >> 12;
	x2_p = (((int32_t)(pressure >> 2)) * ((int32_t) this->_calibration.dig_P8)) >> 13;
    pressure = (uint32_t)((int32_t)pressure + ((x1_p + x2_p + this->_calibration.dig_P7) >> 4));

	return pressure;
}

uint32_t BB_BME280::readHumidity(void) {

  int32_t adc_H = (int32_t) ((((uint32_t) this->_i2cRead((BB_BME280_REGISTER) HUMIDITYDATA)) << 8) |  // HUM_MSB = 0xFD
                             ((uint32_t) this->_i2cRead((BB_BME280_REGISTER) (HUMIDITYDATA + 1)))           // HUM_LSB = 0xFE
							 );

  int32_t v_x1_u32r;

  v_x1_u32r = (this->_t_fine - ((int32_t)76800));

  v_x1_u32r = (((((adc_H << 14) - (((int32_t) this->_calibration.dig_H4) << 20) -
		  (((int32_t) this->_calibration.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
	       (((((((v_x1_u32r * ((int32_t) this->_calibration.dig_H6)) >> 10) *
		    (((v_x1_u32r * ((int32_t) this->_calibration.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
		  ((int32_t)2097152)) * ((int32_t) this->_calibration.dig_H2) + 8192) >> 14));

  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
			     ((int32_t) this->_calibration.dig_H1)) >> 4));

  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;

  return (uint32_t) (v_x1_u32r>>12);
}


uint16_t BB_BME280::getCalibT1(void){
	return this->_calibration.dig_T1;
}
int16_t BB_BME280::getCalibT2(void){
	return this->_calibration.dig_T2;
}
int16_t BB_BME280::getCalibT3(void){
	return this->_calibration.dig_T3;
}
uint16_t BB_BME280::getCalibP1(void){
	return this->_calibration.dig_P1;
}
int16_t BB_BME280::getCalibP2(void){
	return this->_calibration.dig_P2;
}
int16_t BB_BME280::getCalibP3(void){
	return this->_calibration.dig_P3;
}
int16_t BB_BME280::getCalibP4(void){
	return this->_calibration.dig_P4;
}
int16_t BB_BME280::getCalibP5(void){
	return this->_calibration.dig_P5;
}
int16_t BB_BME280::getCalibP6(void){
	return this->_calibration.dig_P6;
}
int16_t BB_BME280::getCalibP7(void){
	return this->_calibration.dig_P7;
}
int16_t BB_BME280::getCalibP8(void){
	return this->_calibration.dig_P8;
}
int16_t BB_BME280::getCalibP9(void){
	return this->_calibration.dig_P9;
}
uint16_t BB_BME280::getCalibH1(void){
	return (uint16_t) this->_calibration.dig_H1;
}
int16_t BB_BME280::getCalibH2(void){
	return this->_calibration.dig_H2;
}
uint16_t BB_BME280::getCalibH3(void){
	return (uint16_t) this->_calibration.dig_H3;
}
int16_t BB_BME280::getCalibH4(void){
	return this->_calibration.dig_H4;
}
int16_t BB_BME280::getCalibH5(void){
	return this->_calibration.dig_H5;
}
int16_t BB_BME280::getCalibH6(void){
	return (int16_t) this->_calibration.dig_H3;
}

// private:

uint8_t BB_BME280::_i2cRead(BB_BME280_REGISTER registerAddr){
  uint8_t value;
  this->_i2c->readbyte(registerAddr, this->_i2cAddr, &value);
  return value;
}

void BB_BME280::_i2cWrite(BB_BME280_REGISTER registerAddr, uint8_t value){
	this->_i2c->writebyte(registerAddr, this->_i2cAddr, value);
}

/**************************************************************************/
/*!
   @brief  Reads the factory-set coefficients
*/
/**************************************************************************/
void BB_BME280::_readCalibration(void){
    uint8_t msb, lsb;

    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_T1_LSB);
    msb = this->_i2cRead((BB_BME280_REGISTER) (CALIB_DIG_T1_LSB + 1));
    this->_calibration.dig_T1 = (uint16_t) ((((uint16_t) msb) << 8) | lsb);

    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_T2_LSB);
    msb = this->_i2cRead((BB_BME280_REGISTER) (CALIB_DIG_T2_LSB + 1));
    this->_calibration.dig_T2 = (int16_t) ((((int16_t) msb) << 8) | lsb);

    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_T3_LSB);
    msb = this->_i2cRead((BB_BME280_REGISTER) (CALIB_DIG_T3_LSB + 1));
    this->_calibration.dig_T3 = (int16_t) ((((int16_t) msb) << 8) | lsb);


    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_P1_LSB);
    msb = this->_i2cRead((BB_BME280_REGISTER) (CALIB_DIG_P1_LSB + 1));
    this->_calibration.dig_P1 = (uint16_t) ((((uint16_t) msb) << 8) | lsb);

    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_P2_LSB);
    msb = this->_i2cRead((BB_BME280_REGISTER) (CALIB_DIG_P2_LSB + 1));
    this->_calibration.dig_P2 = (int16_t) ((((int16_t) msb) << 8) | lsb);

    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_P3_LSB);
    msb = this->_i2cRead((BB_BME280_REGISTER) (CALIB_DIG_P3_LSB + 1));
    this->_calibration.dig_P3 = (int16_t) ((((int16_t) msb) << 8) | lsb);

    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_P4_LSB);
    msb = this->_i2cRead((BB_BME280_REGISTER) (CALIB_DIG_P4_LSB + 1));
    this->_calibration.dig_P4 = (int16_t) ((((int16_t) msb) << 8) | lsb);

    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_P5_LSB);
    msb = this->_i2cRead((BB_BME280_REGISTER) (CALIB_DIG_P5_LSB + 1));
    this->_calibration.dig_P5 = (int16_t) ((((int16_t) msb) << 8) | lsb);

    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_P6_LSB);
    msb = this->_i2cRead((BB_BME280_REGISTER) (CALIB_DIG_P6_LSB + 1));
    this->_calibration.dig_P6 = (int16_t) ((((int16_t) msb) << 8) | lsb);

    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_P7_LSB);
    msb = this->_i2cRead((BB_BME280_REGISTER) (CALIB_DIG_P7_LSB + 1));
    this->_calibration.dig_P7 = (int16_t) ((((int16_t) msb) << 8) | lsb);

    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_P8_LSB);
    msb = this->_i2cRead((BB_BME280_REGISTER) (CALIB_DIG_P8_LSB + 1));
    this->_calibration.dig_P8 = (int16_t) ((((int16_t) msb) << 8) | lsb);

    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_P9_LSB);
    msb = this->_i2cRead((BB_BME280_REGISTER) (CALIB_DIG_P9_LSB + 1));
    this->_calibration.dig_P9 = (int16_t) ((((int16_t) msb) << 8) | lsb);


    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_H1);
    this->_calibration.dig_H1 = (uint8_t) lsb;

    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_H2_LSB);
    msb = this->_i2cRead((BB_BME280_REGISTER) (CALIB_DIG_H2_LSB + 1));
    this->_calibration.dig_H2 = (int16_t) ((((int16_t) msb) << 8) | lsb);

    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_H3);
    this->_calibration.dig_H3 = (uint8_t) lsb;

    msb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_H4_MSB);
    lsb = this->_i2cRead((BB_BME280_REGISTER) (CALIB_DIG_H4_MSB + 1));
    this->_calibration.dig_H4 = (int16_t) ((((int16_t) msb) << 4) | (0x0F & lsb));

    lsb = this->_i2cRead((BB_BME280_REGISTER) (CALIB_DIG_H4_MSB + 1));
    msb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_H5_MSB);
    this->_calibration.dig_H5 = (int16_t) ((((int16_t) msb) << 4) | (lsb >> 4));

    lsb = this->_i2cRead((BB_BME280_REGISTER) CALIB_DIG_H6);
    this->_calibration.dig_H6 = (int8_t) lsb;
}
