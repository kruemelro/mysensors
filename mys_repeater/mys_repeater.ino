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
 * Repeater for MySensors network with possibility to set parent node
 *
 */
 
// Default Settings Area ************************************
#define VERSION "1.21"
#define Sketchname "RepeaterNode"
#define globalconfigversion 3
int BATTERY_SENSE_PIN = A0;
int sleeptime = 0; // Sleep time in Minutes
static const uint8_t FORCE_UPDATE_N_READSstd = 1; //Should be set to report once in a hour
#define EEPROM_REPEATS 5
#define EEPROM_PA_LEVEL 2
#define EEPROM_PARENT 0
#define EEPROM_PARENTSTATIC 0
#define REPORTEVERY 60000 // only for nonsleeping Nodes
// *********************************************************

// Variables Area ******************************************
int8_t myRF24PALevel;
int8_t myParrentNodeId;
int8_t myParrentNodeStatic;
int8_t myRepeats;
int32_t msgcounter=0;
int32_t failcounter=0;
bool rptcycle=0;
bool onvalue=1;
uint64_t UPDATE_INTERVAL;
uint64_t FORCE_UPDATE_N_READS;
int8_t mySleeptime;
// *********************************************************


// Enable debug prints to serial monitor
// #define MY_DEBUG
#define MY_BAUD_RATE 9600

// Enable and select radio type attached
#define MY_RADIO_RF24
#define MY_RF24_CHANNEL (104)
#define MY_RF24_PA_LEVEL myRF24PALevel
#define MY_PARENT_NODE_ID myParrentNodeId

// Repeater Function
#define MY_REPEATER_FEATURE

#include <SPI.h>
#include <MySensors.h>

// Define childs of this Sensor
MyMessage msgVCC(200, V_VOLTAGE);
MyMessage msgcfg1(202, V_VAR1);
MyMessage msgcfg2(202, V_VAR2);
MyMessage msgcfg3(202, V_VAR3);
MyMessage msgcfg4(202, V_VAR4);
MyMessage msgcfg5(202, V_VAR5);
MyMessage iamonline(254, V_LIGHT);
MyMessage sendtx(254, V_VAR1);
MyMessage failedtx(254, V_VAR2);

void before() {
  uint8_t loadState1;
  uint8_t loadState2;
  uint8_t loadState3;
  uint8_t loadState4;
  uint8_t loadState5;
  uint8_t loadState6;
  loadState1 = loadState(21);
  loadState2 = loadState(22);
  loadState3 = loadState(23);
  loadState4 = loadState(24);
  loadState5 = loadState(25);
  loadState6 = loadState(26);
  if (loadState1 != globalconfigversion) {
    // Eprom is not set, first Boot or Update to EEPROM Structure!
    Serial.println("setting Defaults");

    Serial.print("Store - Sleeptime: ");
    Serial.println(sleeptime);
    saveState(22, sleeptime);
    UPDATE_INTERVAL = sleeptime * 60000;
    mySleeptime = sleeptime;
    FORCE_UPDATE_N_READS = FORCE_UPDATE_N_READSstd;
    
    Serial.print("Store - Parent ID: ");
    myParrentNodeId = EEPROM_PARENT;
    Serial.println(EEPROM_PARENT);
    saveState(23, EEPROM_PARENT);
    
    Serial.print("Store - Radio Level: ");
    Serial.println(EEPROM_PA_LEVEL);
    saveState(24, EEPROM_PA_LEVEL);
    loadState4 = EEPROM_PA_LEVEL;

    Serial.print("Store - Parent is static: ");
    Serial.println(EEPROM_PARENTSTATIC);
    saveState(25, EEPROM_PARENTSTATIC);
    loadState5 = EEPROM_PARENTSTATIC;

    Serial.print("Store - Send Repeats: ");
    Serial.println(EEPROM_REPEATS);
    saveState(26, EEPROM_REPEATS);
    myRepeats = EEPROM_REPEATS;
    
    // Save Eprom State 1 to know its allready configured
    saveState(21, globalconfigversion);
  }
  else {
    // Eprom is set, read it!
    Serial.println("Boot, read EEPROM Values");

    // Sleeptime
    UPDATE_INTERVAL = loadState2 * 60000;
    mySleeptime = loadState2;
    FORCE_UPDATE_N_READS = FORCE_UPDATE_N_READSstd;
    
    // Parent NODE ID
    Serial.print("parentnode in Eprom: ");
    Serial.println(loadState3);
    myParrentNodeId = loadState3;
    
    // Radio Power Level
    Serial.print("Radio Level in Eprom: ");
    Serial.println(loadState4);

    // Parent NODE static
    Serial.print("Parent static in Eprom: ");
    Serial.println(loadState5);
    myParrentNodeStatic = loadState5;

    // Send Repeats
    Serial.print("Send Repeats in Eprom: ");
    Serial.println("loadState6");
    myRepeats = loadState6;
    
  }
// Set Radio Power Level
  switch (loadState4)
  {
    case 0:
      myRF24PALevel = RF24_PA_MIN;
      break;
    case 1:
      myRF24PALevel = RF24_PA_LOW;
      break;
    case 2:
      myRF24PALevel = RF24_PA_HIGH;
      break;
    case 3:
      myRF24PALevel = RF24_PA_MAX;
      break;
    default:
      myRF24PALevel = RF24_PA_HIGH;
      break;
  }

// Set Parent Static
if (loadState5 == 1) {
  #define MY_PARENT_NODE_IS_STATIC
  Serial.println("DEBUG: PARENT IS STATIC!!!!!");
}
}

