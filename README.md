# **GPS/GPRS Vehicle Tracker**

# **Dedication**

  I dedicate this project to God Almighty my creator. I also dedicate this work to my two families who have offered me enormous encouragement and support, the first is my parents my sister my brother and my friend Ahmed Bangoura who is my second brother. My second family is the students of ENSIAS school who commit themselves to hard work and to making the school better with their sense of help and sharing.
  
# **Final Result**

![System Design Architecture](img/Hardware_Realization.jpg)

# **Table of Content**

- [**GPS/GPRS Vehicle Tracker**](#gpsgprs-vehicle-tracker)
- [**Dedication**](#dedication)
- [**Final Result**](#final-result)
- [**Table of Content**](#table-of-content)
- [**Source Files**](#source-files)
- [**Description**](#description)
- [**Overview**](#overview)
- [**Hardware**](#hardware)
  - [GPS Module](#gps-module)
  - [GSM/GPRS Module](#gsmgprs-module)
  - [Reduce Power](#reduce-power)
  - [Solar Panel](#solar-panel)
  - [Bill of Materials](#bill-of-materials)
  - [More Materials](#more-materials)
  - [Circuit Design](#circuit-design)
  - [Measured Current](#measured-current)
- [**Software**](#software)
  - [Firmware Part](#firmware-part)
  - [BN-220 GPS](#bn-220-gps)
  - [A9 GSM/GPRS](#a9-gsmgprs)
  - [Execution flow diagram](#execution-flow-diagram)
  - [Web Part](#web-part)
  - [Screen Shot of the Web Application](#screen-shot-of-the-web-application)
- [**Experimental Results**](#experimental-results)


# **Source Files**
    .
    ├── driver              # Driver for peripherals
    |   ├── uart.cpp            # Driver for the UART protocol of ATMEGA328
    |   └── softuart.cpp        # Driver for GPIO and interrupt to simulate the protocol UART
    ├── img                 # README files (images) 
    ├── lib                 # Libraries for modules and functionalities of microcontroller
    |   ├── GPRS.cpp            # Lib for A9 GSM/GPRS module
    |   ├── UBXGPS.cpp          # Lib for parsing UBX messages given by the Ublox GPS
    |   ├── Ublox.cpp           # Lib for the protocol UBX to communicate with the Ublox device
    |   ├── ErrorHandler.cpp    # 
    |   ├── Timer.h             # Lib for timer counter
    |   ├── Power.h             # Lib for power management of ATMEGA328
    |   └── Sleep.h             # Lib to control sleep modes of ATMEGA328 
    ├── webApp              # Web application source files
    |   ├── track.db            # Database file
    |   ├── home.html           # Home web page
    |   └── app.js              # Server app
    ├── main.cpp            # Main program
    ├── LICENSE
    └── README.md    

# **Description**

The purpose of this personal project is to design and realize a system that would be implemented in bikes and vehicles in order to keep track of them. Thus it is also designed to be build-able for small cost and be small as well. It is based on ATMEGA 328P (Arduino Nano), GPS module (BN-220) and GSM/GPRS module (Ai-Thinker A9).

The main challenges I am endeavouring to overcome in are:
- How small can I make my prototype hardware ?
- How small can I make my software ?
- How long can my system stay alive in self-powered mode ?

The idea behind how the system works will be described in details in the [**Overview**](#overview) section.


# **Overview**

As shown in the following image, the location is acquired from satellite using GPS receiver, then it will be preprocessed with Arduino Nano microcontroller which after that will send the location coordinates to the server through GPRS module that will contain a valid SIM card. The request goes through internet to the server side where the website and database are located. The data acquired will be saved in database and displayed on the user's browser as a marker on the map.

![System Design Architecture](img/System_Design_Architecture.jpg)


# **Hardware**

The microcontroller I choose to use for my first prototype was the arduino Nano, its specification satisfy my need for this project. It can operate at a frequency up tp 16Mhz, it can go into different sleep modes, it has enough GPIOS and UART peripheral which are the only ones needed for this application.


## GPS Module

The choice of this module depended on how much space and power it takes, its price and also for its availability on the AliExpress market. Thus, I opted for the BN-220 module that weighs 5.3g, with a dimension of 22mm*20mm\*6mm and with current consumption of : 50mA (Active mode) - 7.5mA (standby mode) . This module is based on the ublox-M8030 chip, it contains a Flash memory that allows to save configuration and has a UART interface.


## GSM/GPRS Module

I chose the A9 module from AI-Thinker because it's lightweight, offers several functions and it is a low power module. It has an SD-Card and SIM-Card interface as well as a UART interface for communication with microcontrollers. The power consumption is lesser than 10mAh, also it can be powered with 3.7V Li-ion battery. Its size: 40mm*55mm.


## Reduce Power

Working on it for the next verion when the system will be independent from the car usb power.


## Solar Panel

Working on it for the next version.


## Bill of Materials

All materiels discussed were bought from AliExpress and here is the bill:

Material | Price
---------|------
Arduino Nano | 3.4$
BN-220 GPS | 9.8$
Ai-Thinker A9 GPRS/GSM | 11$
**Total** | 24.2$


## More Materials

I used other materials that each maker should have. 

[USB Analyzer](https://a.aliexpress.com/_mqt8Z6L) helps me to visualize the low level details of signals used in protocols communication and it saved me lot of time in debugging.

I also used [USB power meter](https://a.aliexpress.com/_mN3DxiF) to keep tracking of current consumption when I switch modules and microcontroller from their active mode to standby mode. It helps me to try different techniques to reduce power consumption and to choose the best solar panel that will power my whole system.

So I used [FTDI chip](https://a.aliexpress.com/_m0YMKaJ) that convert USB protocol to UART protocol to test each module directly, without using arduino which is tired work, since they have UART interface.


## Circuit Design

The image below shows how wiring between different parts of system was done:

![Schematic Design](img/Schematic_Design_Minimalist_Version.jpg)


## Measured Current

In progress.


# **Software**

The software for this project can be split up into two parts: **Firmware and Web** Application. **The firmware** objectif is to provide the location of the vehicle to the app server. Then at the server side, new location of vehicle will be stored in local database.

When client is connected, the server serves it the HTML webpage containing a map showing the tracking of the user vehicle.

Currently the website is hosted at : https://karim-gps.glitch.me/


## Firmware Part

The firmware is not based on arduino Framework, instead it uses pure C and C++ so as to take less memory and can be uploaded even to smaller chips like ATTINY85.

I realized this project for the simple reason of deepening my knowledge in the field of microcontroller and that's why I decided to write all my own libraries and drivers. This helps me to free up more space.

In the following sections I describe how each module works and how to establish communication with them.


## BN-220 GPS

When the GPS is on, the GPS satellites broadcast the signals and GPS receivers use the signals and some calculation to provide periodically some informations like latitude, longitude and altitude in format of NMEA or UBX protocol which is more compact. 

The types of information generated depend on what sentence of protocol NMEA or UBX is chosen. So as to configure the GPS module either use the u-Center software which has GUI and easy to use (this software is compatible with just ublox chip) or by sending to the RX pin of GPS module the commands which can be found in ublox chip datasheet.

I enabled these UBX messages: 
- **NAV_POSSLH** :
  - **size =** 28 Bytes
  - **contents =** latitude, longitude ....
- **NAV_STATUS** : 
  - **size =**  16 Bytes 
  - **contents =** gpsFix, ....

The communication is done through the UART interface and when data is received, the microcontroller parses it, checks it validity and extracts from it the information needed.


## A9 GSM/GPRS

After that microcontroller has gathered necesseraly data, it creates an HTTP request and sends it by using the A9 GPS/GPRS module which is a modem that uses GSM AT-commands to communicate with the microcontroller. Here you can find reference to differents AT-commands supported by this module : [link here](https://docs.ai-thinker.com/_media/b_and_t/nb-iot/n92/kaifazhidaowendang/rda8908a_at_commandmanual9.0.pdf)

The communication is also done through UART interface.

    Note: It's better to buy A9G module which is an upgraded version of the A9 with additional features like GPS and get rid of BN-220 gps module that I'm using. This will reduce more energy consumption. 


## Execution flow diagram

The system execution will follow the steps described in this diagram :

<p align="center">
    <image src="img/Firmware_Flowchart_Diagram.jpg">
</p>


## Web Part

For the development of the web plateform, I used the following languages, library and technology : Javascript, NodeJS, HTML, CSS, Leaflet. 

The server processes the post request received by the tracking system, it retrieves the position then stores it in the local database which is controled by the NeDB API. The data is requested periodically through the user web page using HTTP protocol, then the data is returned in JSON format and mapped into Leaflet map.

The following image clearly shows the web architecture :

![client_server](img/Client_server.png)


## Screen Shot of the Web Application

![client_server](img/WebPage_Screenshot.png)


# **Experimental Results**

In progress.