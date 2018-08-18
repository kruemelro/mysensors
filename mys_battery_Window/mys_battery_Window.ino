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
 // Default Settings Area ******************************************
#define VERSION "1.12"
#define Sketchname "WindowNode"
 
int sleeptime = 60; // Sleep time in Minutes
static const uint8_t FORCE_UPDATE_N_READSstd = 1; //Should be set to report once in a hour
// ********************************************************

// Enable debug prints to serial monitor
//#define MY_DEBUG
#define MY_BAUD_RATE 9600

// Enable and select radio type attached
#define MY_RADIO_NRF24

#include <SPI.h>
#include <MySensors.h>
//#include <Bounce2.h>  

int BATTERY_SENSE_PIN = A0;
int oldBatteryPcnt = 0;
float _battery_min = 1.8;
float _battery_max = 3.2;
#define WindowPin1 2   // Arduino Digital I/O pin for button/reed switch
#define WindowPin2 3 // Arduino Digital I/O pin for button/reed switch

uint64_t UPDATE_INTERVAL;
uint64_t FORCE_UPDATE_N_READS;
int nNoUpdates=0;


MyMessage msg(1, V_TRIPPED);
MyMessage msg2(2, V_TRIPPED);
MyMessage msgVCC(200, V_VOLTAGE);

void setup()
{
  uint8_t loadState1;
  uint8_t loadState2;
  loadState1 = loadState(1);
  loadState2 = loadState(2);
  if (loadState1 != 2) {
    // Eprom is not set, first Boot!
    Serial.println("First Boot, setting Defaults");
    Serial.print("Store - Sleeptime: ");
    Serial.println(sleeptime);
    saveState(2, sleeptime);
    saveState(1, 2);
    UPDATE_INTERVAL = sleeptime * 60000;
    FORCE_UPDATE_N_READS = FORCE_UPDATE_N_READSstd;
  }
  else {
    // Eprom is set, read it!
    UPDATE_INTERVAL = loadState2 * 60000;
    FORCE_UPDATE_N_READS = FORCE_UPDATE_N_READSstd;
  }

// Setup the buttons
  pinMode(WindowPin1, INPUT);
  pinMode(WindowPin2, INPUT);
// Activate internal pull-ups
  digitalWrite(WindowPin1, HIGH);
  digitalWrite(WindowPin2, HIGH);
  
  // use the 1.1 V internal reference
#if defined(__AVR_ATmega2560__)
  analogReference(INTERNAL1V1);
#else
  analogReference(INTERNAL);
#endif

}

void presentation()
{
    sendSketchInfo(Sketchname, VERSION);
    present(1, S_DOOR, "Window 1");
    sleep(100);
    present(2, S_DOOR, "Window 2");
    sleep(100);
    present(200, S_MULTIMETER, "VCC");
}

void loop()
{
  uint8_t value;
  static uint8_t sentValue=2;
  static uint8_t sentValue2=2;

  // Short delay to allow buttons to properly settle
  sleep(5);

  value = digitalRead(WindowPin1);

  if (value != sentValue) {
    // Value has changed from last transmission, send the updated value
    resend(msg.set(value==HIGH), 5);
    sentValue = value;
  }

  value = digitalRead(WindowPin2);

  if (value != sentValue2) {
    // Value has changed from last transmission, send the updated value
    resend(msg2.set(value==HIGH), 5);
    sentValue2 = value;
  }

  float volt = analogRead(BATTERY_SENSE_PIN) * 0.003363075;
      // calculate the percentage
      int batteryPcnt = ((volt - _battery_min) / (_battery_max - _battery_min)) * 100;
      if (batteryPcnt > 100) batteryPcnt = 100;
      if (batteryPcnt < 0) batteryPcnt = 0;
  
  if (oldBatteryPcnt != batteryPcnt || nNoUpdates == FORCE_UPDATE_N_READS) {
    // Power up radio after sleep
    send(msgVCC.set(volt,2));
    sendBatteryLevel(batteryPcnt);
    oldBatteryPcnt = batteryPcnt;
    nNoUpdates = 0;
  } else {
    nNoUpdates++;
  }
  smartSleep(WindowPin1-2, CHANGE, WindowPin2-2, CHANGE, UPDATE_INTERVAL);
}




void resend(MyMessage &msg, int repeats)
{
  int repeat = 1;
  int repeatdelay = 0;
  boolean sendOK = false;

  while ((sendOK == false) and (repeat < repeats)) {
    if (send(msg)) {
      sendOK = true;
    } else {
      sendOK = false;
      Serial.print("FEHLER ");
      Serial.println(repeat);
      repeatdelay += 250;
    } repeat++; delay(repeatdelay);
  }
}
