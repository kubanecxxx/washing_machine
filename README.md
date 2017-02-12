# washing_machine
Old automatic washing machine electronic controller


This is and old washing machine renovation. The main reason for this project is the old mechanical programmer got cracked and the price for a new one was almost price of a new washing machine.

Machine itself
----
Washing machine type is <b>Ardo A900</b>. It has basic features like:
- two speed motor with 5uF condenser
- water filling valve
- water drain pump
- water heater
- front door electric lock
- low and high level water level switch
- temperature switch
- few front panel switches (most of them are not used any more)


Hardware part
-----

Schematics and board layout are drawed in Eagle version 6. I used STM32F100C6 micronctroller.

The reason for using triacs instead of relays was switching in zero crossing of sine voltage. I used relays in previous version of this project and I had a lot of problems with this. Relay contacts got welded, switching caused microcontroller unhandled exceptions etc.

With triacs there are no more problems with MCU unhandled exceptions. But the water heater triac dissipates almost 10Watts of heat. It means <b>huge heatsink</b> is needed.

Software part
----

Software is based on ChibiOS real time operating system for microntollers. I made [CMake fork](https://github.com/kubanecxxx/chibios_fork) of the ChibiOS - it is my repositories. You probably need to clone this repository to build this project or you can take the data from the [CMakeLists.txt](fw/CMakeLists.txt) and create a ChibiOS Makefile or just edit the CMakeLists (you will probably need to adjust path to the GCC toolchain and path to the ChibiOS directory) to build this project on you machine.

Development toolchain is GNU arm gcc (https://launchpadlibrarian.net/209776202/gcc-arm-none-eabi-4_9-2015q2-20150609-linux.tar.bz2) version 4.9 - it contains interesting newlib library which saves a lot of RAM and hardware floating point support and many other features.

You can flash the MCU via openocd or my [kstlink](https://github.com/kubanecxxx/kstlink) project which is simple gdb server to debug and flash STM microcontrollers (I feel openocd too heavy).

Firmware is not well documented yet.



