# SDR-Plugin_SDRuno_TXlink_WSJT-X
Transmitter project with SDRuno plugin driving an Arduino DDS transmitter and with WSJT-X integration.

Latest version with Arduino module supporting CW, FT4, FT8 and RTTY. FT4 & FT8 now include gaussian tone transitions
and advanced handling of late TX initiation to maximise receiver decode success in this case.

TXlink4_Nano.ino is the top level Arduino code module.

The new RTTY functionality includes a new VB6 app that integrates the popular MMTTY decoder (see XMMT master folder)

The SDRuno plugin is based on the SDRuno plugin template. The plugin provides a transmitter UI for the Arduino
sending USB serial commands to it and accepting messages from it for display and SDRuno control. It supports
split frequency operation and uses two SDR VRX to control TX/RX frequencies.

The WSJT-X UDP server within the plugin reads the wsjt-x datagrams and parses them for FT8 messages and control.
It also uses the wsjtx ft8code.exe utility to encode the FT8 symbol lists that are sent to the Arduino.

(see https://github.com/Rich5252/WSJT-X_UDP_parser for a simple standalone version)

The Arduino code is a normal sketch and requires a couple of Arduino libraries installed that are identified in the code. 

The main project is for VS2026 and includes the solution files required for the TXlink plugin and also nana window lib.
NOTE:: SDRuno plugin requires nana GUI support which needs to be rebuilt to create the support libs - see
        nana folder for required VS solution file. Also see https://github.com/SDRplay/plugins

73, G4AHN
