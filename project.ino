/*
 *  Ricky Aguiar
 *	Firstbuild   
 *	TDS meter
 *
 *  This sketch will provide the transfer function and send the data to the Redbear app Via bluetooth
 *
 */


//"SPI.h/Nordic_nRF8001.h/RBL_nRF8001.h" are needed in every new project
#include <SPI.h>
#include <Nordic_nRF8001.h>
#include <RBL_nRF8001.h>
#define TDS A3

void setup()
{

  ble_set_name("TDS Meter");
  
  // Init. and start BLE library.

  ble_begin(); // does ble communicate on a particular pin?

  pinMode(TDS, INPUT);  
  
  // Enable serial debug
  Serial.begin(9600);
}

unsigned char * buf[32] = {0}; // unused, and it's an array of strings
unsigned char len = 0;
uint16_t strLen;						//two byte reserved
uint16_t strLen2;
char formatStr[25];				    // a 1x25 empty array 
char formatStr2[50];
float alpha = 0.6; // weight on new values in the running average
static float inputVoltage = 0.0; // set to zero then gradually increases
//static float averagedVoltage=0;

void loop()
{
 inputVoltage = 0.9*averagedVoltage + 0.1*avgFun();
 //inputVoltage = (alpha * (analogRead(TDS) * (3.3/1023.0))) + ((1 - alpha) * inputVoltage); //avgFun();		//100 voltage samples get averaged 
  // this is a rough running average, that takes a fraction of the new input into the calculation (it's an exponential moving average)
  // this might not work best for a fluctuating input, but at least it's here, and you can play with the alpha above
 uint16_t PPM =  (-114.27*inputVoltage*inputVoltage + 1016.7*inputVoltage);  //POLY transfer function
 uint16_t VOLTAGE = analogRead(A3);
 
 // will only print positive values
 if (PPM > 0) {
    strLen = sprintf(formatStr, "PPM: %d \n", PPM);		//(to be opened for reading, type, parameter to be read)
 } else {
    strLen = sprintf(formatStr, "PPM: %d \n", 0); 
 }

 if (VOLTAGE > 0) {
    strLen2 = sprintf(formatStr2, "Raw Voltage: %d \n", VOLTAGE); // stores the length of formatStr and formatStr2 into
    // the strLen variables
 } else {
    strLen2 = sprintf(formatStr2, "Raw Voltage: %d \n", 0.00);
    // both values 
 }
  
  
  if ( ble_connected() )
  {
  
   ble_write_bytes((byte *)&formatStr, strLen); // passes to the strings to the ble device, specifying their lenght
   ble_write_bytes((byte *)&formatStr2, strLen2);
   
    
  }

  ble_do_events();
  
  if ( ble_available() )
  {
    while ( ble_available() )
    {
      Serial.write(ble_read()); // this is all sent as bites, so it probably prints 0 & 1's, is this meant to write
      // to a device not the serial monitor?
    }
    
    Serial.println();
  }
 delay(1000);
}

// function to average samples of voltages being read on the analog pin
float avgFun() {
  float inputVoltageVal = 0.0;
  float numOfSamp = 10;
  for (int i = 0; i < numOfSamp; i++) {
    inputVoltageVal += (analogRead(TDS) * (3.3/1023.0)); // this is all instantaneous, 
    // should we have it wait for a bit to the next sample?
  }
  inputVoltageVal /= numOfSamp;
  // not a bad method overall
  
  return inputVoltageVal;
}
