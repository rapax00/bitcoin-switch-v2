#ifndef WIFI_HPP
#define WIFI_HPP

/// @brief Connect WiFi
/// @param p_ssid ssid of WiFi
/// @param p_pass password of WiFi
/// @return 1/true if WiFi is conected before 10 seconds, else 0/false
bool conectWiFi(const char *p_ssid, const char *p_pass);

/// @brief Check if WiFi continue conected, else reconnect
/// @return 1/true if WiFi is conected or reconnected before 10 second, else
/// @return 0/false
bool checkWiFi();

#endif
