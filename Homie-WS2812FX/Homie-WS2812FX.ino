#include <Homie.h>
#include <WS2812FX.h>

#define LED_PIN 2 
#define LED_COUNT 60

//#define DEFAULT_COLOR 0xFF5900
#define DEFAULT_RED 0
#define DEFAULT_GREEN 0
#define DEFAULT_BLUE 0
#define DEFAULT_BRIGHTNESS 255
#define DEFAULT_SPEED 200
#define DEFAULT_MODE FX_MODE_STATIC

int Color_Red = 0;
int Color_Green = 0;
int Color_Blue = 0;
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ400);

HomieNode lightNode("light", "ws2812fx");

bool lightOnHandler(const HomieRange& range, const String& value) {
  if (value != "true" && value != "false") return false;

  bool on = (value == "true");
  ws2812fx.setBrightness(on ? 255 : 0);
  lightNode.setProperty("on").send(value);
  Homie.getLogger() << "Light is " << (on ? "on" : "off") << endl;

  return true;
}
bool lightbrightHandler(const HomieRange& range, const String& value) {
  uint8_t tmp = (uint8_t) value.toInt();
  ws2812fx.setBrightness(tmp);
  lightNode.setProperty("brightness").send(value);
  //Homie.getLogger() << "Light is " << (on ? "on" : "off") << endl;
  return true;
}
bool lightspeedHandler(const HomieRange& range, const String& value) {
  uint8_t tmp = (uint8_t) value.toInt();
  ws2812fx.setSpeed(tmp);
  lightNode.setProperty("speed").send(value);
  //Homie.getLogger() << "Light is " << (on ? "on" : "off") << endl;
  return true;
}

bool lightmodeHandler(const HomieRange& range, const String& value) {
  uint8_t tmp = (uint8_t) value.toInt();
  ws2812fx.setMode(tmp);
  lightNode.setProperty("mode").send(value);
  //Homie.getLogger() << "Light is " << (on ? "on" : "off") << endl;
  return true;
}

bool lightcolorredHandler(const HomieRange& range, const String& value) {
  uint8_t tmp = (uint8_t) value.toInt();
  ws2812fx.setColor(tmp*2.55, Color_Blue, Color_Green);
  lightNode.setProperty("colorred").send(value);
  Color_Red = tmp*2.55;
  //Homie.getLogger() << "Light is " << (on ? "on" : "off") << endl;
  return true;
}
bool lightcolorgreenHandler(const HomieRange& range, const String& value) {
  uint8_t tmp = (uint8_t) value.toInt();
  ws2812fx.setColor(Color_Red, Color_Blue, tmp*2.55);
  lightNode.setProperty("colorred").send(value);
  Color_Green = tmp*2.55;
  //Homie.getLogger() << "Light is " << (on ? "on" : "off") << endl;
  return true;
}
bool lightcolorblueHandler(const HomieRange& range, const String& value) {
  uint8_t tmp = (uint8_t) value.toInt();
  ws2812fx.setColor(Color_Red, tmp*2.55, Color_Green);
  lightNode.setProperty("colorred").send(value);
  Color_Blue = tmp*2.55;
  //Homie.getLogger() << "Light is " << (on ? "on" : "off") << endl;
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  ws2812fx.init();
  ws2812fx.setMode(DEFAULT_MODE);
  ws2812fx.setColor(DEFAULT_RED, DEFAULT_GREEN, DEFAULT_BLUE);
  ws2812fx.setSpeed(DEFAULT_SPEED);
  ws2812fx.start();

  Homie_setFirmware("Homie-WS2812FX", "1.0.0");

  lightNode.advertise("on").settable(lightOnHandler);
  lightNode.advertiseRange("brightness",0,100).settable(lightbrightHandler);
  lightNode.advertiseRange("speed",0,100).settable(lightspeedHandler);
  lightNode.advertiseRange("mode",0,47).settable(lightmodeHandler);
  lightNode.advertiseRange("colorred",0,255).settable(lightcolorredHandler);
  lightNode.advertiseRange("colorgreen",0,255).settable(lightcolorgreenHandler);
  lightNode.advertiseRange("colorblue",0,255).settable(lightcolorblueHandler);

  Homie.setup();
}

void loop() {
  Homie.loop();
  ws2812fx.service();
}
