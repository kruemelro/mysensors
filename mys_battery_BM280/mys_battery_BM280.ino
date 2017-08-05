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
int sleeptime = 2; // Sleep time in Minutes
static const uint8_t FORCE_UPDATE_N_READSstd = 30; //Should be set to report once in a hour
// ********************************************************

// Enable debug prints to serial monitor
#define MY_DEBUG
#define MY_BAUD_RATE 9600

// Enable and select radio type attached
#define MY_RADIO_NRF24

#include <SPI.h>
#include <MySensors.h>  
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

int BATTERY_SENSE_PIN = A0;
int oldBatteryPcnt = 0;
float _battery_min = 1.8;
float _battery_max = 3.2;

const char *weather[] = { "stable", "sunny", "cloudy", "unstable", "thunderstorm", "unknown" };
enum FORECAST
{
  STABLE = 0,     // "Stable Weather Pattern"
  SUNNY = 1,      // "Slowly rising Good Weather", "Clear/Sunny "
  CLOUDY = 2,     // "Slowly falling L-Pressure ", "Cloudy/Rain "
  UNSTABLE = 3,   // "Quickly rising H-Press",     "Not Stable"
  THUNDERSTORM = 4, // "Quickly falling L-Press",    "Thunderstorm"
  UNKNOWN = 5     // "Unknown (More Time needed)
};
const int LAST_SAMPLES_COUNT = 5;
float lastPressureSamples[LAST_SAMPLES_COUNT];

// this CONVERSION_FACTOR is used to convert from Pa to kPa in forecast algorithm
// get kPa/h be dividing hPa by 10 
#define CONVERSION_FACTOR (1.0/10.0)

int minuteCount = 0;
bool firstRound = true;
// average value is used in forecast algorithm.
float pressureAvg;
// average after 2 hours is used as reference value for the next iteration.
float pressureAvg2;

float lastTemp;
float lastHum;
float lastBaro;
int lastForecast;
uint8_t nNoUpdatesTemp;
uint8_t nNoUpdatesHum;
uint8_t nNoUpdatesBaro;
uint64_t UPDATE_INTERVAL;
uint64_t FORCE_UPDATE_N_READS;
bool outdoor = false;
float dP_dt;

#undef BME280_ADDRESS         // Undef BME280_ADDRESS from the BME280 library to easily override I2C address
#define BME280_ADDRESS (0x76) // Low = 0x76 , High = 0x77 (default on adafruit and sparkfun BME280 modules, default for library)

Adafruit_BME280 bme; // Use I2C

MyMessage msgTemp(1, V_TEMP);
MyMessage msgHum(2, V_HUM);
MyMessage msgBaro(3, V_PRESSURE);
MyMessage msgBaro2(3, V_FORECAST);
MyMessage msgcfg1(201, V_VAR1);
MyMessage msgcfg2(201, V_VAR2);
MyMessage msgcfg3(201, V_VAR3);

void setup()
{
  uint8_t loadState1;
  uint8_t loadState2;
  uint8_t loadState3;  
  loadState1 = loadState(1);
  loadState2 = loadState(2);
  loadState3 = loadState(3);
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
    Serial.print("REMOTE-CONFIG: Store - Outdoor: ");
    Serial.println(0);
    saveState(3, 0);
    outdoor = false;
    send(msgcfg1.set(sleeptime, 1));
    send(msgcfg2.set(FORCE_UPDATE_N_READSstd, 1));
    send(msgcfg3.set(outdoor, 1));
  }
  else {
    // Eprom is set, read it!
    Serial.print("REMOTE-CONFIG: Sleeptime in Eprom: ");
    Serial.println(loadState1);
    Serial.print("REMOTE-CONFIG: Force-Reads in Eprom: ");
    Serial.println(loadState2);
    Serial.print("REMOTE-CONFIG: Outdoor in Eprom: ");
    Serial.println(loadState3);
    UPDATE_INTERVAL = loadState1 * 60000;
    FORCE_UPDATE_N_READS = loadState2;
    send(msgcfg1.set(loadState1, 1));
    send(msgcfg2.set(loadState2, 1));
    send(msgcfg3.set(loadState3, 1));
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
}

void presentation()
{
  //reads the eprom here to cause presention is executed befor setup()
  uint8_t loadState3;
  loadState3 = loadState(3);
  // Send the sketch version information to the gateway and Controller
  if (loadState3 == 1) {
    sendSketchInfo("TempOutdoorNode", VERSION);
    outdoor = true;
    present(1, S_TEMP);
    sleep(100);
    present(2, S_HUM);
    sleep(100);
    present(3, S_BARO);
    sleep(100);
    present(201, S_CUSTOM);
  }
  else {
    sendSketchInfo("TempNode", VERSION);
    outdoor = false;
    present(1, S_TEMP);
    sleep(100);
    present(2, S_HUM);
    sleep(100);
    present(201, S_CUSTOM);
  }
  
}

