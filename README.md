#Rotating Display
**Abstract:** 
 The Rotating Display is a compact disc-sized device that rotates quietly using a CD motor. It features 40 LEDs that display time and weather data sourced from the internet. The device is wirelessly powered and controlled via a user-friendly web interface. It uses Arduino nano and ESP-01s microcontrollers and is easy to assemble.
 
<p align="center"> 
  <img src="images/figure00.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**1. Description of the device**

The Rotating Display device comprises two primary units: a power supply unit and a display board (**figure 1**). Both are circular in design, with a diameter of 120mm, similar to the dimensions of a standard Compact Disk. The display board is rotated by a CD motor. Energy is wirelessly transmitted from the power supply unit to the display board, eliminating the need for wired connections.

<p align="center"> 
  <img src="images/figure01.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>


**Figure 1:  Rotating Display assembly.** The lower PCB is the power supply with wireless power transmission, the upper PCB is the display board.

The display board is equipped with two rows of LEDs, each containing 20 LEDs. This makes a total of 40 LEDs available for image representation. The LED operations are controlled by an Arduino Nano, while an ESP-01s microcontroller generates the display content. The ESP-01s maintains a Wi-Fi connection to the internet for this purpose.

This internet connection allows the device to retrieve the time from a time server, ensuring time accuracy. It also allows for the acquisition of weather data. Device operation is managed through a web interface (**figure 2**), accessible from any web browser. The interface allows users to manage login credentials, upload image files to the display, and control image and configuration files through a file manager.

<p align="center"> 
  <img src="images/figure02a.jpg" style="display: inline-block; margin: 20px; max-width: 500px">
  <img src="images/figure02b.jpg" style="display: inline-block; margin: 20px; max-width: 500px">
  <img src="images/figure02c.jpg" style="display: inline-block; margin: 20px; max-width: 500px">
</p>

**Figure 2:** The **web user interface** allows for operating mode selection and device configuration.

**2. Operating modes**

When switched on, the device tries to connect to a known wifi. If no valid wifi credentials are found, the unit will be configured as wifi access point. In this mode, a computer or mobile device can be connected, directly (SSID: RD40, no password). 

Once the wifi connection has been established, the unit will display its IP address on the  home page of the device (**figure 3**). Enter this address in your web browser. This will load the web user interface and will allow you to configure your local wifi.

<p align="center"> 
  <img src="images/figure03.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 3: home page.**

There are several operating modes that display time and weather information (**figure 4**). The logo clock mode integrates a customizable image into the clock face. The image can be easily uploaded from the user interface. The same is true for the analog clock, which uses a customizable clock face in the background.

<p align="center"> 
  <img src="images/figure04.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>
**Figure 4: operating modes.** Modes can be selected and configured through the web interface.

**3. Mechanical design**

The device consists of two assemblies: power supply and display. The power supply board is also the base plate, which can either be placed on a flat surface or hung on a wall. A standard CD motor is inserted through a recess in this board so that the CD tray above the board can accommodate the display assembly. The display board is fixed with two M2 screws. Furthermore, the base plate carries a potentiometer to control the motor speed as well as an on/off switch for the motor.

<p align="center"> 
  <img src="images/figure05.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 5: Power Supply Board** with CD motor (middle), 12V power jack (lower side), and speed potentiometer

An important requirement for the display board is that the center of mass of the unit is exactly in the middle, on the axis of rotation of the motor. This is the only way to ensure smooth and vibration-free running of the display. To achieve this, the electronic components are arranged as symmetrically as possible to the vertical axis of symmetry in **figure 7**. This initially ensures that the center of gravity lies on this vertical axis. However, since the components cannot be distributed symmetrically to the horizontal axis of symmetry, the center of gravity must be shifted to the center along this axis using balancing weights. For this purpose, two M2x6mm screws with two nuts each are placed to the right and left of the Arduino nano. The balancing result is very good, but can certainly be further optimized.

<p align="center"> 
  <img src="images/figure06.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 6: Display Board.** Main components are the two LED rows, five serial shift register chips, and two microcontrollers (Arduino nano and ESP-01s). On the backside of the board the secondary coil of the wireless power supply is attached.

<p align="center"> 
  <img src="images/figure07.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 7:** The components of the display board are arranged symmetrically to the vertical axis of symmetry. Therefore, the center of gravity is located on this axis. The vertical position of the center of gravity is adjusted to the middle by means of adjustment weights.

The two LED rows each consist of 20 discrete, rectangular LEDs. The components each have a width of 2mm and can be lined up without spacing. However, the light emitting surface of the LEDs has a diameter of only about 1mm. Thus, there is a 1mm gap between the light points. To fill these gaps, the second row of LEDs is used. It is inclined by 90 degrees along the angle of rotation. Radially, the two LED rows are shifted by 1 mm, creating a dot pattern without gaps (**figure 8**).

