/****************************************************************************************************************************
 *                  Code for a turtle GPS receiver and logger
 *                                 April 2019
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
 * Connect to computer via FTDI adapter and open Serial Monitor. Type <y> within 20 seconds to select menu. Otherwise, 
 *  logging mode will begin.
 *  
 * Menu currently has the following functions:
 *     [R] Reads all locations until fix_attempt == 0. 
 *     [C] Clears EEPROM by writing zeros to first 4000 addresses. If more are needed, increase i.
 *     [Z] prints i number of locations. Set the number of locations to print by setting <numberLines> vairable.
 *     [G] returns to logging mode.
 ***************************************************************************************************************************/

#include <Wire.h>
#include <eepromi2c.h>      //http://apexlogic.net/code-bank/arduino/eeprom-i2c-write-anything/

#include <SoftwareSerial.h>
#include <TinyGPS++.h>      // http://arduiniana.org/libraries/tinygpsplus/
#include <LowPower.h>       // https://github.com/rocketscream/Low-Power

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

unsigned short address_temp;
int numberLines = 35;   // used in Z case. Force prints x number of lines.
byte flag = 0;
unsigned long current = 0;

void setup(){
    Serial.begin(9600);
    Wire.begin();
    ss.begin(GPSBaud);
   
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(transistorGPS, OUTPUT);
    digitalWrite(transistorGPS, LOW); 
        
    pinMode(transistorEEPROM, OUTPUT);
    digitalWrite(transistorEEPROM, HIGH);
    eeRead(address_location, address);
    delay(10);
    digitalWrite(transistorEEPROM, LOW);

    Serial.println("Computer detected.");
    Serial.println("");
    Serial.println("Type \"Y\" to enter data collection mode.");
    Serial.println("Entering Logger mode in 20 seconds.");
    Serial.println("Don't panic."); 
    Serial.println("If you miss this window, just restart the device to again enter data collection mode.");
    Serial.println("");

    current = millis();
    while (millis() - current < 20000 && flag != 1){
        if (Serial.available() == 1) {
            if (toUpperCase(Serial.read()) == 'Y') {
                flag = 1;
                menu();   
            }
        }
    }
    
    
    if (flag == 0){
      Serial.println("");
      Serial.println("Entering Logging mode.");
      blinkForLogger(); 
    }
        
}

//------------------------------------------( Start Loop )------------------------------------------//
void loop(){

//------------------------------------------( ReadClear Loop )------------------------------------------//
  while (flag == 1) {
      digitalWrite(transistorEEPROM, HIGH); 
      digitalWrite(transistorGPS, LOW); 

      switch (toUpperCase(Serial.read())) {
        
        /************************* Switch to GPS logging mode *********************/        
        case 'G':{
          Serial.println("");
          Serial.println("Exiting ReadClear mode.....Entering Logging mode.");
          flag = 0;
          blinkForLogger();       
        }
        break;
        /*---------------------------------------------------------------*/
                 
        /************************* read EEPROM *********************/        
        case 'R':{
          digitalWrite(LED_BUILTIN, HIGH);
          
         // Reset flag and start address
          int x = 0; address = 0;

          Serial.println();
          Serial.println("************* COPY Begin *****************");
          Serial.println("Lat,Lon,Day/Month,UTC_Time");

          while (x == 0) {
            eeRead(address, config);    //Read the first location into structure 'config'
            delay(100);

            if (config.fix_attempt == 0) {
              Serial.println();
              Serial.println("No more locations were recorded");
              Serial.println();
              x = 1;
            }
  
            else {
                printLocation(); 
              }

            address += sizeof(config);
            delay(5);
            }

          digitalWrite(LED_BUILTIN, LOW);
          Serial.println("************* Finished *****************"); Serial.println();
          menu();
        }   
        break;

        /****** erases EEPROM by writing a '0' to each address *******/
        case 'C':{ 
          digitalWrite(LED_BUILTIN, HIGH);

          Serial.println("");
          Serial.println("--------Clearing I2C EEPROM--------");
          Serial.print("Pleast wait until light turns off...");
  
          for (int i = 0; i < 4000; i++) {
            eeWrite(i, 0); delay(6);
          }
          
          Serial.println("Done"); 
          Serial.println("---------Done Clearing---------");
          digitalWrite(LED_BUILTIN, LOW); Serial.println();
          menu(); 
        }
        break;
         /*---------------------------------------------------------------*/

         /********************** for testing *****************************/
        case 'Z':{ 
          digitalWrite(LED_BUILTIN, HIGH);

          eeRead(3999, address_temp);

          address = 0;
          Serial.println("");
          Serial.println("************* Begin *****************");          
          for (int i = 0 ; i < numberLines; i++){
            eeRead(address, config); delay(100);
            printLocation(); 
            address += sizeof(config); 
          }


          Serial.print("Last address: ");Serial.println(address_temp);
          digitalWrite(LED_BUILTIN, LOW); 
          Serial.println("************* End *****************");
          Serial.println();
          menu(); 
        }
        break;
        /*---------------------------------------------------------------*/
        
        default:
          // menu();
        break;
      }
     }
//------------------------------------------( END ReadClear Loop )------------------------------------------//


//------------------------------------------( Begin Logger Loop )------------------------------------------//
     
    digitalWrite(transistorGPS, HIGH);

    delay(100);                       
  
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

//------------------------------------------( End Logger Loop )------------------------------------------//

//  }
}
//------------------------------------------( END Loop )------------------------------------------//

//------------------------------------( Functions )---------------------------------//
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
        delay(10);  
}

void menu() {
  Serial.println("___________MENU___________");
  Serial.println("Please choose:");
  Serial.println("C - Clear memory");
  Serial.println("R - Read each recorded location");
  Serial.print("Z - Force print the first "); Serial.print(numberLines);Serial.println(" lines (for debugging).");
  Serial.println("G - Switch to GPS mode. Restart device to return to menu options.");
  Serial.println("__________________________");
}

void blinkForLogger() {
  for (int i = 0; i < 5; i++){
   digitalWrite(LED_BUILTIN,HIGH);    //set the LED on
   delay(500);                       //wait for a second
   digitalWrite(LED_BUILTIN,LOW);     //set the LED off
   delay(500); 
  }
}

//------------------------------------------( END PROGRAM )------------------------------------------//
