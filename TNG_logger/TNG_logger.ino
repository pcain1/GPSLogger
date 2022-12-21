/****************************************************************************************************************************
 *                  Code for a turtle GPS receiver and logger
 *                                 October 2017
 *                  P. Cain, Georgia Gwinnett College, Lawrenceville, GA                 
 *                                    -and- 
 *                        M. Cross, Toledo Zoo, Toledo, OH
 *
 * Arduino based GPS Logger by Patrick Cain and Matt Cross is licensed under a 
 * Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *
 * Microcontroller is the Arduino Pro Mini 3v3: https://www.sparkfun.com/products/11114 
 * 
 * GPS Receiver: https://www.sparkfun.com/products/13740
 * 
 ***************************************************************************************************************************/

// Libraries and links to download. 
//For info on installing Arduino libraries, see https://www.arduino.cc/en/Guide/Libraries
#include <SoftwareSerial.h>
#include <TinyGPS++.h>      // http://arduiniana.org/libraries/tinygpsplus/
#include <LowPower.h>       // https://github.com/rocketscream/Low-Power
#include <Wire.h>
#include <eepromi2c.h>      //http://apexlogic.net/code-bank/arduino/eeprom-i2c-write-anything/


/*********************( USER DEFINED VARIABLES )*****************
/***************************************************************/
/***************************************************************/
/***************************************************************
Two varibles to define are how long the GPS will stay on to acquire a signal (stay_on). Tradeoff GPS 
up time with battery life. The other variable is how often the unit wakes and logs data (logger_interval. 
This occurs in hour intervals, starting on power up. Twelve hours will give you two points a day, eight
will give you three, etc. */

// how long will GPS receiver stay on?
int stay_on = 1; /*minutes*/

// how long is the data logging interval? (time between readings)
float logger_interval = 0.25; /*hours*/
/***************************************************************/
/***************************************************************/
/***************************************************************/
/***************************************************************/

  
uint32_t feedDuration = stay_on * 60000;                    //time GPS is awake and feeding data to variable containers
uint32_t sleepInterval = (logger_interval * 60 * 60) / 8;   //raw number of cycles of watchdog interrupt 

static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;
uint32_t GPS_run = millis(), timer = millis();

unsigned short address;     //number range is 0 - 65,535
unsigned short address_location = 3999;

TinyGPSPlus gps;

SoftwareSerial ss(RXPin, TXPin);

#define transistorGPS 2
#define transistorEEPROM 5

struct config {
  byte fix_attempt;
  double lat;
  double lon;
  byte day;
  byte month;
  byte hour;
  byte minute;
//  byte second;
//  byte satellites;
//  int altitude;
} config;



//------------------------------------------( Setup )------------------------------------------//
void setup(){
  
    Serial.begin(9600);
    ss.begin(GPSBaud);
    Wire.begin();

    Serial.println(F("Starting GPS..."));
  
    // Set pin as output to toggle off GPS
    pinMode(transistorGPS, OUTPUT);
    pinMode(transistorEEPROM, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    digitalWrite(transistorEEPROM, HIGH);
    eeRead(address_location, address);
    delay(10);
    digitalWrite(transistorEEPROM, LOW);

    // blinks means this sketch is loaded
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
        }
}

//------------------------------------------( END Setup )------------------------------------------//


//------------------------------------------( Start Loop )------------------------------------------//
void loop(){

    digitalWrite(transistorGPS, HIGH);

    delay(1000);                       
  
    GPS_run = millis();
    while (millis() - GPS_run < feedDuration){
        feedGPS();
        }    
        
    config.fix_attempt = 1;    
    config.lat = gps.location.lat();
    config.lon = gps.location.lng();
    config.day = gps.date.day();
    config.month = gps.date.month();
    config.hour = gps.time.hour();
    config.minute = gps.time.minute();
    
    digitalWrite(transistorGPS, LOW);

    digitalWrite(transistorEEPROM, HIGH);
    delay(10);
    eeWrite(address,config);
    
    address += sizeof(config);
    eeWrite(address_location, address);
    
    delay(100);

    digitalWrite(transistorEEPROM, LOW);

    printLocation();
    delay(200);

    for (int i = 0; i < sleepInterval; i++){
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
      } 

}
//------------------------------------------( END Loop )------------------------------------------//

//----------------------------( Functions )-------------------------//
bool feedGPS() {
    while (ss.available()) {
      if (gps.encode(ss.read()))
        return true;
        }
    return false;
}

void printLocation() {

        Serial.print(config.lat,6); Serial.print(",");
        Serial.print(config.lon,6); Serial.print(",");
        Serial.print(config.day); Serial.print("/");
        Serial.print(config.month);Serial.print(",");
        if (config.hour < 10) Serial.print("0");
        Serial.print(config.hour); Serial.print(":");
        if (config.minute < 10) Serial.print("0");
        Serial.println(config.minute);/*Serial.print(":");
        if (config.second < 10) Serial.print("0");
        Serial.print(config.second); Serial.print(",");
        Serial.print(config.satellites);Serial.print(",");
        Serial.print(config.altitude);*/
        Serial.print(address);Serial.print("  ");Serial.println(address_location);
        delay(10);  
}

//------------------------------------------( END PROGRAM )------------------------------------------//
