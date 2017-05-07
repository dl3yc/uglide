# uGlide miniature glider payload repository

## Abstract 
This repository contains software and hardware design files for uGlide - the payload of a small high altitude glider. This is a fork of [uTrak](http://www.github.com/thasti/utrak).

The mission objective is to evaluate the flight dynamics of a miniature glider. The payload transmits telemetry. Currently only GPS position an height are transmitted.

![uTrak PCB v1](pic/utrak-pcb-v1.jpg)

The following tasks shall be performed by the software:
* Filtering and reading GPGGA-sentences from GPS - extracting position, altitude
* Measuring supply voltage with internal ADC
* Reading die temperature with internal ADC
* Generating telemetry frames (Including position, altitude, supply voltage, temperature)
* Configuring the Si4060 transmitter IC
* Power management for MCU and external hardware
* Transmission of APRS position reports

## GPS input
The MSP430 USART is used to store one NMEA-sentence to RAM (from $ to newline). Filtering and processing is controlled by a simple finite state machine. Fix data (position and altitude) are extracted and converted to HAB telemetry compatible format and APRS frames.

## Telemetry generation
From valid GPS data and telemetry data, packets are generated. The text string is formatted to be sent in 7 bit ASCII over RTTY. APRS transmission is done on EU APRS frequency (144.800MHz) and the telemetry frequency.

## Power management
Focus of the software is to get power consumption down as low as possible. This includes using all the available power saving features of the MSP430 and peripherals:
* Low power modes of the MSP430
* Disabling unused peripherals
* Power down mode of the GPS module
* Disabling RF stage

# Hardware modifications
After v1, several modifications were made to the PCB to enable more features being possible. These include:

## TCXO as reference source
Frequency stability was totally unacceptable with crystals. In addition, the datasheet wrongly states that a TCXO has to be connected to XIN, while it has to be connected to XOUT.
Needed after commit: edc32003
- Instead of the XTAL, solder TCXO with clipped sine output, 16.3676Mhz.
- cut trace from C8 to XIN
- solder C=100pF instead as C8
- run wire from right pad of C8 to XOUT-pad (right pad of R5)
- solder bridge over R6
- do not place R4, R5 and R7

## TCXO source as clock input to MSP430
As DominoEX needs to me modulated coherently, MCU and PLL need to run off the same clock. Therefore, the Si4060 will buffer the TXCO signal and output it via GPIO to the MCU.
Needed after commit: ee2a82f8 until 752375ca
- solder wire from MSP430 QFN PAD 21 (PJ.4, XIN) to Si4060 QFN PAD 9 (GPIO0)

# Coarse Changelog
This changelog tries to summarize the development of the tracker firmware. See the git commit log for further information.

- 05-2014: Begin of development
- 09-2014: first test flight (RTTY 50 baud)
- 09-2014: change from the Linx-GPS to the uBlox M8Q module
- 10-2014: first flight with professional PCB & uBlox (0x01, 0x02)
- 11-2014: first flight with TCXO (0x03) - frequency stability improvement
- 10-2014: implementation of APRS transmission
- 11-2014: implementation of DominoEX16
- 11-2014: first flight with APRS & DominoEX16 (0x04)
- 12-2014: reverting to RTTY, now using 100 baud (0x05, 0x06)
- 07-2014: first good use of APRS: antenna now 2m dipole
- 07-2014: dropped support for DominoEX
- 07-2014: transmitting APRS only above a certain altitude (power savings)

