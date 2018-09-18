#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

bool connect2AP(const char* ssid, const char* password){
  bool isWIFI = false;
  WiFi.begin(ssid, password);
  Serial.print("Connecting to "); Serial.print(ssid);

  // Wait for connection
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20)
  { //wait 10 seconds
    delay(500);
    Serial.print(".");
  }
  
  if (i==21){
    Serial.println("");
    Serial.print("Could not connect to ");
    Serial.println(ssid);
  }
  else {
    isWIFI = true; 
    Serial.println("");   
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
    }
    
    return isWIFI;
}