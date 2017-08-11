## MySensors
Firmware for Atmega and NRF2401 Sensors based on MySensors Library

## Usage
The Sensors register themselves to the Gateway, so simple Power on and wait some Seconds. Have a look at the readme from the specific Sensor how to link the Sensor Items to the openHAB items.
Some sensor values are only sent on reboot. To get all openHAB items filled reboot the Sensor after you have linked all items. 

## Remote Config
The Sensors are able to get some Config-Parameters from Remote. The Parameters are Sensor specific and defined in the readme from the Sensor. To use openHAB to send the configs to the Sensors link the Var Childs to openHAB items. You also have to set Smartsleep on in the Properties of the Sensor Child 201.
