
## INTRODUCTION

The Base Board (PCB) is the main board of the photoreactor, it consists of a module Blue Pill STM32F103CBT6 (due to the absence of STM32 chips) that acts as the master of the micro present in the PCC and the ESP32 present in the PCB itself. The ESP32 is a slave
of the Blue Pill and is responsible for communications with the user via Bluetooth and through a 2.8" TFT screen (ILI9341).


## VERSIONS

## USED ​​VERSIONS

"Motherboard 1.0 9V lr":
Description: 
This plate corresponds to the "rectangular" plate that has the TFT screen connected through the IDC connector and the same for the microSD. It has its own Buck (switched) converter with the TPS54240 IC that converts 12 V to 3.3 V with an efficiency of 90%. All digital electronics are powered by 3.3 V.
The 12V connector also has a reverse polarity protection circuit. Use the TPS7A4901 as a 12V to 9V linear regulator with 75% efficiency. The analog electronics are powered by 9 V. Although it could be powered with 12 V If necessary.
It has two SD cards (SPI), being external 6-pin modules (you have to bypass the linear regulator from 5V to 3.3V).

The PCB_STM code is uploaded with the ST-Link to the Blue Pill and the ESP32 is uploaded with a USB-Serial converter using the buttons on the board. Once the code loading begins Hold down the "Boot" button and press the "Reset" button to load into ESP32.

Failures: The UART connection of the original models is exchanged due to confusion with pin remapping on the STM32F103. In the current EasyEDA project this error has already been corrected.

"Motherboard Only Modules":
Description:
This board corresponds to the square motherboard that has the screen integrated inside from the board itself with a socket, and also has an encoder own soldier on plate. The reason for these two changes is not to use connectors. Added socket for external RTC DS3231.
Its name is due to the fact that only commercial modules are used for power. In this In this case, two MP1584 switched buck converters are used that can be purchased on Amazon or Aliexpress. One of the buck converts 12 V to 9 V and the other from 12 V to 3.3 V.
A 3.3 V microSD module is used for the Blue Pill and the SD socket on the screen for the ESP32.

The code loading is the same as the previous model. 

Failures: The UART connection of the original models is exchanged due to confusion with pin remapping on the STM32F103. In the current EasyEDA project this error has already been corrected.

FUTURE VERSION
"Full Custom Motherboard"
The idea of ​​this board is that it integrates the circuitry of the two boards used so far. Being a completely customized plate it would add greater added value to the product.
- Switched 12V to 3.3V converter with the TPS54240
- 12V to 9V linear regulator with TPS7A4901
- Screen with socket to mount directly to the plate
- Own encoder for soldering on the board
- Two microSD sockets integrated on board
- External soldered RTC integrated on board or soldered.
- STM32F103CBT6 chip as main microcontroller instead of using Blue Pill


## DETAILS OF THE FILES

Within each version we find two folders, Hardware and Software.

## HARDWARE
In the Hardware folder we find the Gerber files for manufacturing the board, the BOM (bill of materials) and pick and place for automated manufacturing. Also included is an .OBJ file that can be exported to Fusion360 
(Due to EasyEDA problems, the size must be reduced so that it is real size).

In the Hardware folder of Only Modules we find two versions, ESP32D and ESP32-U, the difference is that the D has a double nucleus and the U has a single nucleus. The footprints are slightly different,but both should work fine for the firmware used. 
They are two different folders because the footprint. The one and the other are slightly different, and sometimes there is a lack of stock of the 32D.

## SOFTWARE
In the software folder you will find the Arduino sketch for the Blue Pill, called PCB_STM, and the Arduino sketch for the ESP32, called PhotoRX.
PCB_STM --> Blue Pill (STM32F103CBT6 or STM32C8T6)
PhotoRX --> ESP32	

In the PCB_STM sketch, the changed pins have been adapted compared to the original firmware for the boards "Placa Base 1.0 9V lr" and "Placa Base Only Modules" with the following define:
#define PCB1_0
This precompiled define sets the pins for the boards. The definitions of these pins are
starting from line 66 of PCB_STM.h

The PhotoRX sketch integrates bluetooth communication as a HUB with the PCB_STM through UART.
It also integrates TFT screen menu and UART communication, such as RTC external clock.
