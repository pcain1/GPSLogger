/***************************************************************************************
 *                  Code for a turtle GPS receiver and logger
 *                                 October 2017
 *                  P. Cain, Georgia Gwinnett College, Lawrenceville, GA                 
 *                                     -and- 
 *                      M. Cross, Toledo Zoo, Toledo, OH
 *
 * Arduino based GPS Logger by Patrick Cain and Matt Cross is licensed under a 
 * Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *
 * Microcontroller is the Arduino Pro Mini 3v3: https://www.sparkfun.com/products/11114 
 * GPS Receiver: https://www.sparkfun.com/products/13740
 * 
 * Wiring:
 * -GPS-
 * Power (red wire) to Vcc  
 * GND (black wire) to transistor to GND
 * Tx (white wire) to D4
 * 
 * -transistor-
 * PN2222 Transistor: base (middle pin) to Pin D2
 * 
 ***************************************************************************************/

// Libraries and links to download. 
//For info on installing Arduino libraries, see https://www.arduino.cc/en/Guide/Libraries
#include <SoftwareSerial.h>
#include <TinyGPS++.h>      // http://arduiniana.org/libraries/tinygpsplus/
#include <LowPower.h>       // https://github.com/rocketscream/Low-Power
#include <EEPROMex.h>       // http://thijs.elenbaas.net/2012/07/extended-eeprom-library-for-arduino

/*********************( USER DEFINED VARIABLES )*****************
Two varibles to define are how long the GPS will stay on to acquire a signal (stay_on). Tradeoff GPS 
up time with battery life. The other variable is how often the unit wakes and logs data (logger_interval). 
This occurs in hour intervals, starting on power up. Twelve hours will give you two points a day, eight
will give you three, etc. */

// how long will GPS receiver stay on
int stay_on = 2; /*minutes*/

// how long is the data logging interval? (time between readings)
int logger_interval = 0; /*hours*/
/**************************************************************/


uint32_t feedDuration = stay_on * 60000;                    //time GPS is awake and feeding data to variable containers
uint32_t sleepInterval = (logger_interval * 60 * 60) / 8;   //raw number of cycles of watchdog interrupt 

static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;

const int memBase = 0;

int address_location;

TinyGPSPlus gps;

SoftwareSerial ss(RXPin, TXPin);

uint32_t GPS_run = millis();
uint32_t timer = millis();

#define transPin 2

//------------------------------------------( Setup )------------------------------------------//
void setup(){
  Serial.begin(9600);
  ss.begin(GPSBaud);

  // Set pin as output to toggle off GPS
  pinMode(transPin, OUTPUT);

  EEPROM.setMemPool(memBase, EEPROMSizeUno);
  const int maxAllowedWrites = 1023;
  EEPROM.setMaxAllowedWrites(maxAllowedWrites);
  
  address_location = EEPROM.readByte(EEPROMSizeUno-1);   //reassign last use address value to address variable
  
  for (int i = 0; i < 5; i++){
   digitalWrite(LED_BUILTIN,HIGH);    //set the LED on
   delay(200);                        //wait for a second
   digitalWrite(LED_BUILTIN,LOW);     //set the LED off
   delay(200); 
  }
}
//------------------------------------------( END Setup )------------------------------------------//


//------------------------------------------( Start Loop )------------------------------------------//
void loop(){
  
    Serial.print(F("Feeding GPS..."));
    digitalWrite(transPin, HIGH);   
    
    GPS_run = millis();
    while (millis() - GPS_run < feedDuration){
        feedGPS();
        }    
    Serial.println(F("Done"));

    Serial.println(F("Turning off transistor"));
    digitalWrite(transPin, LOW);
    
    Serial.print(F("Writing data to EEPROM..."));
    dataWrite();       
    Serial.println(F("Done"));
    Serial.print(F("Sleeping..."));

    delay(100);
    
    for (int i = 0; i < sleepInterval; i++){
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
      }

    Serial.println(F("Done."));
}
//------------------------------------------( END Loop )------------------------------------------//

//------------------------------------------( Functions )------------------------------------------//
bool feedGPS() {
  while (ss.available()) {
    if (gps.encode(ss.read()))
      return true;
      }
  return false;
}

void dataWrite(){
  EEPROM.writeByte(address_location,1);
  address_location += sizeof(byte);
  EEPROM.writeDouble(address_location, gps.location.lat());
  address_location += sizeof(double);
  EEPROM.writeDouble(address_location, gps.location.lng());
  address_location += sizeof(double);
  EEPROM.writeByte(address_location, gps.date.day());
  address_location += sizeof(byte);
  EEPROM.writeByte(address_location, gps.date.month());
  address_location += sizeof(byte);
  EEPROM.writeByte(address_location, gps.time.hour());
  address_location += sizeof(byte);
  EEPROM.writeByte(address_location, gps.time.minute());
  address_location += sizeof(byte);
  EEPROM.writeByte(EEPROMSizeUno-1, address_location); //Saves address in EEPROM so a reset doesn't over write data. 
  delay(100);
  }

//------------------------------------------( END PROGRAM )------------------------------------------//
