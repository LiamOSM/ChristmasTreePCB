# Christmas Tree PCB
**A PCB Christmas Tree Ornament that glows and plays music!**

This is a thing I designed for McMaster University's IEEE Student Branch. An ATtiny85 microcontroller runs the program I created, which displays several light patterns. Pressing the "Light" button cycles through these patterns. The "Music" button starts a simple version of Jingle Bells, which loops forever until either of the buttons is pressed.

After cycling through all the light modes by pressing the "Light" button, there is an "off" mode. In this mode, the microcontroller is in a deep sleep and the entire circuit draws only 0.2 microamperes. With a standard 200mAh coin cell, the circuit will last over a hundred years in this sleep mode!

The "Light" button actually does a hard-reset to the microcontroller, which is how it's able to exit the sleep mode. Therefore, to make the button able to cycle through modes, the current state is stored in EEPROM. When the microcontroller resets, the state is read from EEPROM, incremented by one, and stored back to EEPROM. If the current state is zero, the microcontroller enters the deep sleep mode. Otherwise, it enters the corresponding light pattern mode.

![oops](https://github.com/LiamOSM/ChristmasTreePCB/blob/master/PCB%20%26%20Circuit%20Stuff/Renderings/top.svg "PCB Layout")
![oops](https://github.com/LiamOSM/ChristmasTreePCB/blob/master/PCB%20%26%20Circuit%20Stuff/Renderings/bottom.svg "PCB Layout")