void setup()
{
  // use the 1.1 V internal reference
  #if defined(__AVR_ATmega2560__)
    analogReference(INTERNAL1V1);
  #else
    analogReference(INTERNAL);
  #endif
  
  Serial.println("-------------------------------------------------");
  Serial.println("                 Network Settings                ");
  Serial.println("-------------------------------------------------");
  Serial.print("Parent Node ID: ");
  Serial.println(_transportConfig.parentNodeId);
  Serial.print("Radio Power: ");
  Serial.println(MY_RF24_PA_LEVEL);
  if (myParrentNodeStatic == 0) {
    Serial.println("Parent Node is dynamic");
  }
  else
  {
    Serial.println("Parent Node is static");
  }

  resend(msgcfg1.set(_transportConfig.parentNodeId, 1));
  resend(msgcfg2.set(MY_RF24_PA_LEVEL, 1));
  resend(msgcfg3.set(myParrentNodeStatic, 1));
  resend(msgcfg4.set(myRepeats));
  resend(msgcfg5.set(mySleeptime));
  resend(iamonline.set(onvalue, 1));
  resend(sendtx.set(msgcounter, 1));
  resend(failedtx.set(failcounter, 1));
  Serial.println("Setup Done");
}

void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo(Sketchname, VERSION);

  present(200, S_MULTIMETER, "VCC");
  sleep(100);
  present(202, S_CUSTOM, "System-Config");
  sleep(100);
  present(254, S_LIGHT, "System-Reporting");
  sleep(100);
  Serial.println("Presentation Done");
}

void loop()
{
  static unsigned long lLastRadioSettingsPrint = millis();
  wait(1);
  if ((millis()- lLastRadioSettingsPrint) > REPORTEVERY) {
    float volt = analogRead(BATTERY_SENSE_PIN) * 0.003363075;
    resend(msgVCC.set(volt,2));
    resend(iamonline.set(onvalue));
    resend(sendtx.set(msgcounter));
    resend(failedtx.set(failcounter));
    resend(msgcfg5.set(mySleeptime));
    lLastRadioSettingsPrint = millis();
  }
}

void resend(MyMessage &msg)
{
  int repeat = 1;
  int repeatdelay = 0;
  boolean sendOK = false;

  while ((sendOK == false) and (repeat < myRepeats)) {
    if (send(msg)) {
      sendOK = true;
      msgcounter++;
    } else {
      sendOK = false;
      repeatdelay += 50;
    } repeat++; wait(repeatdelay);
  }
  if (sendOK == false) {
    failcounter++;
  }
}

// Remote Config Messages
void receive(const MyMessage &message) {
  if (message.sensor == 202 && message.getCommand() == C_SET && message.type == V_VAR1) {
    int newvalue = message.getInt();
    Serial.print("REMOTE-CONFIG: Got new parent: ");
    Serial.println(newvalue);
    if (newvalue >=0 && newvalue < 255) {
      Serial.println("REMOTE-CONFIG: Value is valid");
      saveState(23, newvalue);
      resend(msgcfg1.set(newvalue));
    }
    else {
      Serial.println("REMOTE-CONFIG: Value is invalid");
    }
    //Reboot Node to apply new Config
    sleep(500);
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = (1 << WDE);
    while (true) {}
  }

  if (message.sensor == 202 && message.getCommand() == C_SET && message.type == V_VAR2) {
    int newvalue = message.getInt();
    Serial.print("REMOTE-CONFIG: Got new Radio Level: ");
    Serial.println(newvalue);
    if (newvalue >=0 && newvalue < 4) {
      Serial.println("REMOTE-CONFIG: Value is valid");
      saveState(24, newvalue);
      resend(msgcfg2.set(newvalue));
    }
    else {
      Serial.println("REMOTE-CONFIG: Value is invalid");
    }
    sleep(500);
    //Reboot Node to apply new Config
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = (1 << WDE);
    while (true) {}
  }

  if (message.sensor == 202 && message.getCommand() == C_SET && message.type == V_VAR3) {
    int newvalue = message.getInt();
    Serial.print("REMOTE-CONFIG: Got new Parent is static: ");
    Serial.println(newvalue);
    if (newvalue >=0 && newvalue < 2) {
      Serial.println("REMOTE-CONFIG: Value is valid");
      saveState(25, newvalue);
      resend(msgcfg3.set(newvalue));
    }
    else {
      Serial.println("REMOTE-CONFIG: Value is invalid");
    }
    sleep(500);
    //Reboot Node to apply new Config
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = (1 << WDE);
    while (true) {}
  }

  if (message.sensor == 202 && message.getCommand() == C_SET && message.type == V_VAR4) {
    int newvalue = message.getInt();
    Serial.print("REMOTE-CONFIG: Got new send repeat: ");
    Serial.println(newvalue);
    if (newvalue >=1 && newvalue < 61) {
      Serial.println("REMOTE-CONFIG: Value is valid");
      saveState(26, newvalue);
      resend(msgcfg4.set(newvalue));
    }
    else {
      Serial.println("REMOTE-CONFIG: Value is invalid");
    }
    sleep(500);
    //Reboot Node to apply new Config
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = (1 << WDE);
    while (true) {}
  }

  if (message.sensor == 202 && message.getCommand() == C_SET && message.type == V_VAR5) {
    int newvalue = message.getInt();
    Serial.print("REMOTE-CONFIG: Got new sleeptime: ");
    Serial.println(newvalue);
    if (newvalue >0 && newvalue < 61) {
      Serial.println("REMOTE-CONFIG: Value is valid");
      saveState(22, newvalue);
      resend(msgcfg5.set(newvalue));
    }
    else {
      Serial.println("REMOTE-CONFIG: Value is invalid");
    }
    sleep(500);
    //Reboot Node to apply new Config
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = (1 << WDE);
    while (true) {}
  }
}