<p align="center"> 
  <img src="images/figure08.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 8:** The two LED rows are displaced radially by half a LED width

The LEDs must output the image information at exactly the right position with each rotation. For spatial alignment of the image, a trigger signal is generated at a defined position for each rotation, which triggers the sequential, clocked output of the pixels. The trigger signal must be spatially very stable, otherwise no smooth image can be generated. It turned out that a Hall sensor is best suited for this purpose. It is located underneath the Arduino nano (see **figure 6**, right picture, bottom edge of the PCB). The magnet is glued into a hole of the power supply PCB (see figure 5, left picture, 4 o'clock position).

**4. Electronics architecture**

The distribution of the electronic circuitry between the two assemblies is shown in **figure 9**. The power supply assembly contains a Royer converter for wireless power transfer to the display assembly and an adjustment of the motor speed. The 40 LEDs of the display assembly are controlled by 5 shift registers (8 bits each), which in turn are supplied with clock and data by an Arduino nano via an SPI interface. The displayed content is generated by an ESP-01s microcontroller, which is connected to the internet via Wifi. It transmits its data to the Arduino nano via an I2C interface.

<p align="center"> 
  <img src="images/figure09.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 9: electronics schematics.**

**5. Power supply board**

The core of the power supply is a Royer converter for wireless power transfer. An excellent article on the operation and design of the circuit can be found at Mikrocontroller.net (German) and the circuit was taken from there. Two transistors are alternately switched (push pull operation) so that a current flows through one half of the coil at a time. The coil belongs to a resonant circuit with a resonant frequency of about 120 kHz. The control voltage for the transistors is obtained via a coupling coil (**figure 10**). The secondary coil is located below the display board (see **figure 6**, right).

The Royer circuit uses very few components. However, the coil is quite complex. It is a bifilar coil, where the two halves are interleaved. In addition, the coupling coil must be connected with the correct polarity, otherwise the two transistors will be destroyed. In the early stages of development, the coil was wound with copper wire. However, this solution was quite difficult to reproduce, so in the final design the coils (bifilar primary coil, coupling coil, secondary coil) were implemented as a printed circuit (see **figure 5**, left and **figure 6** right). The circuit was found to have surprisingly high and perfectly adequate transmission efficiency, although the quality factor of the printed coils is inevitably compromised. The Royer converter with the printed coil assembly is absolutely safe to rebuild.

<p align="center"> 
  <img src="images/figure10.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 10: Royer converter**. Compare Mikrocontroller.net for reference.


There is also a circuit for supplying the CD motor on the power supply board (**figure 11**). An LM317 voltage regulator is used to generate a variable voltage between 1.7V and 6.0V, which can be adjusted via a potentiometer. The voltage range corresponds to the specified operating range of the CD motor. The supply of the motor can be interrupted with a switch, for example to allow programming of the microcontrollers.

<p align="center"> 
  <img src="images/figure11.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 11: variable voltage regulator** for driving the CD motor

**6. Display Board**

**a) The LEDs**

The 40 LEDs of the display board are driven by 5 cascaded 8-bit shift registers (**figure 12**). The registers load the serial data stream of the SPI interface of the Arduino nano, synchronized with the SPI clock (SRCK), and switch the parallel pattern to the LEDs when a rising edge is sent to the register clock (RCK). The TIPC6C595 shift register has open drain DMOS outputs. The LED current is set to 20mA each. The maximum current per register is therefore 160mA, which is well within specification. The brightness of the LEDs can be controlled by their duty cycle. The output enable input (G) of the register is used for this purpose. Only when this input is low, the DMOS outputs of the register are active.

The SPI interface is operated with a clock rate of 16 MHz. The 40 bits are therefore transferred in 40 x 1/16MHz = 2.5µs. Assuming a maximum rotation speed of the display of 2000 RPM, this corresponds to 30ms per revolution. At one full rotation of the display 240 pixels are output per LED. This corresponds to 125µs per pixel. Thus, the transmission time via the SPI bus (2.5µs) is significantly shorter than the display time per pixel.

<p align="center"> 
  <img src="images/figure12.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 12: circuit diagram of the display board**. Diagram also available as download and on Github.

**b) Arduino interrupts and timing**

The idea of using two microcontrollers (Arduino nano and ESP-01s) is essentially to separate the very time-critical timing of the LED control from the computationally intensive and asynchronous image generation and internet communication. In this way, the Arduino is relieved to the maximum and the display of the image is extremely stable.

