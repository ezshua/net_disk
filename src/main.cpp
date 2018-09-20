/*
  SDWebServer - Example WebServer with SD Card backend for esp8266

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WebServer library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Have a FAT Formatted SD Card connected to the SPI port of the ESP8266
  The web root is the SD Card root folder
  File extensions with more than 3 charecters are not supported by the SD Library
  File Names longer than 8 charecters will be truncated by the SD library, so keep filenames shorter
  index.htm is the default index (works on subfolders as well)

  upload the contents of SdRoot to the root of the SDcard and access the editor by going to http://esp8266sd.local/edit

*/
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
// #include <ESP8266mDNS.h>
//#include <SPI.h>

#include "connect.h"
#include "settings.h"

//#include <SD.h>
#include <SdFat.h>
SdFat SD;

#include "utf8rus.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET D4
Adafruit_SSD1306 oled(OLED_RESET);

const char* ssid = "Rizikon";
const char* password = "02112012nov";
const char* ssid2 = "HomeDotNew";
const char* password2 = "asusnotik";
const char* host = "esp8266sd";

ESP8266WebServer server(80);

static bool hasSD = false;
File uploadFile;

long countFiles=0;
long calcUsedSpaceKB(String basepath) {
  if (!hasSD) return 0;
  
    File dir = SD.open((char *)basepath.c_str());
  if (!dir){
    Serial.print(basepath.c_str());Serial.println(" not found!");
    return -1;
  }
  String path = String();
  if(!dir.isDirectory()){
    dir.close();
    return dir.fileSize()/1024;
  }

  dir.rewindDirectory();
  u_long usedspace = 0; // начинаем подсчет размера занимаемого файлами с нуля  
  
  for (int cnt = 0; true; ++cnt) {
    //File entry = dir.openNextFile();
    FatFile entry;
    if (!entry.openNext(&dir, O_READ))
      break;

    if (entry.isDir()){
      char newDir[128];
      entry.getName(newDir, 128);      
      usedspace += calcUsedSpaceKB(basepath+newDir+"/");
    }
    usedspace += entry.fileSize()/1024; // добавляем размер текущего файла
    countFiles++; // добавляем текущий файл
    entry.close();
 } 
 dir.close();
 return usedspace;
}

u_long calcFreeSpaceKB(){
  if (!hasSD) return 0;
  
  FatVolume *sd_vol = SD.vol();
    // Calculate free space (volume free clusters * blocks per clusters / 2)
  u_long freeSpaceKB = sd_vol->freeClusterCount();
  freeSpaceKB *= sd_vol->blocksPerCluster()/2;
  
  return freeSpaceKB;
}

int filesInfoX = 0;
int filesInfoY = 0;
void displayFilesInfo(){
  // стираем надпись про проверку карты
  // oled.setCursor(filesInfoX, filesInfoY);
  // oled.setTextColor(WHITE, BLACK);
  // oled.println("                                                                       ");// примитивно затираем строку
  // oled.display();

  // устанавливаем в позицию курсор
  oled.setCursor(filesInfoX, filesInfoY);
  oled.setTextColor(WHITE, BLACK);

  countFiles = 0; // обнуляем счетчик файлов
  long uspace = calcUsedSpaceKB("/");  
  if (uspace == -1)
  {
    Serial.println("SDCard empty!");
    // стираем надпись про проверку карты
    oled.println("                                                                       ");// примитивно затираем строку
    //oled.display();
    oled.println("SDCard empty!                                                          ");
    oled.display();
    return;
  }
  
  u_long fspace = calcFreeSpaceKB();
  Serial.print("Files: ");
  Serial.print(countFiles);
  Serial.print(", used: ");
  Serial.print(uspace/1024);
  Serial.print(" MB, free: ");
  Serial.print(fspace/1024);
  Serial.println(" MB");
  //double upers = 100*uspace/(fspace+uspace);
  //double fpers = 100*fspace/(fspace+uspace);
  // стираем надпись про проверку карты
  oled.println("                    ");// примитивно затираем строку
  oled.println("                    ");// примитивно затираем строку
  oled.println("                    ");// примитивно затираем строку
  oled.println("                    ");// примитивно затираем строку
  oled.display();
  // устанавливаем в позицию курсор
  oled.setCursor(filesInfoX, filesInfoY);
  oled.setTextColor(WHITE, BLACK);
  oled.print("Files: ");  oled.println(countFiles);
  oled.print("Used:  ");  oled.print(uspace);  oled.println(" KB"); //");oled.print(upers);oled.println(" %)");
  oled.print("Free:  ");  oled.print(fspace);  oled.println(" KB"); //");oled.print(fpers);oled.println(" %)");
  oled.display();
}


