#include <Arduino.h>
#include <FirebaseClient.h>
#include "ExampleFunctions.h"
#include <Wire.h>
#include <BH1750.h>

const unsigned long pushInterval = 1000; //aktualizace hodnot každých 10s

#define WIFI_SSID "Jakub"
#define WIFI_PASSWORD "fdns96deq"
#define API_KEY "AIzaSyApNau0_yxCJcKexPzC5M8e_TGolCGJf7Y"
#define USER_EMAIL "espmesh@gmail.com"
#define USER_PASSWORD "123456789"
#define DATABASE_URL "https://esp-mesh-fd66a-default-rtdb.europe-west1.firebasedatabase.app/"
 
SSL_CLIENT ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD, 3000);
FirebaseApp app;
RealtimeDatabase Database;
AsyncResult databaseResult;
 
BH1750 luxSenzor;
unsigned long lastPushTime = 0;
 
void pushSensorData();
void getSensorData();
 
void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  set_ssl_client_insecure_and_buffer(ssl_client);
  initializeApp(aClient, app, getAuth(user_auth), auth_debug_print, "🔐 authTask");
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);
  Wire.begin();
  luxSenzor.begin();
}
 
void loop()
{
  app.loop();
  if (app.ready() && millis() - lastPushTime >= pushInterval)
  {
    lastPushTime = millis();
    pushSensorData();
    getSensorData();
  }
}
 
void pushSensorData()
{
  float lux = luxSenzor.readLightLevel();
  String name;
  name = Database.set<number_t>(aClient, "/jakub/bh1750/intenzita", number_t(lux, 2));
}
 
void getSensorData()
{
  int value1 = Database.get<int>(aClient, "/jakub/bh1750/intenzita");
  Serial.println(value1);
}