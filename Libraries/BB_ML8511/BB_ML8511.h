/**
 * BB_ML8511.h - Library providing the basic functionality to control and read
 * the ML8511 uv sensor.
 *
 *  Created on: Oct 18, 2016
 *      Author: E. Mittermeier, BlueberryE
 *  Released into the public domain.
 */

extern "C" {
	#include <avr/io.h>
    #include <stdint.h>
	#include <util/delay.h> // TODO remove??
}

#ifndef BB_ML8511_H_
#define BB_ML8511_H_

#define BB_ML8511_setPort2Out DDRD |= (1 << PD6)
#define BB_ML8511_enable PORTD |= (1 << PD6)
#define BB_ML8511_disable PORTD &= ~(1 << PD6)
#define BB_ML8511_muxChannel 2

/**
 * Objects of this class represent a ML8511
 */
class BB_ML8511{
	public:
	    /**
	     * Initializes a ML8511 object.
	     */
	    BB_ML8511();

	    // TODO remove
		//bool begin(void);

	    /**
	     * Reads the UV signal.
	     * @return the UV signal.
	     */
		uint16_t readUvLevel(void); // output of the adc converter -> convert to voltage using (3.3V / 1024 * level)

		/**
		 * Reads the UV signal. The value is an average of several measurements.
		 * @param measurementCount the number of measurements to be done. Has to be > 0.
		 * @return the average UV signal.
		 */
		uint16_t readUvLevel(uint8_t measurementCount); // output of the adc converter -> convert to voltage using (3.3V / 1024 * level)

	private:
		/**
		 * Initiates the ADC of the Atmega328P
		 */
	    void _adcInit();

		uint16_t _adcRead(uint8_t channel);

		/**
		 * Initiates the Atmega328P for ADC and calls _adcInit()
		 * @return 1
		 */
		int8_t _init();
};

/*
//Convert the voltage to a UV intensity level(!!!)
// outputVoltage = readUvLevel
mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0);
//From: http://forum.arduino.cc/index.php?topic=3922.0
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
*/



#endif /* BB_ML8511_H_ */
