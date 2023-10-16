#include <Arduino.h>
#include <WebSocketsClient.h>

#include "auxiliars.hpp"
#include "env.hpp"
#include "wifi.hpp"

String switchStr = ENV_SWITCH;

int uidLength = 22;  // length of lnurldevice id
String urlPrefix;    // xx://
String lnbitsServer; // xxxx.xxx.xxx
String apiUrl;       // /xxx/xx/xx/
String deviceId;     // xxxxxxxxxxxx

WebSocketsClient webSocket;

String payloadStr;
bool paid;
int pinFromSocket;
int delayFromSocket;

void setVariables();
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);

void setup() {
  Serial.begin(115200); // Start serial communication

  //// Conection to WiFi ////
  Serial.println("Conecting to WiFi...");
  conectWiFi(ENV_SSID, ENV_PASS);
  Serial.println("Connected to WiFi");

  //// Conection to websocket ///
  setVariables();
  Serial.println("Conceting to websocket: " + urlPrefix + lnbitsServer + apiUrl + deviceId);
  webSocket.beginSSL(lnbitsServer, 443, apiUrl + deviceId);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(1000);
  Serial.println("Connected to websocket");
}

void loop() {
  checkWiFi(ENV_SSID, ENV_PASS); // Check WiFi conection

  //// Check if recive pay /////
  while (paid == false) {
    webSocket.loop();
    if (paid) {
      pinFromSocket = getValue(payloadStr, '-', 0).toInt();
      delayFromSocket = getValue(payloadStr, '-', 1).toInt();

      Serial.println("Payment recived");
      pinMode(pinFromSocket, OUTPUT);
      digitalWrite(pinFromSocket, HIGH);
      delay(delayFromSocket);
      digitalWrite(pinFromSocket, LOW);
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