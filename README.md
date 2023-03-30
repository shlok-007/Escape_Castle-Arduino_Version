# IITBBS_GC_RS_6
ArduGem Submission for GC'23

Escape Castle Game

Description:
Tricky Castle is a simple 2D platformer game that uses an accelerometer to control the player character. The objective of the game is to reach the door at the end of each level while avoiding obstacles and enemies. The player can jump by shaking the accelerometer, and the longer they shake it, the higher the jump.

Dependencies:

SPI.h
Adafruit_GFX.h
TFT_ILI9163C.h
Wire.h

Installation:

Clone or download the code from the repository.
Upload the code to an Arduino board.
Connect the accelerometer to the board according to its pinout.
Connect the board to a TFT display.
Calibrate the accelerometer by running the code and following the instructions on the display.
Usage:

Once the accelerometer is calibrated, the game will start automatically.
The player can move left or right by tilting the accelerometer in the desired direction.
The player can jump by shaking the accelerometer.
The longer the accelerometer is shaken, the higher the player will jump.
The player must reach the door at the end of each level to progress to the next level.
The player must avoid obstacles and enemies.
The game can be restarted at any time by pressing the reset button on the board.