void returnOK() {
  server.send(200, "text/plain", "");
}

void returnFail(String msg) {
  server.send(500, "text/plain", msg + "\r\n");
}

bool loadFromSdCard(String path){
  digitalWrite(LED_BUILTIN, 0); // подсвечиваем активность
  Serial.print("From: "); Serial.println(path);
  String dataType = "text/plain";
  //if(path.endsWith("/")) path += "index.htm";
  //if (path.startsWith("/") && path.endsWith("/")) path += "index.htm";
  if(path == "/index.htm") path = "/readme.md";
  if(path == "/") path += "index.htm";
      
  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";
  else if(path.endsWith(".nc")) dataType = "text/plain";
  else if(path.endsWith(".gcode")) dataType = "text/plain";

  File dataFile = SD.open(path.c_str());
  if (!dataFile){
    Serial.print(path.c_str());Serial.println(" not found!");
    digitalWrite(LED_BUILTIN, 1); // подсвечиваем активность
    return false;
  }

  if(dataFile.isDirectory()){
    path += "/readme.txt";
    dataType = "text/plain";
    dataFile = SD.open(path.c_str());
  }

  if (!dataFile){
    Serial.print(path.c_str());Serial.println(" not found!");
    digitalWrite(LED_BUILTIN, 1); // подсвечиваем активность
    return false;
  }

  if (server.hasArg("download")) dataType = "application/octet-stream";

  char name[128];
  dataFile.getName(name, 128);
  Serial.print("Send: ");Serial.print(name);Serial.print(" ");
  Serial.print(dataFile.size());Serial.print(" bytes, type: ");Serial.println(dataType);
  uint32_t realSize = server.streamFile(dataFile, dataType);
  if ((realSize) != dataFile.size()) {
    Serial.println("Sent less data than expected!");
    Serial.print("Full size: "); Serial.println(dataFile.size());
    Serial.print("Real size: "); Serial.println(realSize);
  }

  dataFile.close();
  digitalWrite(LED_BUILTIN, 1); // подсвечиваем активность
  return true;
}

int startM = 0;
int pers = 0;
int statusX = 0;
int statusY = 0;
void handleFileUpload(){
  if(server.uri() != "/edit") return;  
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    digitalWrite(LED_BUILTIN, 0); // подсвечиваем активность
    if(SD.exists((char *)upload.filename.c_str())) SD.remove((char *)upload.filename.c_str());
    uploadFile = SD.open(upload.filename.c_str(), FILE_WRITE);
    Serial.print("Upload: START, filename: "); Serial.println(upload.filename);
    oled.setCursor(statusX, statusY); 
    oled.setTextColor(WHITE,BLACK); 
    oled.println("                                                                                                               ");// примитивно затираем строку
    oled.setCursor(statusX, statusY); 
    oled.setTextColor(WHITE,BLACK); 
    oled.print("Up: "); oled.println(utf8rus(upload.filename)); oled.display();
    startM = millis();    
    pers = 0;
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(uploadFile) uploadFile.write(upload.buf, upload.currentSize);
    pers += upload.currentSize;
    //Serial.print("Upload: WRITE+");Serial.print(upload.currentSize); Serial.print(" = ");Serial.print(upload.totalSize); Serial.println(" bytes");
    // oled.setCursor(statusX, statusY); 
    // oled.setTextColor(WHITE,BLACK); 
    // oled.print("Upload: "); oled.print(upload.totalSize); oled.println(" B"); oled.display();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // подсвечиваем активность
  } else if(upload.status == UPLOAD_FILE_END){    
    if(uploadFile) uploadFile.close();
    int interval = millis() - startM;
    int speed = upload.totalSize / interval;
    Serial.print("Upload: END, Size: "); Serial.print(upload.totalSize); Serial.print(" bytes, Interval: ");
    Serial.print(interval);Serial.print(" msec, Speed: ");
    Serial.print(speed);Serial.println(" kbytes/sec");
    oled.setCursor(statusX, statusY); 
    oled.setTextColor(WHITE,BLACK);
    oled.println("                                                                                                               ");// примитивно затираем строку
    oled.setCursor(statusX, statusY); 
    oled.setTextColor(WHITE,BLACK); 
    oled.print("Upl: "); oled.print(upload.totalSize/1024); oled.println(" KB");
    oled.print("Spd:"); oled.print(speed); oled.println(" KB/sec");
    oled.display();
        // Serial.print("Upload: END, Interval: ");Serial.print(interval);Serial.println(" msec");
        // Serial.print("Upload: END, Speed: ");Serial.print(speed);Serial.println(" kbytes/sec");
    
    displayFilesInfo();
    digitalWrite(LED_BUILTIN, 1); // подсвечиваем активность
  }
}

