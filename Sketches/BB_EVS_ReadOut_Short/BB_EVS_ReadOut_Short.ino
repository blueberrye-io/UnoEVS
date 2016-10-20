/**
 * Trigger measurements on an UnoEVS and receive data from it.
 *
 * v0.01 created 19. Oct. 2016
 * by Engelbert Mittermeier (BlueberryE GmbH)
 */

// The following two functions are used for SPI communication
// This can easily be replaced by the SPI.h library

// initialize the SPI as master
void my_Spi_init(){
  pinMode(SS, OUTPUT);
  SPCR |= _BV(MSTR);
  SPCR |= _BV(SPE);
  SPCR |= _BV(SPR1);
  SPCR |= _BV(SPR0);
  pinMode(SCK, OUTPUT);
  pinMode(MOSI, OUTPUT);
}

// used for data transfer (read and write)
uint8_t my_Spi_transfer(uint8_t inData){
  SPDR = inData;
    while (!(SPSR & _BV(SPIF))) ; // wait
    return SPDR;
}


// Define codes for commands:

// set the UnoEVS to sleep mode
const uint8_t cmdSleep = 0xF0;                

// commands for the BME280 (Temperature, Pressure, Humidity):
// trigger the measurements
const uint8_t cmdBmeStart = 0x10;
// get the temperature value
const uint8_t cmdBmeGetTemperature = 0x11;
// get the pressure value
const uint8_t cmdBmeGetPressure = 0x12;
// get the humidity value
const uint8_t cmdBmeGetHumidity = 0x13;

// commands for the LTR303ALS01 (visible light):
// trigger the measurements
const uint8_t cmdLtrStart = 0x20;
// get the value from channel 0
const uint8_t cmdLtrGetCh0 = 0x21;
// get the value from channel 0
const uint8_t cmdLtrGetCh1 = 0x22;

// commands for the ML8511 (UV):
// trigger the measurements
const uint8_t cmdMl8511Start = 0x30;
// get the value from the sensor
const uint8_t cmdMl8511GetValue = 0x31;


// Define variables taking the measurement data:
// variables for the bme280
uint8_t bmeTemperature[4]; // the temperature value consists of 4 bytes
uint8_t bmePressure[4];    // the pressure value consists of 4 bytes
uint8_t bmeHumidity[4];    // the humidity value consists of 4 bytes

// variables for the ltr303als01 (visible light)
uint8_t ltrVisibleCh0[2];  // the value from channel 0 consists of 2 bytes
uint8_t ltrVisibleCh1[2];  // the value from channel 1 consists of 2 bytes

// variable for the ml8511 (uv)
uint8_t ml8511Value[2];    // the value from the ml8511

// waiting times: time for wake up, sensor measurement time, calculation time, ...
const uint16_t waitTime = 10;
const uint16_t waitTimeMl8511 = 200; //10ms is too less -> the ml8511 waits 100ms before reading the data

void setup() {
  Serial.begin(9600);
  digitalWrite(SS, HIGH);
  my_Spi_init();
  digitalWrite(SS, LOW);
  my_Spi_transfer(cmdSleep);
  digitalWrite(SS, HIGH);
  Serial.println("Debug Message : Setup completed");
}

