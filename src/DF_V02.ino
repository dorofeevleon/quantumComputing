/*

    Hardware: NodeMCU 0.9 4MB FS 2MB

 		colour handling used from WireKraken https://github.com/wirekraken/ESP8266-Websockets-LED
*/

#include <FS.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

#define FASTLED_ESP8266_NODEMCU_PIN_ORDER //needs before the lib is called -> https://github.com/FastLED/FastLED/wiki/ESP8266-notes
#include <FastLED.h>
#include <EEPROM.h>
#include "credentials.h"
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoOTA.h>

uint8_t bright = 25; // Brightness (0 - 255) -> bright=0 leads to sstrange LED Effects
uint8_t ledMode = 0; // Effect (0 - 29)

uint8_t flag = 1; // effect cancel flag

CRGBArray<LED_COUNT> leds;

uint8_t delayValue = 20; // delay
uint8_t stepValue = 10; // pixel pitch
uint8_t hueValue = 0; // color tone

// initialization of websocket on port 81
WebSocketsServer webSocket(81);
ESP8266WebServer server(80);


int test_ledMode;

/* *******************************************************

        Setup

 *  *******************************************************
*/

void setup() {
  Serial.begin(myBaudrate);
  myFileName = String(__FILE__);
  versionsInfo();


  //Port Definition
  pinMode(PIN_LED1, OUTPUT);
  digitalWrite(PIN_LED1, LOW);
  pinMode(PIN_LED2, OUTPUT);
  digitalWrite(PIN_LED2, LOW);



  // turn off RGB Strip
  analogWrite(PIN_red, 0x400); //10Bit PWM
  analogWrite(PIN_green, 0x400);
  analogWrite(PIN_blue, 0x400);

  LEDS.setBrightness(bright);
  LEDS.addLeds<WS2812B, PIN_WS2812B, GRB>(leds, LED_COUNT);  // settings for your tape (tapes on WS2811, WS2812, WS2812B)
  updateColor(0, 0, 0);
  LEDS.show();

  //restore old setting
  getSetting();

  WiFiManager wifiManager;
  wifiManager.autoConnect("Deckenlampe Finn");
  WiFi.begin(); //soft coded from WifiManager
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  server.begin();

  SPIFFS.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  ArduinoOTA.setHostname("DeckenlampeF");
 //ArduinoOTA.setPassword("admin");
  ArduinoOTA.begin();
}

/* *******************************************************

        L O O P

 *  *******************************************************
*/
void loop() {


  //handling incoming HTTP or WebSockets requests
  webSocket.loop();
  server.handleClient();
  ArduinoOTA.handle();
  ledEffect(ledMode);


  //store setting earliest 10second after changess made effective
  if (myLight.changed ==  1 && ((millis() - myLight.stamp) > 10000)) {
    storeSetting();
  }
}

/* *******************************************************

        store parameter to EEPROM

 *  *******************************************************
*/
void storeSetting() {

  myLight.changed = 0; // reset change flag
  EEPROM.begin(512);

  EEPROM.put(10, myLight);

  // EEPROM.begin(512);                      // Only needed for ESP8266 to get data written  https://www.kriwanek.de/index.php/de/homeautomation/esp8266/364-eeprom-f%C3%BCr-parameter-verwenden
  EEPROM.commit();
  EEPROM.end();                         // Free RAM copy of structure

  Serial.println("Setting gespeichert!");
  //readMyEEPROM();
}


/* *******************************************************

        get parameter from EEPROM

 *  *******************************************************
*/
void getSetting() {


  //https://www.arduino.cc/en/Reference/EEPROMPut
  EEPROM.begin(512);
  EEPROM.get(10, myLight);
  EEPROM.end();

  //activate LED1+LED2
  digitalWrite(PIN_LED1, myLight.LED1);
  digitalWrite(PIN_LED2, myLight.LED2);   // activate LED2

  //activate RGB
  if (myLight.RGBStatus == 1) {
    analogWrite(PIN_red, 0x400 - (4 * abs(0 + (myLight.RGB_rgb >> 16) & 0xFF)));
    analogWrite(PIN_green, 0x400 - (4 * abs(0 + (myLight.RGB_rgb >>  8) & 0xFF)));
    analogWrite(PIN_blue, 0x400 - (4 * abs(0 + (myLight.RGB_rgb >>  0) & 0xFF)));
  } else {
    analogWrite(PIN_red, 0x400);
    analogWrite(PIN_green, 0x400);
    analogWrite(PIN_blue, 0x400);
  }


  if (myLight.StripStatus == 1) {
    activateStripEffekt();
  } else { //turn off all lights
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    for (int x = 0; x < LED_COUNT; x++) {
      leds[x].setRGB(r, g, b);
    }
    LEDS.show();
  }


  Serial.println("Setting wieder hergestellt!");
  //readMyEEPROM();
}


/*=======================================================================
        activate WS2812B strip (effect or colour only)
    =======================================================================
*/
void activateStripEffekt() {
  if (myLight.Strip_F == 0) {
    //colour only
    uint8_t r = abs(0 + (myLight.Strip_rgb >> 16) & 0xFF);
    uint8_t g = abs(0 + (myLight.Strip_rgb >>  8) & 0xFF);
    uint8_t b = abs(0 + (myLight.Strip_rgb >>  0) & 0xFF);

    for (int x = 0; x < LED_COUNT; x++) {
      leds[x].setRGB(r, g, b);
    }
    LEDS.show();
  } else {
    //effects
    Serial.print("LED Effekt ");
    Serial.println(myLight.Strip_F);
    flag = 0;
    ledMode = myLight.Strip_F;

  }
  bright = myLight.Brightness  ;
  LEDS.setBrightness(bright);
}

//=======================================================================
//        read EEPROM
//       (for debug purpose only)
//=======================================================================

void readMyEEPROM() {

  byte value;
  EEPROM.begin(512);
  for (int address = 0; address < 20; address++) {
    value = EEPROM.read(address);

    Serial.print(address);
    Serial.print("\t");
    Serial.print(value, DEC);
    Serial.println();
  }
}

//=======================================================================
//                    Versionsinfo
//=======================================================================
void versionsInfo()
{
  Serial.print("\nArduino is running Sketch: ");
  Serial.println(__FILE__);
  Serial.print("Compiled on: ");
  Serial.print(__DATE__);
  Serial.print(" at ");
  Serial.print(__TIME__);
  Serial.print("\n\n");
}
