/**
 * BB_EVS.cpp - this is an example for the usage of the BlueberryE UnoEVS.
 * It shows how to control the UnoEVS with commands transferred via SPI.
 * Measurements are triggered from a master by "external" SPI commands. This
 * master may e.g. be an Uno335. The measurement results are transferred back
 * the master. All communication between master and UnoEVS is controlled by
 * the master. The UnoEVS triggers the measurements in the sensors of
 * the board and performs all necessary calculations to get physical values.
 *
 * In order to reduce the power consumption, the Atmega328P of the UnoEVS
 * is set to sleep between the measurements. A signal change on the
 * SPI slave select pin triggers the wake up of the controller.
 *
 *  Created on: Oct 18, 2016
 *      Author: E. Mittermeier, BlueberryE
 *  Released into the public domain.
 */

// as we use 3.3V, 8MHZ is a reasonable operation frequency for
// the Atmega328P
#define F_CPU 8000000UL

// some convenience definitions
#define redLedOn  PORTD |= (1 << PD7)
#define redLedOff PORTD &= ~(1 << PD7)
#define greenLedOn PORTB |= (1<<PB0)
#define greenLedOff PORTB &= ~(1<<PB0)

extern "C" {
    #include <avr/io.h>
    #include <stdint.h>
    #include <util/delay.h>
    #include <avr/power.h>  
    #include <avr/wdt.h>
    #include <avr/sleep.h>
    #include <avr/interrupt.h>
}

// include the libraries for the sensors
#include <BB_I2C.h>
#include <BB_BME280.h>
#include <BB_LTR303ALS01.h>
#include <BB_ML8511.h>

/**
 * Initiate the SPI settings. Note - the controller
 * of this board will act as SPI slave. SPI communication
 * does not use interrupts ("polling mode")
 */
void SPI_SlaveInit_woInt(void){

    volatile char dummy;

    // Set PB4(MISO) as output - all others are input
    DDRB |= (1 << PB4);

    //Enable SPI in Slave Mode
    SPCR  = (1 << SPE);

    dummy   = SPSR;                         // Clear SPIF bit in SPSR
    dummy   = SPDR;
}

/**
 * Transfer data via SPI.
 * @param inData for write operations: this is the data which
 *               will be transferred from slave to master
 *               for read operations: no meaning
 * @return for read operations: the data which was transferred from
 *         master to slave
 *         for write operations: no meaning
 */
uint8_t SPI_transferData(uint8_t inData){
    SPDR = inData;
    //asm volatile("nop");
    while (!(SPSR & (1 << SPIF)));
    return SPDR;
}

// define an interrupt service routine which we will need to wake
// up the processor from sleep.
// Trigger will be a signal change at the SPI slave select pin.
ISR(PCINT0_vect){
}