void loop() {
    
    // Trigger BME280 meaurements and get data:
    
    digitalWrite(SS, LOW); // select the UnoEVS + wake up
    delay(waitTime);

    // trigger the bme280 measurements
    my_Spi_transfer(cmdBmeStart);
    delay(waitTime);
          
    // fetch the bme280 data from the EVS
    my_Spi_transfer(cmdBmeGetTemperature);
    for (unsigned int i = 0; i < sizeof(bmeTemperature); i++) bmeTemperature[i] = my_Spi_transfer(0xFF);
    my_Spi_transfer(cmdBmeGetPressure);
    for (unsigned int i = 0; i < sizeof(bmeTemperature); i++) bmePressure[i] = my_Spi_transfer(0xFF);
    my_Spi_transfer(cmdBmeGetHumidity);
    for (unsigned int i = 0; i < sizeof(bmeTemperature); i++) bmeHumidity[i] = my_Spi_transfer(0xFF);

    my_Spi_transfer(cmdSleep); // set the UnoEVS to sleep
                
    digitalWrite(SS, HIGH); 
               
    // print out the data or do some other stuff:
    Serial.print("T = "); Serial.print(convertArray(bmeTemperature, 4, 1.0 / 100.0)); Serial.println("degC");
    Serial.print("P = "); Serial.print(convertArray(bmePressure, 4, 1.0 / 100.0)); Serial.println("hPa");
    Serial.print("H = "); Serial.print(convertArray(bmeHumidity, 4, 1.0 / 1024.0)); Serial.println("%");

    // Trigger LTR303ALS01 (visible light) meaurements and get data:

    digitalWrite(SS, LOW); // select the UnoEVS + wake up
    delay(waitTime);

    // trigger the ltr303als01 measurements
    my_Spi_transfer(cmdLtrStart);
    delay(waitTime);

    // fetch the ltr303als01 data from the EVS
    my_Spi_transfer(cmdLtrGetCh0);
    for (unsigned int i = 0; i < sizeof(ltrVisibleCh0); i++) ltrVisibleCh0[i] = my_Spi_transfer(0xFF);
    my_Spi_transfer(cmdLtrGetCh1);
    for (unsigned int i = 0; i < sizeof(ltrVisibleCh1); i++) ltrVisibleCh1[i] = my_Spi_transfer(0xFF);
                
    my_Spi_transfer(cmdSleep); // set the UnoEVS to sleep
                
    digitalWrite(SS, HIGH); 
               
    // print out the data or do some other stuff:
    Serial.print("CH0 = ");
    Serial.print("CH0 = "); Serial.println(convertArray(ltrVisibleCh0, 2, 1.0));
    Serial.print("CH1 = ");
    Serial.print("CH1 = "); Serial.println(convertArray(ltrVisibleCh1, 2, 1.0));
                   
    // Trigger ML8511 (UV) meaurements and get data:

    digitalWrite(SS, LOW); // select the UnoEVS + wake up
    delay(waitTime);

    // trigger the ml8511 measurements
    my_Spi_transfer(cmdMl8511Start);
    delay(waitTimeMl8511);

    // fetch the ml8511 data from the EVS
    my_Spi_transfer(cmdMl8511GetValue);
    for (unsigned int i = 0; i < sizeof(ml8511Value); i++) ml8511Value[i] = my_Spi_transfer(0xFF);
    
    my_Spi_transfer(cmdSleep); // set the UnoEVS to sleep
                
    digitalWrite(SS, HIGH); 
                
    Serial.print("UV = ");
    Serial.println((((uint32_t) ml8511Value[0]) <<  8) | 
                   ((uint32_t)  ml8511Value[1]));
    Serial.print("UV = "); Serial.println(convertArray(ml8511Value, 2, 1.0));

    Serial.println("--------------------------------");
    delay(2000); // wait 2 s before the next cycle
}

/**
 * Converts an array containing raw measurement data to one value.
 * @param measValues[] the raw data with sequence from high to low
 * @param dataCount the number of values in measValues[]
 * @param scale a value used for scaling the resulting data.
 * 
*/
float convertArray(uint8_t measValues[], uint8_t dataCount, float scale){
    uint32_t tempValue = 0x00;
    for (unsigned int i = 0; i < dataCount; i++) tempValue = tempValue | ((uint32_t) measValues[i] << ( (dataCount - 1 - i) * 8));
    /*
    Serial.println();
    for (int i = 0; i < dataCount; i++){
        Serial.println(measValues[i]);
        Serial.println((uint32_t) measValues[i]);
        Serial.println((uint32_t) measValues[i] << ( (dataCount - 1 - i) * 8));
        tempValue = tempValue | ((uint32_t) measValues[i] << ( (dataCount - 1 - i) * 8));
        Serial.println(tempValue);
        Serial.println("x-x-");
    }
    Serial.println();
    */
    return tempValue * scale;
}    


