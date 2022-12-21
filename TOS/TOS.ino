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
 * This sketch is used to manage data stored via the TOS_logger sketch.
 *
 * Currently has three functions:
 *     [R] Reads all locations until fix_attempt == 0. 
 *     [C] Clears EEPROM by writing zeros to all addresses.
 *     [Z] is super secret: prints first i number of locations. For debugging. i == numberLines
 * 
 ***************************************************************************************************************************/

#include <SoftwareSerial.h>
#include <TinyGPS++.h>      // http://arduiniana.org/libraries/tinygpsplus/
#include <LowPower.h>       // https://github.com/rocketscream/Low-Power
#include <EEPROMex.h>       // http://thijs.elenbaas.net/2012/07/extended-eeprom-library-for-arduino

#define transistorGPS 2

/*********************( USER DEFINED VARIABLES )*****************
/***************************************************************/
/***************************************************************/
/***************************************************************
Two varibles to define are how long the GPS will stay on to acquire a signal (stay_on). Tradeoff GPS 
up time with battery life. The other variable is how often the unit wakes and logs data (logger_interval). 
This occurs in hour intervals, starting on power up. Twelve hours will give you two points a day, eight
will give you three, etc. */

// how long will GPS receiver stay on
int stay_on = 2; /*minutes*/

// how long is the data logging interval? (time between readings)
float logger_interval = 0; /*hours*/
/***************************************************************/
/***************************************************************/
/***************************************************************/
/***************************************************************/

uint32_t feedDuration = stay_on * 60000;                    //time GPS is awake and feeding data to variable containers
uint32_t sleepInterval = (logger_interval * 60 * 60) / 8;   //raw number of cycles of watchdog interrupt 

static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;
uint32_t GPS_run = millis(), timer = millis();

const int memBase = 0;

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

int numberLines = 25;   // used in Z case. Force prints x number of lines.

int address = 0;
int address_location;


byte fix_attempt;
double latOut;
double lonOut;
byte dayOut;
byte monthOut;
byte hourOut;
byte minuteOut;
byte secOut;

int addressDouble = sizeof(double);
int addressByte   = sizeof(byte);


int x = 0;  // Flag for Read loop
byte flag = 0;
unsigned long current = 0;

//------------------------------------------( Setup )------------------------------------------//
void setup() {
  Serial.begin(9600);
  ss.begin(GPSBaud);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  // Set pin as output to toggle off GPS
  pinMode(transistorGPS, OUTPUT);
  digitalWrite(transistorGPS, LOW);   
  
  EEPROM.setMemPool(memBase, EEPROMSizeUno);
  const int maxAllowedWrites = 1024;
  EEPROM.setMaxAllowedWrites(maxAllowedWrites);

  address = EEPROM.readByte(EEPROMSizeUno-1);   //reassign last use address value to address variable
  
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
      
  Serial.println("");
  Serial.println("Entering Logging mode.");
  if (flag == 0) blinkForLogger();      

}
//------------------------------------------( END Setup )------------------------------------------//

