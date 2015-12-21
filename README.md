# TedPi #

This repository contains bare metal code for Raspberry Pi. 
It is intended for people who want to write their own code for RPi from scratch.
This is purely for educational purposes and it might contain bugs (most likely), so run at your own risk.

### Current version 0.0.1 (alpha) ###

* GPIO, LED, IRQ, UART, I2C and systimer drivers
* contains driver for [DS1337](http://datasheets.maximintegrated.com/en/ds/DS1337-DS1337C.pdf) real-time clock
* does not use standard libraries (e.g., newlib)
* command-line interface (CLI)

### What's next ###

* complete IRQ driver
* add interrupt support to other drivers
* add GUI support (mailbox interface)
* add MMU driver (virtual memory)
* define processes (1 core only)
* ???

### Contributors ###

* [matallui@gmail.com](mailto:matallui@gmail.com)
