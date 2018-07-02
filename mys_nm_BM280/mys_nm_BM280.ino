/**********************************
 * MySensors node configuration
 */

// General settings
#define SKETCH_NAME "TempNode"
#define SKETCH_VERSION "1.24"
//#define MY_DEBUG

// NRF24 radio settings
#define MY_RADIO_NRF24

// Advanced settings
#define MY_BAUD_RATE 9600
#define MY_SMART_SLEEP_WAIT_DURATION_MS 1000
#define MY_SPLASH_SCREEN_DISABLED

/***********************************
 * NodeManager modules for supported sensors
 */
 
#define USE_BATTERY
#define USE_CONFIGURATION
#define USE_OPENHABCONFIGURATION
#define USE_BME280

/***********************************
 * NodeManager built-in features
 */
 
#define FEATURE_DEBUG OFF
#define FEATURE_POWER_MANAGER OFF
#define FEATURE_INTERRUPTS OFF
#define FEATURE_CONDITIONAL_REPORT OFF
#define FEATURE_EEPROM ON
#define FEATURE_SLEEP ON
#define FEATURE_RECEIVE ON
#define FEATURE_TIME OFF
#define FEATURE_RTC OFF
#define FEATURE_SD OFF
#define FEATURE_HOOKING OFF

/***********************************
 * Load NodeManager Library
 */

#include "NodeManagerLibrary.h"
NodeManager node;

/***********************************
 * Add your sensors below
 */

// built-in sensors
SensorBattery battery(node);
SensorConfiguration configuration(node);
SensorOpenHABConfiguration ohabcfg(node);

// Attached sensors
SensorBME280 bme280(node);

/***********************************
 * Main Sketch
 */

// before
void before() {
  // setup the serial port baud rate
  Serial.begin(MY_BAUD_RATE);
  /*
  * Configure your sensors below
  */
  node.setADCOff();
  node.setSleepBetweenSend(150);
  node.setSleepMinutes(2);
  node.setSaveSleepSettings(true);
  battery.setMinVoltage(1.8);
  battery.setMaxVoltage(3.2);
  /*
  * Configure your sensors above
  */
  node.before();
}

// presentation
void presentation() {
  // call NodeManager presentation routine
  node.presentation();
}

// setup
void setup() {
  // call NodeManager setup routine
  node.setup();
}

// loop
void loop() {
  // call NodeManager loop routine
  node.loop();
}

#if FEATURE_RECEIVE == ON
// receive
void receive(const MyMessage &message) {
  // call NodeManager receive routine
  node.receive(message);
}
#endif

#if FEATURE_TIME == ON
// receiveTime
void receiveTime(unsigned long ts) {
  // call NodeManager receiveTime routine
  node.receiveTime(ts);
}
#endif
