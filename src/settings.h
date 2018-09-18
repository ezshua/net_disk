#include <Arduino.h>
#include <SdFat.h>

extern String ssid_list[5];
extern String password_list[5];
bool LoadSettingFile(SdFat SD, char * name);
