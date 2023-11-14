#ifndef AUXILIARS_HPP
#define AUXILIARS_HPP

/// @brief Blink led two times, total delay 300ms
void blinkOnboardLed();

/// @brief Parsing string of websocket
/// @param p_data string to parsing
/// @param p_separator separator of values
/// @param p_index value to return of p_data respect to p_separator
/// @return value parsed
String getValue(String p_data, char p_separator, int p_index);

#endif