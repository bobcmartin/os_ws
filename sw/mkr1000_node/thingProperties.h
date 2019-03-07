#include <ArduinoIoTCloud.h>
#include <WiFiConnectionManager.h>

const char THING_ID[] = "";

const char SSID[]     = SECRET_SSID;    // Network SSID (name)
const char PASS[]     = SECRET_PASS;    // Network password (use for WPA, or use as key for WEP)


float Wind_Dir;
float Humidity;
float Temp;
float pressure_hg;
float Wind_Speed;

void initProperties(){
  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.addProperty(Wind_Dir, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(Humidity, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(Temp, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(pressure_hg, READ, 5 * SECONDS, NULL);
  ArduinoCloud.addProperty(Wind_Speed, READ, 5 * SECONDS, NULL);
}

ConnectionManager *ArduinoIoTPreferredConnection = new WiFiConnectionManager(SSID, PASS);
