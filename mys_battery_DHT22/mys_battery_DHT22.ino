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

// Default Values Stored in Eprom
int sleeptime = 5; // Sleep time in Minutes
static const uint8_t FORCE_UPDATE_N_READSstd = 12; //Should be set to report once in a hour
// ********************************************************

// Enable debug prints to serial monitor
//#define MY_DEBUG
#define MY_BAUD_RATE 9600

// Enable and select radio type attached
#define MY_RADIO_NRF24

#include <SPI.h>
#include <MySensors.h>  
#include <DHT.h>

int BATTERY_SENSE_PIN = A0;
int oldBatteryPcnt = 0;
float _battery_min = 1.8;
float _battery_max = 3.2;

#define DHT_DATA_PIN 3

float lastTemp;
float lastHum;

uint8_t nNoUpdatesTemp;
uint8_t nNoUpdatesHum;
uint64_t UPDATE_INTERVAL;
uint64_t FORCE_UPDATE_N_READS;


DHT dht;

MyMessage msgTemp(1, V_TEMP);
MyMessage msgHum(2, V_HUM);
MyMessage msgcfg1(201, V_VAR1);
MyMessage msgcfg2(201, V_VAR2);



void setup()
{
  dht.setup(DHT_DATA_PIN);
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
    send(msgcfg1.set(sleeptime, 1));
    send(msgcfg2.set(FORCE_UPDATE_N_READSstd, 1));
  }
  else {
    // Eprom is set, read it!
    Serial.print("REMOTE-CONFIG: Sleeptime in Eprom: ");
    Serial.println(loadState1);
    Serial.print("REMOTE-CONFIG: Force-Reads in Eprom: ");
    Serial.println(loadState2);
    UPDATE_INTERVAL = loadState1 * 60000;
    FORCE_UPDATE_N_READS = loadState2;
    send(msgcfg1.set(loadState1, 1));
    send(msgcfg2.set(loadState2, 1));
  }


  // use the 1.1 V internal reference
#if defined(__AVR_ATmega2560__)
  analogReference(INTERNAL1V1);
#else
  analogReference(INTERNAL);
#endif
}

void presentation()
{
    sendSketchInfo("TempNode-DHT22", VERSION);
    present(1, S_TEMP);
    sleep(100);
    present(2, S_HUM);
    sleep(100);
    present(201, S_CUSTOM);
   
}

void loop()
{
dht.readSensor(true);
  
float temperature = dht.getTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed reading temperature from DHT!");
  } else if (temperature != lastTemp || nNoUpdatesTemp == FORCE_UPDATE_N_READS) {
    // Only send temperature if it changed since the last measurement or if we didn't send an update for n times
    lastTemp = temperature;
    // Reset no updates counter
    nNoUpdatesTemp = 0;
    send(msgTemp.set(temperature, 1));
    
  #ifdef MY_DEBUG
    Serial.print("T: ");
    Serial.println(temperature);
    #endif
  } else {
    // Increase no update counter if the temperature stayed the same
    nNoUpdatesTemp++;
  }

  // Get humidity from DHT library
  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
    Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum || nNoUpdatesHum == FORCE_UPDATE_N_READS) {
    // Only send humidity if it changed since the last measurement or if we didn't send an update for n times
    lastHum = humidity;
    // Reset no updates counter
    nNoUpdatesHum = 0;
    send(msgHum.set(humidity, 1));
    
    #ifdef MY_DEBUG
    Serial.print("H: ");
    Serial.println(humidity);
    #endif
  } else {
    // Increase no update counter if the humidity stayed the same
    nNoUpdatesHum++;
  }

  // get the battery Voltage
  //int sensorValue = analogRead(BATTERY_SENSE_PIN);
  
  //int batteryPcnt = sensorValue / 10;
  //float batteryV  = sensorValue * 0.003363075;
  float volt = analogRead(BATTERY_SENSE_PIN) * 0.003363075;
      // calculate the percentage
      int batteryPcnt = ((volt - _battery_min) / (_battery_max - _battery_min)) * 100;
      if (batteryPcnt > 100) batteryPcnt = 100;
      if (batteryPcnt < 0) batteryPcnt = 0;
  
  if (oldBatteryPcnt != batteryPcnt || nNoUpdatesHum == FORCE_UPDATE_N_READS) {
    // Power up radio after sleep
    sendBatteryLevel(batteryPcnt);
    oldBatteryPcnt = batteryPcnt;
  }
  smartSleep(UPDATE_INTERVAL);
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
