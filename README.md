# Twiddler

A bit-twiddler binary computer toy.

## Hardware

### Introduction to the TM1638

Check out this excellent intro to the TM1638 chip:
http://tronixstuff.com/2012/03/11/arduino-and-tm1638-led-display-modules/

### Wiring Diagrams

You can find wiring diagrams for both common models, along with info about wiring multiple in parallel here:
https://github.com/zegreatclan/AssettoCorsaTools/wiki/Arduino-Tm1638-Display-Wiring

When wiring your own twiddler you may need to change the pin assignments, at the top of the `src/twiddler.ino` file you'll find a configuration for which pins are attached to data, clock & strope. 

```ardunio
// connect to TM1638 with data pin, clock pin, strobe pin
TM1638 module(5, 6, 7);
```

## Software

## TM1638 device driver

Twiddler uses the device driver written by `rjbatista`, which can be found here:
https://github.com/rjbatista/tm1638-library

Twiddler was originally developed on a `Particle Core v1` chip, so I published the same library on the Particle registry:

```bash
particle library search TM1638
> Found 1 library matching TM1638
TM1638 1.0.0 [mine] 0 A library for interacting an arduino with a TM1638/TM1640
```

## Application code

You can find the source code for the Twiddler application in the `src` directory.