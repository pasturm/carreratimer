# Carrera race track timer

The goal of this project was to build a timer and lap counter for my kid's 
Carrera slot car racing track (Carrera Go!!!, 1:43).

I am using an Arduino Uno and two reed switches as sensors. The reed switches
are small electrical switches which are mounted below the tracks (with tape and 
without the need to modify the tracks). They detect the magnet of the cars when the 
cars pass by.

The lap count, lap record and the current lap time of two tracks are displayed 
on a 16x2 LCD and can be reset with a pushbutton.

The Arduino is powered by a USB battery pack.

## Software
Arduino sketch: [carreratimer.ino](./carreratimer.ino)

## Hardware
* Arduino Uno
* Jumper wires, 220 Ohm resistors, potentiometer, pushbutton (from Arduino Starter Kit)
* Reed switches (Littlefuse MITI-3V1-6-12.5 7mm Ultra-Miniature Reed Switch)
* External USB battery

<img src="figures/breadboard.png" width="75%" />

<img src="figures/carrera.jpg" width="75%" />
