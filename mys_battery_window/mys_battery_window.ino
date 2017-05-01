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
 // User Settings Area ******************************************
#define SKETCHNAME "Window"
#define VERSION "1.00"
// Sleep time in Minutes
int sleeptime = 15; 
// ********************************************************


// Enable debug prints to serial monitor
// #define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#include <SPI.h>
#include <MySensors.h>  

static const uint64_t UPDATE_INTERVAL = sleeptime * 60000;
#define PRIMARY_BUTTON_PIN 2   // Arduino Digital I/O pin for button/reed switch
#define SECONDARY_BUTTON_PIN 3 // Arduino Digital I/O pin for button/reed switch

int BATTERY_SENSE_PIN = A0;  // select the input pin for the battery sense point
int oldBatteryPcnt = 0;

static const uint8_t FORCE_UPDATE_N_READS = 4;

#define CHILD_ID_Window 20
#define CHILD_ID_Window2 21
#define CHILD_ID_ST 99

float lastWindow;
uint8_t nNoUpdates;

MyMessage msg(CHILD_ID_Window, V_TRIPPED);
MyMessage msg2(CHILD_ID_Window2, V_TRIPPED);
MyMessage msgSleeptime(CHILD_ID_ST, V_CUSTOM);

void setup()
{
  // use the 1.1 V internal reference
#if defined(__AVR_ATmega2560__)
  analogReference(INTERNAL1V1);
#else
  analogReference(INTERNAL);
#endif

// Setup the buttons
  pinMode(PRIMARY_BUTTON_PIN, INPUT);
  pinMode(SECONDARY_BUTTON_PIN, INPUT);
// Activate internal pull-ups
  digitalWrite(PRIMARY_BUTTON_PIN, HIGH);
  digitalWrite(SECONDARY_BUTTON_PIN, HIGH);
// send Sleeptime
  send(msgSleeptime.set(sleeptime, 1));
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCHNAME, VERSION);

  present(CHILD_ID_Window, S_DOOR);
  sleep(100);
  present(CHILD_ID_Window2, S_DOOR);
  sleep(100);
  present(CHILD_ID_ST, S_CUSTOM);
}

void loop()
{
  uint8_t value;
  static uint8_t sentValue=2;
  static uint8_t sentValue2=2;

  // Short delay to allow buttons to properly settle
  sleep(5);

  value = digitalRead(PRIMARY_BUTTON_PIN);

  if (value != sentValue) {
    // Value has changed from last transmission, send the updated value
    send(msg.set(value==HIGH));
    sentValue = value;
  }

  value = digitalRead(SECONDARY_BUTTON_PIN);

  if (value != sentValue2) {
    // Value has changed from last transmission, send the updated value
    send(msg2.set(value==HIGH));
    sentValue2 = value;
  }
  // get the battery Voltage
  int sensorValue = analogRead(BATTERY_SENSE_PIN);
  
  int batteryPcnt = sensorValue / 10;
  float batteryV  = sensorValue * 0.003363075;
  
  if (oldBatteryPcnt != batteryPcnt || nNoUpdates == FORCE_UPDATE_N_READS) {
    // Power up radio after sleep
    sendBatteryLevel(batteryPcnt);
    oldBatteryPcnt = batteryPcnt;
    nNoUpdates = 0;
  } else {
    // Increase no update counter if the temperature stayed the same
    nNoUpdates++;
  }
  smartSleep(PRIMARY_BUTTON_PIN-2, CHANGE, SECONDARY_BUTTON_PIN-2, CHANGE, UPDATE_INTERVAL);
}
