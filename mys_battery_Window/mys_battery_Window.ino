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
#define VERSION "1.00"
#define Sketchname "WindowNode"
#define useack "TRUE"
// Default Values Stored in Eprom
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


//Bounce debouncer1 = Bounce();
//Bounce debouncer2 = Bounce();  
//int oldValue1=-1;
//int oldValue2=-1;
uint64_t UPDATE_INTERVAL;
uint64_t FORCE_UPDATE_N_READS;
int nNoUpdates=0;


MyMessage msg(1, V_TRIPPED);
MyMessage msg2(2, V_TRIPPED);
MyMessage msgcfg1(201, V_VAR1);
MyMessage msgcfg2(201, V_VAR2);

void setup()
{
  uint8_t loadState1;
  uint8_t loadState2;
  loadState1 = loadState(1);
  loadState2 = loadState(2);
  if (loadState1 == 255) {
    // Eprom is not set, first Boot!
    Serial.println("REMOTE-CONFIG: First Boot, setting Defaults");
    Serial.print("REMOTE-CONFIG: Store - Sleeptime: ");
    Serial.println(sleeptime);
    saveState(1, sleeptime);
    UPDATE_INTERVAL = sleeptime * 60000;
    Serial.print("REMOTE-CONFIG: Store - Force-Reads: ");
    Serial.println(FORCE_UPDATE_N_READSstd);
    saveState(2, FORCE_UPDATE_N_READSstd);
    FORCE_UPDATE_N_READS = FORCE_UPDATE_N_READSstd;
    send(msgcfg1.set(sleeptime, 1), useack);
    send(msgcfg2.set(FORCE_UPDATE_N_READSstd, 1), useack);
  }
  else {
    // Eprom is set, read it!
    Serial.print("REMOTE-CONFIG: Sleeptime in Eprom: ");
    Serial.println(loadState1);
    Serial.print("REMOTE-CONFIG: Force-Reads in Eprom: ");
    Serial.println(loadState2);
    UPDATE_INTERVAL = loadState1 * 60000;
    FORCE_UPDATE_N_READS = loadState2;
    send(msgcfg1.set(loadState1, 1), useack);
    send(msgcfg2.set(loadState2, 1), useack);
  }

// Setup the buttons
  pinMode(WindowPin1, INPUT);
  pinMode(WindowPin2, INPUT);
// Activate internal pull-ups
  digitalWrite(WindowPin1, HIGH);
  digitalWrite(WindowPin2, HIGH);
// After setting up the button, setup debouncer
  //debouncer1.attach(WindowPin1);
  //debouncer2.attach(WindowPin2);
  //debouncer1.interval(5);
  //debouncer2.interval(5);
  
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
    present(1, S_DOOR, "Window 1", useack);
    sleep(100);
    present(2, S_DOOR, "Window 2", useack);
    sleep(100);
    present(201, S_CUSTOM, "Remote Config", useack);
}

void loop()
{
//  // Window 1
//  debouncer1.update();
//  // Get the update value
//  int value1 = debouncer1.read();
// 
//  if (value1 != oldValue1) {
//     // Send in the new value
//     send(msg.set(value1==HIGH ? 1 : 0));
//     oldValue1 = value1;
//  }
//  // Window 2
//  debouncer2.update();
//  // Get the update value
//  int value2 = debouncer2.read();
// 
//  if (value2 != oldValue2) {
//     // Send in the new value
//     send(msg2.set(value2==HIGH ? 1 : 0));
//     oldValue2 = value2;
//  }

  uint8_t value;
  static uint8_t sentValue=2;
  static uint8_t sentValue2=2;

  // Short delay to allow buttons to properly settle
  sleep(5);

  value = digitalRead(WindowPin1);

  if (value != sentValue) {
    // Value has changed from last transmission, send the updated value
    send(msg.set(value==HIGH), useack);
    sentValue = value;
  }

  value = digitalRead(WindowPin2);

  if (value != sentValue2) {
    // Value has changed from last transmission, send the updated value
    send(msg2.set(value==HIGH), useack);
    sentValue2 = value;
  }

  float volt = analogRead(BATTERY_SENSE_PIN) * 0.003363075;
      // calculate the percentage
      int batteryPcnt = ((volt - _battery_min) / (_battery_max - _battery_min)) * 100;
      if (batteryPcnt > 100) batteryPcnt = 100;
      if (batteryPcnt < 0) batteryPcnt = 0;
  
  if (oldBatteryPcnt != batteryPcnt || nNoUpdates == FORCE_UPDATE_N_READS) {
    // Power up radio after sleep
    sendBatteryLevel(batteryPcnt, useack);
    oldBatteryPcnt = batteryPcnt;
    nNoUpdates = 0;
  } else {
    nNoUpdates++;
  }
  smartSleep(WindowPin1-2, CHANGE, WindowPin2-2, CHANGE, UPDATE_INTERVAL);
}


// Remote Config Messages
void receive(const MyMessage &message) {
  if (message.sensor == 201 && message.getCommand() == C_SET && message.type == V_VAR1) {
    String inString = message.getString();
    Serial.print("REMOTE-CONFIG: Got new Sleeptime: ");
    Serial.println(inString.toInt());
    saveState(1, inString.toInt());
    sleep(500);
    //Reboot Node to apply new Config
      WDTCSR |= (1<<WDCE) | (1<<WDE);
      WDTCSR= (1<<WDE);
      while(true){}
  }
  if (message.sensor == 201 && message.getCommand() == C_SET && message.type == V_VAR2) {
    String inString = message.getString();
    Serial.print("REMOTE-CONFIG: Got new Force-Reads: ");
    Serial.println(inString.toInt());
    saveState(2, inString.toInt());
    sleep(500);
    //Reboot Node to apply new Config
      WDTCSR |= (1<<WDCE) | (1<<WDE);
      WDTCSR= (1<<WDE);
      while(true){}
  } 
}
