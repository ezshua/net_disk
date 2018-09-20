#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

bool connect2AP(const char *ssid, const char *password)
{
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid);

  // Wait for connection
  uint8_t counter = 0;
  while (WiFi.status() != WL_CONNECTED)
  { //wait 30 seconds
    delay(1000);
    Serial.print(".");
    counter++;
    if (counter > 30)
    {
      Serial.println("Bad!");
      Serial.print("Could not connect to ");
      Serial.println(ssid);
      return false;
    }
  }
  Serial.println("Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

bool connect2WIFI(uint8_t count, String ssid_l[], String pass_l[])
{
  //WiFi.disconnect(); // на случай сброса конфигурации
  WiFi.mode(WIFI_STA);
  delay(500);
  uint8_t counter = 0;

  // пробуем сохраненный сеанс
  if (WiFi.SSID() != "")
  {
    Serial.println(String("Using a saved SSID: ") + WiFi.SSID());
    Serial.print("Connecting");
    WiFi.begin();
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
      Serial.print(".");
      counter++;
      if (counter > 30)
      {
        Serial.println("Bad!");
        break;
      }
    }
    if (counter <= 30)
    {
      Serial.println("Ok");
      Serial.println(String("SSID: ") + WiFi.SSID());
      Serial.println(String("IP: ") + WiFi.localIP().toString());
      return true;
    }
  }
  else
  {
    Serial.println("No saved SSID");
  }

  // пробуем использовать SmartConfig
  counter = 0;
  Serial.print("SmartConfig");
  WiFi.beginSmartConfig();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
    counter++;
    if (WiFi.smartConfigDone())
    {
      Serial.println("Done");
      WiFi.stopSmartConfig();
      delay(2000);
      Serial.println(String("SSID: ") + WiFi.SSID());
      Serial.println(String("IP: ") + WiFi.localIP().toString());
      return true;
      //break;
    }
    // не смогли получить конфигурацию и подключиться к сети за 30 секунд
    if (counter > 30)
    {
      Serial.println("Bad!");
      WiFi.stopSmartConfig();
      break;
    }
  }

  // пробуем подключиться по файлу конфигурации
  if (count > 0)
    for (int i = 0; i < count; i++)
    {
      if (connect2AP(ssid_l[i].c_str(), pass_l[i].c_str()))
        return true;
    }

  Serial.println("Could not connect to any AP!");
  // возможно тут надо переключиться в режим АР
  //while (1) delay(100); // зависаем!
  return false;
}