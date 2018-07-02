/**********************************
 * MySensors node configuration
 */

// General settings
#define SKETCH_NAME "WindowNode"
#define SKETCH_VERSION "1.21"
//#define MY_DEBUG
//#define MY_NODE_ID 99

// NRF24 radio settings
#define MY_RADIO_NRF24
//#define MY_RF24_ENABLE_ENCRYPTION
//#define MY_RF24_CHANNEL 125
#define MY_RF24_PA_LEVEL RF24_PA_MAX
//#define MY_DEBUG_VERBOSE_RF24
//#define MY_RF24_DATARATE RF24_250KBPS

// Advanced settings
#define MY_BAUD_RATE 9600
//#define MY_SMART_SLEEP_WAIT_DURATION_MS 500
#define MY_SPLASH_SCREEN_DISABLED

/***********************************
 * NodeManager modules for supported sensors
 */

#define USE_BATTERY
#define USE_CONFIGURATION
#define USE_INTERRUPT

/***********************************
 * NodeManager built-in features
 */

// Enable/disable NodeManager's features
#define FEATURE_DEBUG OFF
#define FEATURE_POWER_MANAGER OFF
#define FEATURE_INTERRUPTS ON
#define FEATURE_CONDITIONAL_REPORT OFF
#define FEATURE_EEPROM OFF
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
//PowerManager power(5,6);

// Attached sensors
SensorDoor door1(node,2);
SensorDoor door2(node,3);

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
  node.setSleepMinutes(60);
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
