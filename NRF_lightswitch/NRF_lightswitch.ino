// Include app_gpiote and related files from NRF5 SDK 12
extern "C" {
#include "app_gpiote.h"
#include "nrf_gpio.h"
#include "app_error.h"
}
#include <nrf.h>


// Default Settings Area ******************************************
#define VERSION "1.02"

int sleeptime = 360; // Sleep time in Minutes
// ********************************************************


#define IS_NRF51  //true if the target is an nRF51.  If an nRF52, then comment this line out!
#define APP_GPIOTE_MAX_USERS 1  // max users for app_gpiote, we only use one
#define MY_RADIO_NRF5_ESB
#define MY_NODE_ID 60   // To avoid getting a new ID at each flashing of the sensors...

#include <MySensors.h>

#define LED_BUILTIN             (29)
#define PIN_BUTTON1             (28)

// variables for app_gpiote calls
uint32_t err_code;
static app_gpiote_user_id_t m_gpiote_user_id;
uint32_t PIN_BUTTON1_MASK; // Mask for PIN_BUTTON1 input
uint64_t UPDATE_INTERVAL;

// Sensor messages
#define CHILD_ID_DOOR 1
MyMessage doorMsg(CHILD_ID_DOOR, V_STATUS);
MyMessage msgVCC(200, V_VOLTAGE);
bool last_sent_value;
bool door_status;
long last_button_event = 0;

// Cause of interrupt
volatile byte interrupt_cause = 0;


// Settings to avoid killing coin cell in case of connection problem
#define MY_TRANSPORT_WAIT_READY_MS  10000
#define MY_SLEEP_TRANSPORT_RECONNECT_TIMEOUT_MS 5000

// Battery settings
#define BATTERY_ALERT_LEVEL 30  // (%) Will triple blink after sending data if battery is equal or below this level
// Parameters for VCC measurement
#define BATTERY_VCC_MIN  2400  // Minimum expected Vcc level, in milliVolts. 
#define BATTERY_VCC_MAX  2900  // Maximum expected Vcc level, in milliVolts.
// This a a coefficient to fix the imprecision of measurement of the battery voltage
#define BATTERY_COEF 1000.0f  // (reported voltage / voltage) * 1000
uint16_t currentBatteryPercent;
uint16_t lastBatteryPercent = -1;
// Enables/disables sleeps between sendings to optimize for CR2032 or similar coin cell
#define USE_COIN_CELL


// Called before initialization of the library
void before() {
  hwPinMode(LED_BUILTIN, OUTPUT_D0H1);
  blinkityBlink(2, 3);
}

// Setup node
void setup(void) {

  //Configure button pins as inputs
  nrf_gpio_cfg_input(PIN_BUTTON1, NRF_GPIO_PIN_NOPULL);
  APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);               //Only initialize once. Increase value of APP_GPIOTE_MAX_USERS if needed

  // Initialize value of pin (for DRV5032 hall sensor HIGH = no magnet nearby = door opened);
  door_status = digitalRead(PIN_BUTTON1);
  //last_sent_value = !door_status; // so we always send value in first loop
  sendDoorStatus();

  // Registers user and pins we are "watching"
  //   gpiote_event_handler is handler called by interrupt, see signature below
  PIN_BUTTON1_MASK = 1 << PIN_BUTTON1; // Set mask, will be used for registration and interrupt handler
  //  app_gpiote_user_register(p_user_id, pins_low_to_high_mask, pins_high_to_low_mask, event_handler)
  //  to have no trigger for high=>low or low=>high change on your button, pass 0 instead
  //  here I check PIN_BUTTON1 on both low=>high and high=>low changes and PIN_BUTTON2 only on high=>low change when someone presses the button
  err_code = app_gpiote_user_register(&m_gpiote_user_id, PIN_BUTTON1_MASK, PIN_BUTTON1_MASK, gpiote_event_handler);
  APP_ERROR_CHECK(err_code);  // will reset if user registration fails

  // Enable SENSE and interrupt
  err_code = app_gpiote_user_enable(m_gpiote_user_id);
  APP_ERROR_CHECK(err_code);  // will reset if SENSE enabling fails

  last_button_event = millis();
  UPDATE_INTERVAL = sleeptime * 60000;
}

void presentation()  {
  sleep(100);
  sendSketchInfo("LightSwitch", VERSION);
  sleep(100);
  present(CHILD_ID_DOOR, S_BINARY, "LightSwitch");
  sleep(100);
  present(200, S_MULTIMETER, "VCC");
}

// Sleep between sendings to preserve coin cell
//  if not using button cell just make sure the #define USE_COIN_CELL is commented at the beginning of the sketch and it will do nothing
void sleepForCoinCell() {
#ifdef USE_COIN_CELL
  sleep(400);
#endif
}

