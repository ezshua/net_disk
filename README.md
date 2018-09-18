# net_disk

Простая реализация WiFi-SDCard на esp8266 для использования в 3D принтерах. 
Позволяет не "дергать" карту для загрузки новых заданий.
Управление питанием этого модуля осуществляется через сигнал CS основной карты - при начале 
работы принтера с картой памяти модуль WiFi доступа отключается. 
Конкурентный доступ решается организационными мерами! 

Original: https://github.com/shuzonudas/portable-disk-drive
The code is in the esp8266-arduino examples. 
I have kept it here to make it consistent with my video.
https://www.youtube.com/watch?v=zJP3Ie3nE7c


Modding by ezshua:

v0.3
- добавлено считывание конфигурационного файла "settings.md"
- примерное содержимое файла настроек (лишние пробелы не допускаются, пустые строки допускаются):
  APName:AccessPointName - "AccessPointName" - имя точки доступа (регистр обязателен)
  APPass:AccessPointPass - "AccessPointPass" - пароль точки доступа (регистр обязателен)
  APName и APPass - служебные теги (регистр обязателен)
  Таких пар может быть несколько (не более 5), подключение к точкам по порядку, 
  при неработающей предыдущей паре параметров.
  
v0.2
- Добавлен вывод информации о количестве файлов, занятом и свободном месте на экран
- Русские имена файлов не отображаются!

v0.1
- Переделана поддержка sd карты - переведено на поддержку библиотеки SdFat (длинные имена файлов)
- Для экономии памяти длинна пути с именем файла ограничена в 128 символов!
- Добавлена поддержка oled дисплея на контроллере SSD1306 с поддержкой русских символов