void deleteRecursive(String path){
  File file = SD.open((char *)path.c_str());
  if(!file.isDirectory()){
    file.close();
    SD.remove((char *)path.c_str());
    return;
  }

  file.rewindDirectory();
  while(true) {
    File entry = file.openNextFile();
    if (!entry) break;
    String entryPath = path + "/" +entry.name();
    if(entry.isDirectory()){
      entry.close();
      deleteRecursive(entryPath);
    } else {
      entry.close();
      SD.remove((char *)entryPath.c_str());
    }
    yield();
  }

  SD.rmdir((char *)path.c_str());
  file.close();
}

void handleDelete(){
  if(server.args() == 0) return returnFail("BAD ARGS");
  String path = server.arg(0);
  if(path == "/" || !SD.exists((char *)path.c_str())) {
    returnFail("BAD PATH");
    return;
  }
  deleteRecursive(path);
  returnOK();
}

void handleCreate(){
  if(server.args() == 0) return returnFail("BAD ARGS");
  String path = server.arg(0);
  if(path == "/" || SD.exists((char *)path.c_str())) {
    returnFail("BAD PATH");
    return;
  }

  if(path.indexOf('.') > 0){
    File file = SD.open((char *)path.c_str(), FILE_WRITE);
    if(file){
      file.write((const char *)0);
      file.close();
    }
  } else {
    SD.mkdir((char *)path.c_str());
  }
  returnOK();
}

void printDirectory() {
  if (!hasSD) return;
  if(!server.hasArg("dir")) return returnFail("BAD ARGS");
  String path = server.arg("dir");
  if(path != "/" && !SD.exists((char *)path.c_str())) return returnFail("BAD PATH");
  File dir = SD.open((char *)path.c_str());
  path = String();
  if(!dir.isDirectory()){
    dir.close();
    return returnFail("NOT DIR");
  }
  
  digitalWrite(LED_BUILTIN, 0); // подсвечиваем активность
  dir.rewindDirectory();
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/json", "");
  WiFiClient client = server.client();
  
  server.sendContent("[");
  for (int cnt = 0; true; ++cnt) {
    //File entry = dir.openNextFile();
    FatFile entry;
    char newname[128];
    if (!entry.openNext(&dir, O_READ))
      break;
    
    entry.getName(newname, 128);
    String newnameS = newname;
    if (newnameS.endsWith(".md"))
      break; // не показываем служебные файлы
    
    String output;
    if (cnt > 0)
      output = ',';

    output += "{\"type\":\"";
    output += (entry.isDir()) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += newname;
    output += "\"";
    output += "}";
    //Serial.println("-- "+output);
    server.sendContent(output);
    entry.close();    
 }
 server.sendContent("]");
 dir.close();
 digitalWrite(LED_BUILTIN, 1); // подсвечиваем активность
}



void handleNotFound(){
  //Serial.print("handleNotFound!");
  if(hasSD && loadFromSdCard(server.uri())) return;
  String message = "SDCARD Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.print(message);
}

