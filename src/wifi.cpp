#include <WiFi.h>

#include "auxiliars.hpp" // for blinkOnboardLed
#include "wifi.hpp"

const char *ssid;
const char *pass;

bool conectWiFi(const char *p_ssid, const char *p_pass) {
  ssid = p_ssid;
  pass = p_pass;

  int seconds = 0;

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    blinkOnboardLed();
    delay(700);
    seconds++;
  }

  return (WiFi.status() == WL_CONNECTED) ? true : false;
}

bool checkWiFi() {
  int seconds = 0;

  while (WiFi.status() != WL_CONNECTED && seconds < 10) {
    WiFi.begin(ssid, pass);
    blinkOnboardLed();
    delay(700);
    seconds++;
  }

  return (WiFi.status() == WL_CONNECTED) ? true : false;
}