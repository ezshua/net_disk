#include <Arduino.h>

bool connect2AP(const char* ssid, const char* password);
bool connect2WIFI(uint8_t count, String ssid_l[], String pass_l[]);

bool connectSaved();
bool connectSmart();
bool connectSettings(uint8_t count, String ssid_l[], String pass_l[]);
