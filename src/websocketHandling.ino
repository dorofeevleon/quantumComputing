
/* *******************************************************

        Websocket handling

 *  *******************************************************
*/
//функция обработки входящих сообщений
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  if (type == WStype_CONNECTED) {
    IPAddress ip = webSocket.remoteIP(num);

    String message = String("Connected");
    webSocket.broadcastTXT(message);   // we send the last value to all clients when connecting
  }

  if (type == WStype_TEXT) {
    String data;
    for (int x = 0; x < length; x++) {
      if (!isdigit(payload[x])) continue;
      data += (char) payload[x];

    }

    if (payload[0] == 'B') {
      flag = 0;
      Serial.print("Bright: ");
      bright = data.toInt();
      Serial.println(data);
      LEDS.setBrightness(bright);
      myLight.Brightness = bright;

    }

    else if (payload[0] == 'S') { //setting data
      String myBroadcastingString =   "S=" + String(myLight.LED1 == 0 ? "0" : "1") + String(myLight.LED2 == 0 ? "0" : "1") + String(myLight.RGBStatus == 0 ? "0" : "1") + String(myLight.StripStatus == 0 ? "0" : "1") ;
      Serial.println(myBroadcastingString);
      webSocket.broadcastTXT(myBroadcastingString);

    }

    else if (payload[0] == 'F') {
      flag = 0;
      Serial.print("Function: ");
      ledMode = data.toInt();
      Serial.println(data);
      ledEffect(ledMode);
      myLight.Strip_F  = ledMode;
      myLight.StripStatus = 1;
      myLight.changed =  1; //set flag for changes
      myLight.stamp = millis(); //store timestamp
    }

    else if (payload[0] == 'L') {
      // turn on/off lights
      int myLightStatus = payload[3] - 48; //data.toInt();
      int myLightNo  = payload[1] - 48; //ata.toInt();
      Serial.print("Licht ");
      Serial.print(myLightNo);
      Serial.print(" ist ");
      Serial.println(myLightStatus);
      String myBroadcastingString;
      //Serial.println(myLightStatus);
      switch (myLightNo) {
        case 1:
          digitalWrite(PIN_LED1, myLightStatus);
          myLight.LED1 = digitalRead(PIN_LED1);
          //myBroadcastingString =   "L1=" + String(myLight.LED1);

          break;
        case 2:
          digitalWrite(PIN_LED2, myLightStatus);
          myLight.LED2 = digitalRead(PIN_LED2);
          //myBroadcastingString =   "L2=" + String(myLight.LED2);


          break;
        case 3:
          if (myLightStatus == 1) {
            analogWrite(PIN_red, 0x400 - (4 * abs(0 + (myLight.RGB_rgb >> 16) & 0xFF)));
            analogWrite(PIN_green, 0x400 - (4 * abs(0 + (myLight.RGB_rgb >>  8) & 0xFF)));
            analogWrite(PIN_blue, 0x400 - (4 * abs(0 + (myLight.RGB_rgb >>  0) & 0xFF)));
          } else {
            analogWrite(PIN_red, 0x400);
            analogWrite(PIN_green, 0x400);
            analogWrite(PIN_blue, 0x400);
          }
          myLight.RGBStatus  = myLightStatus;
          break;
        case 4:
          if (myLightStatus == 1) {
            flag = 0;
            activateStripEffekt();
          } else {
            flag = 0;
            //turn off Strip
            ledMode = flag;
            uint32_t rgb = 0x0;
            uint8_t r = abs(0 + (rgb >> 16) & 0xFF);
            uint8_t g = abs(0 + (rgb >>  8) & 0xFF);
            uint8_t b = abs(0 + (rgb >>  0) & 0xFF);

            for (int x = 0; x < LED_COUNT; x++) {
              leds[x].setRGB(r, g, b);
            }
            LEDS.show();

          }
          myLight.StripStatus  = myLightStatus;
          break;

      }
      Serial.println(myBroadcastingString);
      webSocket.broadcastTXT(myBroadcastingString);
      myLight.changed =  1; //set flag for changes
      myLight.stamp = millis(); //store timestamp

    }
    if (payload[0] == '+') {
      // LED Strip
      uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);
      Serial.print("Farbe: ");
      Serial.println(rgb);

      // преобразуем 24 бит по 8 бит на канал
      analogWrite(PIN_red, 0x400 - (4 * abs(0 + (rgb >> 16) & 0xFF)));
      analogWrite(PIN_green, 0x400 - (4 * abs(0 + (rgb >>  8) & 0xFF)));
      analogWrite(PIN_blue, 0x400 - (4 * abs(0 + (rgb >>  0) & 0xFF)));
      myLight.RGB_rgb = rgb;// store colour
      myLight.changed =  1; //set flag for changes
      myLight.stamp = millis(); //store timestamp
      myLight.RGBStatus = 1;

    }
    if (payload[0] == 'I') { //Info Data
      byte mac[6]; 
      WiFi.macAddress(mac);
      String myBroadcastingString =   "<br> SSID: " + String(WiFi.SSID()) +
                                      "<br> IP: " + WiFi.localIP().toString() +
                                      "<br> MAC: "+ mac2String(mac) +
                                      "<br> Sketch: " + myFileName +
                                      "<br> Sub-Sketch:" + String(__FILE__) +
                                      "<br> Datum:" + String(__DATE__) +
                                      " um " + String(__TIME__) +
                                      "<br> <br> Juli 2020"

                                      ;

      //          Serial.print("\nArduino is running Sketch: ");
      //  Serial.println(__FILE__);
      //  Serial.print("Compiled on: ");
      //  Serial.print();
      //  Serial.print(" at ");
      //  Serial.print(__TIME__);
      //  Serial.print("\n\n");


      Serial.println(myBroadcastingString);
      webSocket.broadcastTXT(myBroadcastingString);
    }

    //**
    else if (payload[0] == '#') {
      // WS2812B Strip Effekte
      if (!flag) {
        Serial.print("flag : ");
        Serial.println(flag);
        ledMode = flag;
        ledEffect(ledMode);
        flag = 1;
        myLight.StripStatus = 1;


      }
      else {
        //convert to 24 bit color number
        uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);

        //преобразуем 24 бит по 8 бит на канал
        uint8_t r = abs(0 + (rgb >> 16) & 0xFF);
        uint8_t g = abs(0 + (rgb >>  8) & 0xFF);
        uint8_t b = abs(0 + (rgb >>  0) & 0xFF);

        Serial.print("WS2812B ColorPicker: ");
        Serial.print((r));
        Serial.print((g));
        Serial.println((b));
        myLight.Strip_rgb = rgb;// store colour
        myLight.Strip_F  = 0; //neutralize Effects
        myLight.changed =  1; //set flag for changes
        myLight.stamp = millis(); //store timestamp
        myLight.StripStatus = 1;
        for (int x = 0; x < LED_COUNT; x++) {
          leds[x].setRGB(r, g, b);
        }
        LEDS.show();

      }
    }
  }
}

/* *******************************************************

       file type function

 *  *******************************************************
*/

//
String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";

}
/* *******************************************************

        file search function in the file system

 *  *******************************************************
*/
//
bool handleFileRead(String path) {
#ifdef DEBUG
  Serial.println("handleFileRead: " + path);
#endif
  if (path.endsWith("/")) path += "index.html";
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, getContentType(path));
    file.close();
    return true;
  }
  return false;

}



String mac2String(byte ar[]){ //https://forum.arduino.cc/index.php?topic=356979.0
  String s;
  for (byte i = 0; i < 6; ++i)
  {
    char buf[3];
    sprintf(buf, "%02X", ar[i]);
    s += buf;
    if (i < 5) s += ':';
  }
  return s;
}