int main(void){
    // the processor works with 3.3V -> we need 8 MHz
    clock_prescale_set(clock_div_2); // set MCU freq to 16/2 MHz

    // power saving
    wdt_disable();  // switch off watchdog

    // define ports
    PORTC = 0xFF;
    DDRD = (1 << PD6) | (1 << PD7);
    PORTD = (1 << PD1) | (1 << PD3) | (1 << PD4) | (1 << PD5);
    DDRB = (1 << PB0) | (1 << PB1);

    greenLedOff;
    redLedOff;

    // initialization

    SPI_SlaveInit_woInt();

    BB_I2C i2c;

    BB_BME280 bme(&i2c);
    // check the initialization
    if (bme.readChipId() != 0x60){
        redLedOn;
        while(1);
    }
    // indicates successful BME280 initialization
    greenLedOn;
    _delay_ms(2000);
    greenLedOff;
    _delay_ms(2000);

    BB_LTR303ALS01 ltr(&i2c);
    // check the initialization
    if (ltr.readManufacturerId() != 0x05){
        greenLedOn;
        return false;
    }
    // indicates successful LTR303ALS01 initialization
    // TODO - write better code(!)
    greenLedOn;
    _delay_ms(500);
    greenLedOff;
    _delay_ms(500);
    greenLedOn;
    _delay_ms(500);
    greenLedOff;
    _delay_ms(2000);

    BB_ML8511 ml8511;
    // indicates successful ML8511 initialization
    // TODO - write better code(!)
    greenLedOn;
    _delay_ms(500);
    greenLedOff;
    _delay_ms(500);
    greenLedOn;
    _delay_ms(500);
    greenLedOff;
    _delay_ms(500);
    greenLedOn;
    _delay_ms(500);
    greenLedOff;
    _delay_ms(2000);

    // define variables and command codes

    uint8_t newCommand = 0xFF;

    const uint8_t cmdSleep = 0xF0;

    const uint8_t cmdBmeStart = 0x10;
    const uint8_t cmdBmeGetTemperature = 0x11;
    const uint8_t cmdBmeGetPressure = 0x12;
    const uint8_t cmdBmeGetHumidity = 0x13;

    int32_t bmeTemperature;
    uint32_t bmePressure;
    uint32_t bmeHumidity;

    const uint8_t cmdLtrStart = 0x20;
    const uint8_t cmdLtrGetCh0 = 0x21;
    const uint8_t cmdLtrGetCh1 = 0x22;
    uint16_t ltrCh0;
    uint16_t ltrCh1;

    const uint8_t cmdMl8511Start = 0x30;
    const uint8_t cmdMl8511GetValue = 0x31;
    uint16_t ml8511UvLevel;

    // initialize interrupt handling:

    // PB2 is input and is used as _SS
    // we will use it also for external interrupt to wake up the processor
    // Special function PCINT2 -> PB2 triggers a PCI0 interrupt request
    cli(); // disable interrupts during set up
    PCICR |= (1 << PCIE0); // Pin Change Interrupt Control Register -> activate PCI0
    PCMSK0 |= (1 << PCINT2); // Pin Change Mask Register -> Pin Change on PIN PB2 will trigger a PCI0 interrupt request
    sei(); // enable interrupts again

    while(1){
        newCommand = SPI_transferData(0xFF);
        switch(newCommand){
            case cmdBmeStart:
                // do the measurements
                bmeTemperature = bme.readTemperature();
                bmePressure = bme.readPressure();
                bmeHumidity = bme.readHumidity();
                cli();
            break;
            case cmdBmeGetTemperature:
                // send the temperature data (4 bytes)
                SPI_transferData((uint8_t) (bmeTemperature >> 24));
                SPI_transferData((uint8_t) (bmeTemperature >> 16));
                SPI_transferData((uint8_t) (bmeTemperature >> 8));
                SPI_transferData((uint8_t) bmeTemperature);
            break;
            case cmdBmeGetPressure:
                // send the temperature data (4 bytes)
                SPI_transferData((uint8_t) (bmePressure >> 24));
                SPI_transferData((uint8_t) (bmePressure >> 16));
                SPI_transferData((uint8_t) (bmePressure >> 8));
                SPI_transferData((uint8_t) bmePressure);
            break;
            case cmdBmeGetHumidity:
                // send the temperature data (4 bytes)
                SPI_transferData((uint8_t) (bmeHumidity >> 24));
                SPI_transferData((uint8_t) (bmeHumidity >> 16));
                SPI_transferData((uint8_t) (bmeHumidity >> 8));
                SPI_transferData((uint8_t) bmeHumidity);
            break;
            case cmdLtrStart:
                ltrCh0 = ltr.readChannel0();
                ltrCh1 = ltr.readChannel1();
                cli();
            break;
            case cmdLtrGetCh0:
                // send the visible light (2 bytes)
                SPI_transferData((uint8_t) (ltrCh0 >> 8));
                SPI_transferData((uint8_t) ltrCh0);
            break;
            case cmdLtrGetCh1:
                // send the visible light (2 bytes)
                SPI_transferData((uint8_t) (ltrCh1 >> 8));
                SPI_transferData((uint8_t) ltrCh1);
            break;
            case cmdMl8511Start:
                // do the measurements
                ml8511UvLevel = ml8511.readUvLevel(3);
                cli();
            break;
            case cmdMl8511GetValue:
                // send the uv level (2 bytes)
                SPI_transferData((uint8_t) (ml8511UvLevel >> 8));
                SPI_transferData((uint8_t) ml8511UvLevel);
            break;
            case cmdSleep:
                set_sleep_mode(SLEEP_MODE_PWR_DOWN);
                power_adc_disable();
                sleep_enable();
                sei();
                sleep_cpu();
                sleep_disable();
                if (PINB & (1 << PB2) ){
                    // SS is high -> this was the wrong signal -> sleep again
                    sleep_enable();
                    sleep_cpu();
                    sleep_disable();
                }
                power_adc_enable();
            break;
        }
    }
}