// void pf(SdBaseFile s, char subdirs[][50], ArduinoOutStream outp, char dirnr = -1) {
//   SdFile file;
//   char name[20];                            // 8.3 format
//   while (1) {
//     if (!file.openNext(&s, O_READ)) break;  // no file - end
//     bool z = file.isDir();
//     file.getName(name, 20);                 // File/directory name
//     uint32_t fsize = file.fileSize();       // filesize
//     file.close();                           // closing bevore open the next one
//     if (z) {                                // add subdir and run new list
//       memcpy(subdirs[dirnr + 1], name, strlen(name) + 1);
//       SdBaseFile n;                         // new basefile for next directory
//       uint16_t index = s.curPosition() / 32 - 1; //index of current directory
//       n.open(&s, index, O_READ);            // open the directory
//       //pf(n, subdirs, outp, dirnr + 1);           // list files/directorys in this dir
//     }
//     else {                                  // print filename
//       for (byte i = 0; i <= dirnr; i++)     // print all subdirectorys
//         outp << (subdirs[i]) << '\\';
//       outp << name << ' ' << fsize << "\n";
//     }
//   }
// }

// void printSDroot(ArduinoOutStream cout) {
//   SdBaseFile s;
//   char dircount[5][50];                     // max 5 subdirs
//   s.openRoot(SD.vol());                     // open root dir
//   pf(s, dircount, cout);
// }

void setup(void){
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  Serial.begin(115200);
  delay(1000);
  //Serial.setDebugOutput(true);
  Serial.print("\n");
  
  oled.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);  // initialize with the I2C addr 0x3C (for the 128x64)
  oled.cp437(true);
  oled.clearDisplay();
  oled.display();

  oled.setCursor(0,0);
  //oled.setTextSize(2);
  oled.setTextColor(WHITE);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // подсвечиваем активность
  bool loadSett = false;
  if (SD.begin(SS, SPI_HALF_SPEED)){
     Serial.println("SDCard initialized");
     hasSD = true;
     //SD.initErrorHalt();
     loadSett = LoadSettingFile(SD, "/settings.md");
  }    
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // подсвечиваем активность
  // старая версия подключения, для пары точек с фиксированными в коде параметрами
  if (!loadSett){
    bool noWiFi = true;
    Serial.println("Connecting to WiFi...");
    while (noWiFi) {
      noWiFi = !connect2AP(ssid, password);
      if (noWiFi) {
        // oled.print("Could not connect to ");    
        // oled.println(ssid);
        // oled.display();

        noWiFi = !connect2AP(ssid2, password2);
        // oled.print("Could not connect to ");    
        // oled.println(ssid2);
        // oled.display();
      }
      if (noWiFi) {
        oled.clearDisplay();
        oled.display();
        delay(3000);
      }
    }
  }
  else{
    bool isWiFi = true;
    do{
    for(int i = 0; i < 5; i++)
    {
      if (ssid_list[i] == "") continue;

      isWiFi = connect2AP(ssid_list[i].c_str(), password_list[i].c_str());
      if (isWiFi) break;
    }
    if (!isWiFi){
        Serial.println("Could not connect to any AP!");
        oled.println("NO WIFI!");
        oled.display();
        // возможно тут надо переключиться в режим АР
        //while (1) delay(100); // зависаем!
    }
    } while (!isWiFi);
  }
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // подсвечиваем активность
  oled.print("SSID: "); oled.println(ssid);
  oled.print("IP:   "); oled.println(WiFi.localIP());
  oled.display();  

  server.on("/list", HTTP_GET, printDirectory);
  server.on("/edit", HTTP_DELETE, handleDelete);
  server.on("/edit", HTTP_PUT, handleCreate);
  server.on("/edit", HTTP_POST, [](){ returnOK(); }, handleFileUpload);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // подсвечиваем активность
  if (hasSD){    
    oled.println("SDCard init!"); oled.display();
    filesInfoX = oled.getCursorX();
    filesInfoY = oled.getCursorY();
    oled.println("Checking card...");
    oled.display();
    //oled.setCursor(filesInfoX, filesInfoY);    
    displayFilesInfo();          
    statusX = oled.getCursorX();
    statusY = oled.getCursorY();
  }
  else
  {
    Serial.println("No SDCard initialized.");
    oled.println("No SDCard"); oled.display();
  }
  
  digitalWrite(LED_BUILTIN, 1);
}

void loop(void){
  server.handleClient();  
}



