/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * DESCRIPTION
 *
 * This is an example that demonstrates how to report the battery level for a sensor
 * Instructions for measuring battery capacity on A0 are available here:
 * http://www.mysensors.org/build/battery
 *
 */



// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

// OTA Support
#define MY_OTA_FIRMWARE_FEATURE

// Repeater Function
#define MY_REPEATER_FEATURE

#include <SPI.h>
#include <MySensors.h>  
#include <Vcc.h>


const float VccExpected   = 3.0;
const float VccCorrection = 2.860/2.92;  // Measured Vcc by multimeter divided by reported Vcc
Vcc vcc(VccCorrection);
int oldBatteryPcnt = 0;

static const uint8_t FORCE_UPDATE_N_READS = 10;


void setup()
{
	// use the 1.1 V internal reference
#if defined(__AVR_ATmega2560__)
	analogReference(INTERNAL1V1);
#else
	analogReference(INTERNAL);
#endif

}

void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("Repeater Node", "1.0");
}

void loop()
{


  int batteryPcnt = (int)vcc.Read_Perc(VccExpected);
	if (oldBatteryPcnt != batteryPcnt) {
		// Power up radio after sleep
		sendBatteryLevel(batteryPcnt);
		oldBatteryPcnt = batteryPcnt;
	}
}