void loop()
{
float temperature = bme.readTemperature();
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

  
// Get Barmoter from DHT library if Sensor is configured to Outdoor
  if (outdoor) {
  float baro = bme.readPressure() / 100.0;
  int forecast = sample(baro);
  if (isnan(baro)) {
    Serial.println("Failed reading humidity from DHT");
  } else if (baro != lastBaro || nNoUpdatesBaro == FORCE_UPDATE_N_READS) {
    // Only send Barometer if it changed since the last measurement or if we didn't send an update for n times
    lastBaro = baro;
    lastForecast = forecast;
    // Reset no updates counter
    nNoUpdatesBaro = 0;
    send(msgBaro.set(baro, 1));
    send(msgBaro2.set(weather[forecast]));
    
    #ifdef MY_DEBUG
    Serial.print("P: ");
    Serial.println(baro);
    #endif
  } else {
    // Increase no update counter if the humidity stayed the same
    nNoUpdatesBaro++;
  }
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
  if (message.sensor == 201 && message.getCommand() == C_SET && message.type == V_VAR3) {
    String inString = message.getString();
    Serial.print("REMOTE-CONFIG: Got new Outdoor: ");
    Serial.println(inString.toInt());
    saveState(3, inString.toInt());
    sleep(500);
    //Reboot Node to apply new Config
      WDTCSR |= (1<<WDCE) | (1<<WDE);
      WDTCSR= (1<<WDE);
      while(true){}
  }
  
}

float getLastPressureSamplesAverage()
{
  float lastPressureSamplesAverage = 0;
  for (int i = 0; i < LAST_SAMPLES_COUNT; i++)
  {
    lastPressureSamplesAverage += lastPressureSamples[i];
  }
  lastPressureSamplesAverage /= LAST_SAMPLES_COUNT;

  return lastPressureSamplesAverage;
}



// Algorithm found here
// http://www.freescale.com/files/sensors/doc/app_note/AN3914.pdf
// Pressure in hPa -->  forecast done by calculating kPa/h
int sample(float pressure)
{
  // Calculate the average of the last n minutes.
  int index = minuteCount % LAST_SAMPLES_COUNT;
  lastPressureSamples[index] = pressure;

  minuteCount++;
  if (minuteCount > 185)
  {
    minuteCount = 6;
  }

  if (minuteCount == 5)
  {
    pressureAvg = getLastPressureSamplesAverage();
  }
  else if (minuteCount == 35)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) // first time initial 3 hour
    {
      dP_dt = change * 2; // note this is for t = 0.5hour
    }
    else
    {
      dP_dt = change / 1.5; // divide by 1.5 as this is the difference in time from 0 value.
    }
  }
  else if (minuteCount == 65)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) //first time initial 3 hour
    {
      dP_dt = change; //note this is for t = 1 hour
    }
    else
    {
      dP_dt = change / 2; //divide by 2 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 95)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) // first time initial 3 hour
    {
      dP_dt = change / 1.5; // note this is for t = 1.5 hour
    }
    else
    {
      dP_dt = change / 2.5; // divide by 2.5 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 125)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    pressureAvg2 = lastPressureAvg; // store for later use.
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) // first time initial 3 hour
    {
      dP_dt = change / 2; // note this is for t = 2 hour
    }
    else
    {
      dP_dt = change / 3; // divide by 3 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 155)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) // first time initial 3 hour
    {
      dP_dt = change / 2.5; // note this is for t = 2.5 hour
    }
    else
    {
      dP_dt = change / 3.5; // divide by 3.5 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 185)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) // first time initial 3 hour
    {
      dP_dt = change / 3; // note this is for t = 3 hour
    }
    else
    {
      dP_dt = change / 4; // divide by 4 as this is the difference in time from 0 value
    }
    pressureAvg = pressureAvg2; // Equating the pressure at 0 to the pressure at 2 hour after 3 hours have past.
    firstRound = false; // flag to let you know that this is on the past 3 hour mark. Initialized to 0 outside main loop.
  }

  int forecast = UNKNOWN;
  if (minuteCount < 35 && firstRound) //if time is less than 35 min on the first 3 hour interval.
  {
    forecast = UNKNOWN;
  }
  else if (dP_dt < (-0.25))
  {
    forecast = THUNDERSTORM;
  }
  else if (dP_dt > 0.25)
  {
    forecast = UNSTABLE;
  }
  else if ((dP_dt > (-0.25)) && (dP_dt < (-0.05)))
  {
    forecast = CLOUDY;
  }
  else if ((dP_dt > 0.05) && (dP_dt < 0.25))
  {
    forecast = SUNNY;
  }
  else if ((dP_dt >(-0.05)) && (dP_dt < 0.05))
  {
    forecast = STABLE;
  }
  else
  {
    forecast = UNKNOWN;
  }

  // uncomment when debugging
  //Serial.print(F("Forecast at minute "));
  //Serial.print(minuteCount);
  //Serial.print(F(" dP/dt = "));
  //Serial.print(dP_dt);
  //Serial.print(F("kPa/h --> "));
  //Serial.println(weather[forecast]);

  return forecast;
}
