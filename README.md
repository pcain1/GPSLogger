## README ##
***

### GPS data logger code ###

This sketch is for anyone interested in creating a GPS data logger. Our aim is to use this for tracking movements of turtles.


### How do I get set up? ###

Materials can be found the original version can be found [here](http://sfe.io/w126703).    

PCB designs and components are at [easyeda.com](https://easyeda.com/Splat01/Expanded_Memory-bc33ae210a844b5aa67b5967a7301c84).

A video of construction can be found [here](https://youtu.be/LDO5o6MhcLM).  

The Arduino sketches are in two folders. The **TOS** folder is for the logger version that uses Atmega onboard EEPROM. The **TNG** folder is for the newer, surface mount component, version with expanded memory.

**TOS_ReadClear.ino** and **TNG_ReadClear.ino** sketches handle data for the older, onboard EEPROM, and new external EEPROM versions, respectively. Load the sketch to the Arduino, and boot up a Serial Monitor at 9600. Typing an "R" (capital) will read through all saved locations until it runs out of locations. "C" will assign a value, 0, to all addresses (basically erasing all data). "Z" will forceably output every location, even if there was not an actual reading. This is needed if you don't get a location at a particular time, but know there might be more locations afterwards. The number of lines read is determined by the `numberLines` variable in the sketch.

**TOS_logger.ino** and **TNG_logger.ino** are sketches that control the GPS receiver. The main variables that require assignment are  `stay_on`,which controls how long the GPS receiver is on (in minutes), affecting satellite acquisition time. Longer times means more satellite fixes. `logger_interval` controls time when receiver is not active (sleep mode) in hours. For example, a value of 5 will wake the receiver up every [(5 \* 60 \* 60) / 8] sleep cycles.
 
### Desired additions ###
We're working on adding a probe that detects water so that the receiver only wakes when turtles is basking. This will allow the recording of locations when the turtle is out of water, otherwise the receiver will not be able to get a fix.

### Who do I contact? ###

Questions or comments can be sent to pcain1@ggc.edu or matt.cross@toledozoo.org.



***

[![Creative Commons License](https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png)](http://creativecommons.org/licenses/by-nc-sa/4.0/)  
Arduino based GPS Logger by [Patrick Cain](http://bitbucket.org/splat01) and Matt Cross is licensed under a [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-nc-sa/4.0/).

