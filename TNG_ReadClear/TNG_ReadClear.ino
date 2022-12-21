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
 * This sketch is used to manage data stored to an I2C EEPROM via the TNG_logger sketch.
 *
 * Currently has three functions:
 *     [R] Reads all locations until fix_attempt == 0. 
 *     [C] Clears EEPROM by writing zeros to first 4000 addresses. If more are needed, increase i.
 *     [Z] is super secret: prints first i number of locations. For debugging.
 * 
 ***************************************************************************************************************************/

#include <Wire.h>
#include "eepromi2c.h"

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
int address;

int numberLines = 35;   // used in Z case. Force prints x number of lines.

void setup(){
  
    Serial.begin(9600);
    Wire.begin();

    pinMode(transistorEEPROM, OUTPUT);
    pinMode(transistorGPS, OUTPUT);

    digitalWrite(transistorEEPROM, HIGH); 
    digitalWrite(transistorGPS, LOW); 

    
//    // blinks fast means read/clear sketch is loaded
//    for (int i = 0; i < 5; i++) {
//        digitalWrite(LED_BUILTIN, HIGH);
//        delay(250);
//        digitalWrite(LED_BUILTIN, LOW);
//        delay(250);
//        }

    menu();
}


void loop(){

    if (Serial.available() == 1) {
      switch (toUpperCase(Serial.read())) {
        
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
          menu();
        break;
      }
    }
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
  Serial.println("Please type:");
  Serial.println("C - clear memory");
  Serial.println("R - Read locations (Reads each location)");
  Serial.print("Z - force print the first "); Serial.print(numberLines);Serial.println(" lines");
}