//------------------------------------------( Start Loop )------------------------------------------//
void loop() {
 
  while (flag == 1) {
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
      
      /****** erases EEPROM by writing a '0' to each address *******/
      case 'C':   
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println();
        Serial.println("************* ERASE Begin *****************");
        Serial.println();
        Serial.print("Clearing EEPROM...");

        for (int i = 0 ; i < EEPROMSizeUno; i++ )
          EEPROM.write(i, 0);

        Serial.println("Done");
        digitalWrite(LED_BUILTIN, LOW); Serial.println();
        Serial.println("************* ERASE Done *****************");

        menu();
        
        break;
       /*---------------------------------------------------------------*/


      /************************* read EEPROM *********************/
      case 'R':   // 
        digitalWrite(LED_BUILTIN, HIGH);

        // Reset flag and start address
        x = 0; address = 0;

        Serial.println();
        Serial.println("************* COPY Begin *****************");
        Serial.println("Lat,Lon,Day/Month,UTC_Time");

        while (x == 0) {
          fix_attempt = EEPROM.readByte(address); address += addressByte;
          latOut = EEPROM.readDouble(address); address += addressDouble;
          lonOut = EEPROM.readDouble(address); address += addressDouble;
          dayOut = EEPROM.readByte(address);   address += addressByte;
          monthOut = EEPROM.readByte(address); address += addressByte;
          hourOut = EEPROM.readByte(address);  address += addressByte;
          minuteOut = EEPROM.readByte(address); address += addressByte;
          //secOut = EEPROM.readByte(address);   address += addressByte;

          if (fix_attempt == 0) x = 1;

          else {
            Serial.print(latOut, 6); Serial.print(",");
            Serial.print(lonOut, 6); Serial.print(",");
            Serial.print(dayOut); Serial.print("/");
            Serial.print(monthOut); Serial.print(",");
            if (hourOut < 10) Serial.print("0");
            Serial.print(hourOut); Serial.print(":");
            if (minuteOut < 10) Serial.print("0");
            Serial.print(minuteOut);//Serial.print(":");
            //Serial.print(secOut);
            Serial.println();
            delay(200);
          }
        }

        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("************* Finished *****************"); Serial.println();
        
        menu();
        
        break;
       /*---------------------------------------------------------------*/
      
      case 'Z':
        address = 0;
        Serial.println();Serial.print("*************Printing the first ");Serial.print(numberLines);Serial.println(" lines*************");
        Serial.println("Lat,Lon,Day/Month,UTC_Time");
        for (int i = 0; i < numberLines; i++) {
          fix_attempt = EEPROM.readByte(address); address += addressByte;
          latOut = EEPROM.readDouble(address); address += addressDouble;
          lonOut = EEPROM.readDouble(address); address += addressDouble;
          dayOut = EEPROM.readByte(address);   address += addressByte;
          monthOut = EEPROM.readByte(address); address += addressByte;
          hourOut = EEPROM.readByte(address);  address += addressByte;
          minuteOut = EEPROM.readByte(address); address += addressByte;
          //secOut = EEPROM.readByte(address);   address += addressByte;

          Serial.print(fix_attempt); Serial.print(",");
          Serial.print(latOut, 6); Serial.print(",");
          Serial.print(lonOut, 6); Serial.print(",");
          Serial.print(dayOut); Serial.print("/");
          Serial.print(monthOut); Serial.print(",");
          if (hourOut < 10) Serial.print("0");
          Serial.print(hourOut); Serial.print(":");
          if (minuteOut < 10) Serial.print("0");
          Serial.print(minuteOut);//Serial.print(":");
          //Serial.print(secOut);
          Serial.println();
          delay(200);
        }
        Serial.println("************* Finished *****************"); Serial.println();

        menu();

        break;
    }
  }

  Serial.print(F("Feeding GPS..."));
  digitalWrite(transistorGPS, HIGH);   
  
  GPS_run = millis();
  while (millis() - GPS_run < feedDuration){
      feedGPS();
      }    
  Serial.println(F("Done"));

  Serial.println(F("Turning off transistor"));
  digitalWrite(transistorGPS, LOW);
  
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
void menu() {
  Serial.println("___________MENU___________");
  Serial.println("Please choose:");
  Serial.println("C - Clear memory.");
  Serial.println("R - Read each recorded location.)");
  Serial.print("Z - Force print the first "); Serial.print(numberLines);Serial.println(" lines (for debugging).");
  Serial.println("G - Switch to GPS mode. Restart device to return to menu options.");
  Serial.println("__________________________");
}

bool feedGPS() {
  while (ss.available()) {
    if (gps.encode(ss.read()))
      return true;
      }
  return false;
}

void dataWrite(){
  EEPROM.writeByte(address,1);
  address += sizeof(byte);
  EEPROM.writeDouble(address, gps.location.lat());
  address += sizeof(double);
  EEPROM.writeDouble(address, gps.location.lng());
  address += sizeof(double);
  EEPROM.writeByte(address, gps.date.day());
  address += sizeof(byte);
  EEPROM.writeByte(address, gps.date.month());
  address += sizeof(byte);
  EEPROM.writeByte(address, gps.time.hour());
  address += sizeof(byte);
  EEPROM.writeByte(address, gps.time.minute());
  address += sizeof(byte);
  EEPROM.writeByte(EEPROMSizeUno-1, address); //Saves address in EEPROM so a reset doesn't over write data. 
  delay(100);
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
