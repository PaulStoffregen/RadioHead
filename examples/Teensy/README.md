Teensy 3.x and T4.x Specific Examples for Certain Radios

According to libraries for the RF69, RF95, RF24, RF22, MRF89 and the CC110 radios you have to pass interruptnumber-to-interruptpin mapping as opposed to just the pin number.  This is accoplished by passing digitalPinToInterrupt(n) where "n" can be 0, 1, or 2 for the allowable interrupt pins.

As a sample this was done for RF69 examples and tested on a T3.2 and a T4.
