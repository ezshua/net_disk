#include <Arduino.h>
#include <SdFat.h>
//#include <WString.h>

String ssid_list[5];
bool havessid = false;
String password_list[5];
bool havepass = false;

bool LoadSettingFile(SdFat SD, char * name){
  bool result = true;

  File file = SD.open(name, O_READ);
  if (!file) return false;
  String str, str2;
  //int i = 0;
  int ap_index = 0;
  while(file.available()>0){
    // str = file.readString();
    str = file.readStringUntil('\n'); // читать до "перевод строки"
    //Serial.print(i++); Serial.print(": ");Serial.println(str);
              
    //название точки доступа
    if (str.startsWith("APName:")){
      if (havessid && !havepass){
        // найдена новая АР, у предыдущей нет пароля
        password_list[ap_index] = "";
        ap_index++;
        if (ap_index > 4) {
          Serial.println("Превышено количество AP в файле конфигурации settings.md");
          file.close();
          return false;
        }
        havessid = false;
        havepass = false;
      }
      if (str.indexOf(0xd) > -1)
        ssid_list[ap_index] = str.substring(str.indexOf(':')+1, str.indexOf(0xd)); // отрезаем "возврат каретки"
      else
        ssid_list[ap_index] = str.substring(str.indexOf(':')+1);
      //Serial.println(ssid_list[ap_index]); 
      havessid = true;
      
    }

    //пароль точки доступа
    if (str.startsWith("APPass:")){
      if (!havessid && havepass){
        // лишний пароль, выбрасываем
        password_list[ap_index] = "";
        havessid = false;
        havepass = false;
      }
      if (str.indexOf(0xd) > -1)
        password_list[ap_index] = str.substring(str.indexOf(':')+1, str.indexOf(0xd)); // отрезаем "возврат каретки"
      else
        password_list[ap_index] = str.substring(str.indexOf(':')+1);
      //Serial.println(password_list[ap_index]); 
      havepass = true;
    }

    // переходим к следующей точке доступа
    if (havessid && havepass){
      
      ap_index++;
      if (ap_index > 4) {
        Serial.println("Превышено количество AP в файле конфигурации settings.md");
        file.close();
        return false;
        }
      havepass = false;
      havessid = false;
    }
    // продолжаем проверять другие настройки
    /* code */
  }
  file.close();
  Serial.print("Load ");Serial.print(ap_index);Serial.println(" AP setting");
  return result;
}