The software of the Arduino programmed in C consists of two short interrupt routines. The first routine (INT0) is triggered by the Hall sensor exactly once for each full rotation (**figure 13**). This routine starts timer 1, which sets the duration for which a single row of pixels is displayed. Once the timer expires, it is automatically restarted and a second interrupt routine is triggered (INT1). In this second routine the bit pattern of the LEDs is updated. So in this way a new LED pattern appears with each expiration of timer 1. It is important that timer 1 restarts automatically (hardware controlled). Otherwise there could be a delay if the processor cannot start the interrupt routine immediately (because another routine prevents this, for example).

Another important function is to set the duration of timer 1 so that exactly 240 pixel rows per turn are output. For this purpose, the variable "tpt" (time per turn) is increased by the set value of timer 1 with each call of INT1. The routine INT0 can read tpt and thus knows the exact time duration for one revolution - even if it consists of more or less than 240 timer 1 cycles. The new time per pixel (tpt/240) is then calculated from this value and timer 1 is set accordingly.

To control the brightness of the LEDs, the threshold function of timer 1 is used: as soon as a programmable threshold is exceeded, an output of the Arduino switches. The signal is used to switch on the LEDs for the selected time. The principle is similar to the well known pulse width modulation. However, the pulses are synchronized with the pixel clock.

<p align="center"> 
  <img src="images/figure13.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 13: Arduino interrupt timing.** Timer 1 clocks the pixel output and sets the LED brightness.

**c) ESP-01s for Internet communication and image generation**

The microcontroller ESP-01s has a Wifi module for internet communication and just two programmable input/output pins. The module is placed at the edge of the display board, because the interference field of the wireless power supply is lowest there. In previous versions of the board, the module was located in the center of the board, which caused the controller to misbehave in a way that was difficult to interpret, apparently due to an interference from the wireless power supply. This issue is fully resolved in the latest version of the display. 

Although a concern in the beginning of the development, it turned out that the fast rotation of the display does not lead to any noticeable impairment of the Wifi connection.

**d) Programming of the microcontrollers**

The Arduino nano can be programmed via its mini-USB socket. During programming the power supply of the display can be switched off (but does not have to).

Programming the ESP-01 is a bit more complicated, because it does not have an integrated USB level converter. Therefore below the microcontroller there is a 6 pin female connector to which a FT232 level converter can be connected. Note the correct orientation of the FT232! Next to the connector a small picture shows the correct orientation. The microcontoller cannot be powered by the computer during programming. Therefore the power supply of the display must be switched on. Before programming, the jumper must be plugged in position "P". Then the reset button next to the controller is pressed so that it changes to programming mode. Now the upload can be started on the computer. Afterwards the jumper is put back to its original position and the system is restarted.

Please note that the program code and the content of the flash disk (especially the code of the internet pages) have to be loaded separately. A good tutorial on how to load the Flash Disk with Visual Studio Code and platformIO can be found here: randomnerdtutorials.com

**7. The Software of the ESP-01s** 

The ESP-01s handles the generation of the image data and its transmission to the Arduino nano (once per second). The displayed information (time and weather data) is retrieved from the Internet. Furthermore, the microcontroller serves as a simple HTML web server that can be accessed by any browser to control the display.

**a) Generation of the image data**

Even though the resolution of the display achieved with the 40 LEDs is relatively low, it is sufficient to display images and text upright, undistorted and therefore easily readable. The image data is generated in a rectangular pixel matrix (i.e. a cartesian coordinate system) with 110 x 110 pixels (**figure 14**). This bitmap then has to be transformed into the polar coordinate system of the rotating LED rows, before it is transferred to the Arduino. To make the transformation fast, a lookup table is used, which transforms the x,y coordinates of the bitmap into the r, theta coordinates of the rotating LED lines. The transformation also takes into account the alternating order of the LEDs between the two rows and the 90 degree angle between the two rows.

<p align="center"> 
  <img src="images/figure14.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 14:** The display handles a **110x110 pixel bitmap**, which is transformed into the polar coordinate system of the rotating LED rows

**b) Control of the display via web interface**

For the implementation of the web server the library ESPAsyncWebServer is used. It has the advantage that the server activity runs in the background independent of other processes running simultaneously on the controller. The server retrieves the HTML, CSS and JS data of the displayed web pages from the local file system (LittleFS) of the microcontroller. Therefore, the web pages and the server can be implemented independently. Retrieving or modifying data is done using the HTTP GET and POST methods.

Certain parameters (brightness of the LEDs, operating mode of the display) can be set via the web interface. In addition, configuration data, such as the access data to the weather service openweather.org or the Wifi data can be edited and saved. These data are also stored in the file system LittleFS of the ESP-01 and are therefore also available after a reboot.