// main loop
void loop(void)
{
  // blinkityBlink(2, 3); // not so useful, just for testing :)
  // for sending battery level at first run
  if (lastBatteryPercent < 0) {
    sendBatteryStatus();
    sleepForCoinCell();
  }

  if (interrupt_cause == PIN_BUTTON1) {
    
    // debounce Button
    if (millis() < last_button_event || (millis() - last_button_event > 100)) {
      last_button_event = millis();
      if (door_status != last_sent_value) {
      sendDoorStatus();
    }
    }
    // if door status changed, we send door message
    
  }
  else {  // end of sleeping period, we send battery level
    sendBatteryStatus();
  }

  // Go to sleep
  mySleepPrepare();
  interrupt_cause = 0; // reset interrupt cause
  sleep(UPDATE_INTERVAL);
}

void sendDoorStatus() {
  send(doorMsg.set(door_status));
  last_sent_value = door_status;
}

void sendBatteryStatus() {
  uint16_t batteryVoltage = hwCPUVoltage();
  float volt = batteryVoltage;
  volt = volt / 1000;

  if (batteryVoltage > BATTERY_VCC_MAX) {
    currentBatteryPercent = 100;
  }
  else if (batteryVoltage < BATTERY_VCC_MIN) {
    currentBatteryPercent = 0;
  }
  else {
    currentBatteryPercent = (100 * (batteryVoltage - BATTERY_VCC_MIN)) / (BATTERY_VCC_MAX - BATTERY_VCC_MIN);
  }
  
  if (currentBatteryPercent != lastBatteryPercent) {
    sendBatteryLevel(currentBatteryPercent);
    sleep(100);
    send(msgVCC.set(volt, 2));
    lastBatteryPercent = currentBatteryPercent;
  }
}

// "Interrupt handler"
//  not real handler, but call inside handler to
void gpiote_event_handler(uint32_t event_pins_low_to_high, uint32_t event_pins_high_to_low)
{
  MY_HW_RTC->CC[0] = (MY_HW_RTC->COUNTER + 2); // Taken from d0016 example code, ends the sleep delay
  if ((PIN_BUTTON1_MASK & event_pins_low_to_high) || (PIN_BUTTON1_MASK & event_pins_high_to_low)) {
    interrupt_cause = PIN_BUTTON1;
    door_status = !door_status;
  }
}



/**
   Utility functions for NRF51/52, from nerverdie's code here
   https://forum.mysensors.org/topic/6961/nrf5-bluetooth-action/1307
*/


void disableNfc() {  //only applied to nRF52

#ifndef IS_NRF51
  //Make pins 9 and 10 usable as GPIO pins.
  NRF_NFCT->TASKS_DISABLE = 1; //disable NFC
  NRF_NVMC->CONFIG = 1; // Write enable the UICR
  NRF_UICR->NFCPINS = 0; //Make pins 9 and 10 usable as GPIO pins.
  NRF_NVMC->CONFIG = 0; // Put the UICR back into read-only mode.
#endif
}

void turnOffRadio() {
  NRF_RADIO->TASKS_DISABLE = 1;
  while (!(NRF_RADIO->EVENTS_DISABLED)) {}  //until radio is confirmed disabled
}

void turnOffUarte0() {
#ifndef IS_NRF51
  NRF_UARTE0->TASKS_STOPRX = 1;
  NRF_UARTE0->TASKS_STOPTX = 1;
  NRF_UARTE0->TASKS_SUSPEND = 1;
  NRF_UARTE0->ENABLE = 0; //disable UART0
  while (NRF_UARTE0->ENABLE != 0) {}; //wait until UART0 is confirmed disabled.
#endif

#ifdef IS_NRF51
  NRF_UART0->TASKS_STOPRX = 1;
  NRF_UART0->TASKS_STOPTX = 1;
  NRF_UART0->TASKS_SUSPEND = 1;
  NRF_UART0->ENABLE = 0; //disable UART0
  while (NRF_UART0->ENABLE != 0) {}; //wait until UART0 is confirmed disabled.
#endif
}

void turnOffAdc() {
#ifndef IS_NRF51
  if (NRF_SAADC->ENABLE) { //if enabled, then disable the SAADC
    NRF_SAADC->TASKS_STOP = 1;
    while (NRF_SAADC->EVENTS_STOPPED) {} //wait until stopping of SAADC is confirmed
    NRF_SAADC->ENABLE = 0; //disable the SAADC
    while (NRF_SAADC->ENABLE) {} //wait until the disable is confirmed
  }
#endif
}


void turnOffHighFrequencyClock() {
  NRF_CLOCK->TASKS_HFCLKSTOP = 1;
  while ((NRF_CLOCK->HFCLKSTAT) & 0x0100) {}  //wait as long as HF clock is still running.
}


void mySleepPrepare()
{
  turnOffHighFrequencyClock();
  turnOffRadio();
  turnOffUarte0();
}


void blinkityBlink(uint8_t pulses, uint8_t repetitions) {
  for (int x = 0; x < repetitions; x++) {
    // wait only in case there's been a previous blink
    if (x > 0) {
      sleep(500);
    }
    for (int i = 0; i < pulses; i++) {
      // wait only in case there's been a previous blink
      if (i > 0) {
        sleep(100);
      }
      digitalWrite(LED_BUILTIN, HIGH);
      wait(20);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}



