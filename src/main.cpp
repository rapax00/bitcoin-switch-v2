#include "env.hpp"
#include <Arduino.h>
#include <SPIFFS.h>
#include <WebSocketsClient.h>
#include <WiFi.h>

////////////////////////////////// variables /////////////////////////////////

String ssid = ENV_SSID;
String pass = ENV_PASS;
String switchStr = ENV_SWITCH;

////////////////////////////// END of variables //////////////////////////////

int uidLength = 22;  // length of lnurldevice id
String urlPrefix;    // xx://
String lnbitsServer; // xxxx.xxx.xxx
String apiUrl;       // /xxx/xx/xx/
String deviceId;     // xxxxxxxxxxxx

String payloadStr;
bool paid;
int pinFromSocket;
int delayFromSocket;

WebSocketsClient webSocket;

void setVariables();
void blinkOnboardLed();
String getValue(String data, char separator, int index);
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);

void setup() {
  Serial.begin(115200);         // Start serial communication
  pinMode(LED_BUILTIN, OUTPUT); // To blink onboard LED for status

  //// Conection to WiFi ////
  Serial.print("Conecting to WiFi");
  WiFi.begin(ssid.c_str(), pass.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    blinkOnboardLed();
    delay(700);
  }
  Serial.println("");
  Serial.println("Connected to WiFi: " + ssid);

  //// Conection to websocket ///
  setVariables();
  Serial.println("Conceting to websocket: " + urlPrefix + lnbitsServer + apiUrl + deviceId);
  webSocket.beginSSL(lnbitsServer, 443, apiUrl + deviceId);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(1000);
}

void loop() {
  //// Check WiFi conection ////
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Reconecting to WiFi");
    WiFi.begin(ssid.c_str(), pass.c_str());
    blinkOnboardLed();
    Serial.print(".");
    delay(200);
  }

  //// Check if recive pay /////
  while (paid == false) {
    webSocket.loop();
    if (paid) {
      Serial.println("Payment recived");
      pinFromSocket = getValue(payloadStr, '-', 0).toInt();
      delayFromSocket = getValue(payloadStr, '-', 1).toInt();

      pinMode(pinFromSocket, OUTPUT);
      digitalWrite(pinFromSocket, HIGH);
      delay(delayFromSocket);
      digitalWrite(pinFromSocket, LOW);
    }
  }

  paid = false;
}

/////////////////// HELPERS ////////////////////

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

void blinkOnboardLed() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
}

//// parset ////
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
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