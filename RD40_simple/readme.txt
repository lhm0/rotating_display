RD40_simple is a simple version of the Arduino nano and the ESP01s software. It is meant for people who would like to understand, how the display and its code work.

The functionality has been reduced to a minimum. The display retrieves the time from an NTP server, converts it into local time, and displays it as text on the display. There is no web user interface.

The code does programmed in C and does not use object oriented structures, in order to keep it as simple as possible.

The data tables have been "off loaded" to a separate file "templates.cpp".


NOTE:
In order to log into you local WiFi network, you need to enter your Wifi credentials. Open the folder RD40_simple_ESP8266 in Visual Studio Code and got to lines 65 and 66. Replace 'PUT YOUR SSID HERE' with the name for your network. Replace 'PUT YOUR PASSWORD HERE' with your password.