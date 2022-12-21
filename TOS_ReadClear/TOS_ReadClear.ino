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

#include <EEPROMex.h>

int numberLines = 25;   // used in Z case. Force prints x number of lines.

int address = 0;

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
const int memBase = 0;

// Flag for Read loop
int x = 0;

void setup() {
  Serial.begin(9600);

  EEPROM.setMemPool(memBase, EEPROMSizeUno);
  const int maxAllowedWrites = 1024;
  EEPROM.setMaxAllowedWrites(maxAllowedWrites);

  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    }

  digitalWrite(LED_BUILTIN, LOW);

  menu();

  delay(1000);
}

void loop() {

  if (Serial.available() == 1) {

    switch (toUpperCase(Serial.read())) {
      
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

  else {
    if (Serial.available() > 1) Serial.flush();
  }

}

void menu() {
  Serial.println();Serial.println("___________MENU___________");
  Serial.println("  C - Clears EEPROM.");
  Serial.println("  R - Read locations (Reads each location, but stops when Month == 0).");
  Serial.print("  Z - Force prints the first ");Serial.print(numberLines);Serial.println(" locations. Change variable <numberLines> if needed.");
  Serial.println("__________________________");
}
