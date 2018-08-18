/**
   The MySensors Arduino library handles the wireless radio link and protocol
   between your home built sensors/actuators and HA controller of choice.
   The sensors forms a self healing radio network with optional repeaters. Each
   repeater and gateway builds a routing tables in EEPROM which keeps track of the
   network topology allowing messages to be routed to nodes.

   Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
   Copyright (C) 2013-2015 Sensnology AB
   Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors

   Documentation: http://www.mysensors.org
   Support Forum: http://forum.mysensors.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

 *******************************

   DESCRIPTION

   This is an example that demonstrates how to report the battery level for a sensor
   Instructions for measuring battery capacity on A0 are available here:
   http://www.mysensors.org/build/battery

*/
// Default Settings Area ******************************************
#define VERSION "1.12"

// Default Values Stored in Eprom
int sleeptime = 2; // Sleep time in Minutes
// ********************************************************

// Enable debug prints to serial monitor
//#define MY_DEBUG
#define MY_BAUD_RATE 9600

// Enable and select radio type attached
#define MY_RADIO_RF24

#include <SPI.h>
#include <MySensors.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

int BATTERY_SENSE_PIN = A0;
int oldBatteryPcnt = 0;
float _battery_min = 1.8;
float _battery_max = 3.2;

float lastTemp;
float lastHum;
uint8_t nNoUpdatesTemp;
uint8_t nNoUpdatesHum;
uint64_t UPDATE_INTERVAL;
uint64_t FORCE_UPDATE_N_READS;
float dP_dt;

#undef BME280_ADDRESS         // Undef BME280_ADDRESS from the BME280 library to easily override I2C address
#define BME280_ADDRESS (0x76) // Low = 0x76 , High = 0x77 (default on adafruit and sparkfun BME280 modules, default for library)

Adafruit_BME280 bme; // Use I2C

MyMessage msgTemp(1, V_TEMP);
MyMessage msgHum(2, V_HUM);
MyMessage msgVCC(200, V_VOLTAGE);
MyMessage msgcfg1(201, V_VAR1);

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
    UPDATE_INTERVAL = sleeptime * 60000;
    FORCE_UPDATE_N_READS = 30;
    send(msgcfg1.set(sleeptime, 1));
    // Save Eprom State 1 to know its allready configured
    saveState(1, 2);
  }
  else {
    // Eprom is set, read it!
    Serial.print("Sleeptime in Eprom: ");
    Serial.println(loadState2);
    UPDATE_INTERVAL = loadState2 * 60000;
    FORCE_UPDATE_N_READS = 60 / loadState2;
    send(msgcfg1.set(loadState2, 1));
  }


  // use the 1.1 V internal reference
#if defined(__AVR_ATmega2560__)
  analogReference(INTERNAL1V1);
#else
  analogReference(INTERNAL);
#endif

  if (!bme.begin(BME280_ADDRESS)) {
    Serial.println(F("Could not find a valid BME280 sensor, check wiring or I2C address!"));
    while (1) {
      yield();
    }
  }
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1,   // temperature
                  Adafruit_BME280::SAMPLING_NONE, // pressure
                  Adafruit_BME280::SAMPLING_X1,   // humidity
                  Adafruit_BME280::FILTER_OFF );
}

void presentation()
{
  sendSketchInfo("TempNode", VERSION);
  present(1, S_TEMP, "Temperature");
  sleep(100);
  present(2, S_HUM, "Humidity");
  sleep(100);
  present(200, S_MULTIMETER, "VCC");
  sleep(100);
  present(201, S_CUSTOM, "cfg_Sleeptime");
}

void loop()
{
  bme.takeForcedMeasurement();
  float temperature = bme.readTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed reading temperature from DHT!");
  } else if (temperature != lastTemp || nNoUpdatesTemp == FORCE_UPDATE_N_READS) {
    // Only send temperature if it changed since the last measurement or if we didn't send an update for n times
    lastTemp = temperature;
    // Reset no updates counter
    nNoUpdatesTemp = 0;
    send(msgTemp.set(temperature, 2));

#ifdef MY_DEBUG
    Serial.print("T: ");
    Serial.println(temperature);
#endif
  } else {
    // Increase no update counter if the temperature stayed the same
    nNoUpdatesTemp++;
  }

  // Get humidity from DHT library
  float humidity = bme.readHumidity();
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
  float volt = analogRead(BATTERY_SENSE_PIN) * 0.003363075;
  // calculate the percentage
  int batteryPcnt = ((volt - _battery_min) / (_battery_max - _battery_min)) * 100;
  if (batteryPcnt > 100) batteryPcnt = 100;
  if (batteryPcnt < 0) batteryPcnt = 0;

  if (oldBatteryPcnt != batteryPcnt || nNoUpdatesHum == FORCE_UPDATE_N_READS) {
    // Power up radio after sleep
    send(msgVCC.set(volt, 2));
    send(msgcfg1.set(UPDATE_INTERVAL / 60000, 1));
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
    saveState(2, inString.toInt());
    send(msgcfg1.set(inString.toInt(), 1));
    sleep(500);
    //Reboot Node to apply new Config
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = (1 << WDE);
    while (true) {}
  }
}
