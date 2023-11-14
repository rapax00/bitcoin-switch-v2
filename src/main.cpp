/*
 *  Inspiration project: Bitcoin Switch
 *  - Web: https://ereignishorizont.xyz/bitcoinswitch/en/
 *  - Repository: https://github.com/lnbits/bitcoinswitch
 *  Aditionals:
 *  - OLED Library: https://github.com/olikraus/u8g2
 *  - Web Socket Library: https://github.com/Links2004/arduinoWebSockets
 */

#include <Arduino.h>
#include <U8g2lib.h>
#include <WebSocketsClient.h>
#include <string>

#include "auxiliars.hpp"
#include "env.hpp"
#include "wifi.hpp"

//////////////////////////////////// OLED /////////////////////////////////////

const uint8_t PIN_SCL = 22;
const uint8_t PIN_SDA = 21;

U8G2_SH1106_128X32_VISIONOX_F_HW_I2C u8g2(U8G2_R0, 4, PIN_SCL, PIN_SDA);

////////////////////////////////// end OLED ///////////////////////////////////

//////////////////////////////////// SOCKET ///////////////////////////////////

String switchStr = ENV_SWITCH;

int uidLength = 22;  // length of lnurldevice id
String urlPrefix;    // xx://
String lnbitsServer; // xxxx.xxx.xxx
String apiUrl;       // /xxx/xx/xx/
String deviceId;     // xxxxxxxxxxxx

WebSocketsClient webSocket; // declare instance of websocket

String payloadStr;
bool paid;
int pinFromSocket;
int delayFromSocket;

////////////////////////////////// end SOCKET /////////////////////////////////

void setVariables();
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);

void setup() {
  Serial.begin(115200); // Start serial communication
  pinMode(BUILTIN_LED, OUTPUT);

  //// OLED ////
  u8g2.begin();
  u8g2.setFont(u8g2_font_amstrad_cpc_extended_8f);
  u8g2.setContrast(40);

  //// Conection to WiFi ////
  Serial.print("Conecting to WiFi...");
  u8g2.drawStr(0, 20, "Conecting WiFi");
  u8g2.sendBuffer();

  WiFi.begin(ENV_SSID, ENV_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    blinkOnboardLed();
    delay(700);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
  } else {
    Serial.println("\nWiFi not connected!");
  }

  u8g2.clear();

  //// Conection to websocket ////
  setVariables();
  Serial.println("Conceting to websocket: " + urlPrefix + lnbitsServer + apiUrl + deviceId);
  u8g2.drawStr(0, 20, "Conecting WSc");
  u8g2.sendBuffer();

  webSocket.beginSSL(lnbitsServer, 443, apiUrl + deviceId);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(1000);
  while (!webSocket.isConnected()) {
    webSocket.loop();
  }

  u8g2.clear();

  //// Ready ////
  Serial.println("Ready!");
  u8g2.drawStr(0, 20, "Ready!");
  u8g2.sendBuffer();
  delay(1000);
  u8g2.clear();
}

void loop() {
  checkWiFi(); // Check WiFi conection

  Serial.println("Waiting payment");
  u8g2.setFont(u8g2_font_amstrad_cpc_extended_8f);
  u8g2.drawStr(4, 20, "Waiting payment");
  u8g2.sendBuffer();

  //// Check if recive pay /////
  while (paid == false) {
    webSocket.loop();
    if (paid) {
      pinFromSocket = getValue(payloadStr, '-', 0).toInt(); // get value of pin config in socket
      pinMode(pinFromSocket, OUTPUT);
      delayFromSocket = getValue(payloadStr, '-', 1).toInt(); // get value of delay config in socket

      //// payment recived ////
      u8g2.drawStr(0, 20, "Payment recived!");
      u8g2.sendBuffer();
      digitalWrite(pinFromSocket, HIGH);
      delay(1000);
      digitalWrite(pinFromSocket, LOW);
      u8g2.clear();

      //// prepare beer ////
      u8g2.drawStr(16, 10, "Prepare beer");
      u8g2.sendBuffer();
      delay(1000);
      u8g2.setFont(u8g2_font_courB18_tf);
      for (int i = 0; i < 3; i++) { // Countdown
        u8g2.drawGlyph(60, 30, 0x0033 - i);
        u8g2.sendBuffer();
        delay(1000);
      }
      u8g2.clear();

      //// serving beer ////
      u8g2.setFont(u8g2_font_amstrad_cpc_extended_8f);
      u8g2.drawStr(0, 10, "Serving...");
      u8g2.setFont(u8g2_font_unifont_t_symbols);
      u8g2.drawGlyph(96, 16, 0x2615); // coffee 2615

      // progress bar background //
      for (int i = 0; i < 128; i = i + 8) {
        u8g2.drawGlyph(i, 32, 0x2591);
      }
      u8g2.sendBuffer();

      // activate switch //
      digitalWrite(pinFromSocket, HIGH);

      // progress bar //
      int limit = delayFromSocket / 128;
      for (int i = 0; i < limit; i++) {
        u8g2.drawGlyph(i * (128 / (float)limit), 32, 0x2593);
        u8g2.sendBuffer();
        delay(128);
      }
      u8g2.clear(); // clear display and buffer

      digitalWrite(pinFromSocket, LOW);

      u8g2.setFont(u8g2_font_amstrad_cpc_extended_8f);
      u8g2.drawStr(36, 20, "cheers!");
      u8g2.sendBuffer();
      delay(1500);
      u8g2.clear(); // clear display and buffer
    }
  }

  paid = false;
}

/////////////////// HELPER ////////////////////

void setVariables() {
  if (switchStr == NULL) {
    Serial.println("error: switch URL null");
    return;
  }

  int protocolIndex = switchStr.indexOf("://");
  urlPrefix = switchStr.substring(0, protocolIndex + 3);

  int domainIndexEnd = switchStr.indexOf("/", protocolIndex + 3);
  lnbitsServer = switchStr.substring(protocolIndex + 3, domainIndexEnd);

  apiUrl = switchStr.substring(domainIndexEnd, switchStr.length() - uidLength);

  deviceId = switchStr.substring(switchStr.length() - uidLength);
}

////////////////// WEBSOCKET ///////////////////

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    Serial.printf("[WSc] Disconnected!\n");
    break;
  case WStype_CONNECTED:
    Serial.printf("[WSc] Connected to url: %s\n", payload);
    webSocket.sendTXT("Connected"); // send message to server when Connected
    break;
  case WStype_TEXT:
    payloadStr = (char *)payload;
    payloadStr.replace(String("'"), String('"'));
    payloadStr.toLowerCase();
    Serial.println("Received data from socket: " + payloadStr);
    paid = true;
  case WStype_ERROR:
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
    break;
  }
}