The LittleFS file system allows files to be organized in a directory structure and stored in the microcontroller's flash disk. The user interface of the device has a file browser with which files can be uploaded from the end device (e.g. mobile device) to the microcontroller or downloaded from the microcontroller. This is especially important to be able to load image data to the device. In addition, files and entire directories can be deleted, renamed and moved. This simplifies the organization of the flash disk considerably.

**c) Software architecture**

The ESP-01 software is written in C++. In order to keep an overview despite the large amount of software, the function blocks and the data have been divided into classes. The following is a brief description of the most important classes:

Class RD40

The class provides the central rotating display object. It manages the (private) data of the displayed image (one bit per LED). The class has methods for displaying bitmaps and for passing data to the display controller.

Class myBMP

This class creates 110x110 bit bitmaps for display. It uses private methods to print text on the bitmap, draw lines and circles, or load images.

Class WebInterface

This class manages the web user interface. Once the webInterface object has been initiated, the web server will run in the background without the need for any attention from the software. The class provides certain attributes, such as clockMode and brightness, which are managed by the web interface. The user can change configuration data via the web interface. The data is then stored as parameter files on the flash disk.

Not being a software engineer myself, the source code would most certainly benefit from a code review and revision.

**d) Development Environment**

Visual Studio Code with the PlatformIO extension was used as the development environment. The environment is much faster and more comfortable than the Arduino IDE. In addition, it can also be used for the development of the source code of the web pages.

**8. Reproduction of the rotating display**

All necessary data, such as source code, circuit diagrams, printed board layouts, parts list and sources of supply are included either in this article or in my public github repository. They may be used for non-commercial purposes, such as by hobbyists or in education, whether for reproduction or further development. Either way, please respect the terms of the license.

The device mainly uses standard components that are easy to obtain. The only exceptions could be the LEDs and the shift register (TIPC6C595, Texas Instruments). The LEDs can be purchased from LED1.de (€ 0,32 per piece, from purchase of 50 pieces). The shift register is available in large quantities from DigiKey at the time of writing. The circuit boards were manufactured at Aisler.net.

In the following some hints for the assembly are given. The assembly is simple, especially since almost only through-hole components are used. Nevertheless, good soldering skills are required. As a beginner project the device is only conditionally suitable.

<p align="center"> 
  <img src="images/figure15.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 15:** Start with the assembly of the power supply board. Insert and solder all electronic components, Afterwards, insert the CD motor from the soldering side and bond it with a fast curing two component epoxy resin.

<p align="center"> 
  <img src="images/figure16.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 16:** Two M2 nuts are glued into the CD tray PCB. It is attached with double-sided adhesive tape to the back side of the CD motor tray. Two M2 screws may help to bring the PCB into the right position.

<p align="center"> 
  <img src="images/figure17.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 17:** The rubber bumpers are glued to the ends of the 20 mm long bolts with the epoxy resin. The magnet is then glued into the hole provided for it. Attention: the correct orientation of the magnet (with the right side up) is very important! Make sure that the device works properly before you glue the magnet!

<p align="center"> 
  <img src="images/figure18.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 18:** Continue with the display board assembly. First prepare the LEDs by shortening the wires. Insert them one row at a time. Hold the LEDs in place with tape before soldering. Note: The LEDs must be inserted in the correct orientation. Use the picture as a reference to identify the cathode and anode of the components. Note the marking on the PCB (A = Anode, K = Cathode).

<p align="center"> 
  <img src="images/figure19a.jpg" style="display: inline-block; margin: 20px; max-width: 305px">
  <img src="images/figure19.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 19:** Note: The Hall sensor is the only component, that is inserted through the holes from the soldering side! It needs to be installed before you insert the Arduino nano, as the Hall sensor is soldered on the component side of the PCB.

<p align="center"> 
  <img src="images/figure20.jpg" style="display: inline-block; margin: 20px; max-width: 600px">
</p>

**Figure 20:** When all components are installed, the secondary coil PCB can be attached using 4 pins.

**9. Conclusions**

The rotating display works according to a simple principle. However, the development of the Rotating Display device presented numerous highly interesting engineering challenges. Finding solutions for this was not only very interesting, but an opportunity to learn a wide variety of technologies. Last but not least, it was an intellectual challenge that I enjoyed a lot.

Worth mentioning are:

Simple design with few mechanical components
Replicable wireless power supply with printed coils
Alternating arrangement of LEDs to achieve higher resolution
Complete balancing of the display board to ensure vibration-free operation
Timer controlled clocking of the LEDs and regulation of their brightness
User-friendly interface via a standard web browser
Retrieval of time and weather data from the internet
Implementation of a file browser for the ESP-01’s flash disk for uploading image data
Transparent, object-oriented structure of the ESP-01s software
