//v1.2
// Testovaný funkční kód

#include <Arduino.h>
#include <FirebaseClient.h>
#include "ExampleFunctions.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <FS.h>
#include <LittleFS.h>
#include <WebServer.h>

const unsigned long pushInterval = 10000; //aktualizace hodnot každých 10s

#define WIFI_SSID "Jakub"
#define WIFI_PASSWORD "fdns96deq"

#define API_KEY "xxxxx"
#define USER_EMAIL "espmesh@gmail.com"
#define USER_PASSWORD "123456789"
#define DATABASE_URL "https://esp-mesh-fd66a-default-rtdb.europe-west1.firebasedatabase.app/"

SSL_CLIENT ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
WebSocketsServer webSocket = WebSocketsServer(81);
WebServer server(80);


UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD, 3000);
FirebaseApp app;
RealtimeDatabase Database;
AsyncResult databaseResult;

Adafruit_BME280 bme;
unsigned long lastPushTime = 0;

void show_status(const String &name);
void pushSensorData();

void setup(){
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
  set_ssl_client_insecure_and_buffer(ssl_client);

  Serial.println("Initializing app...");
  initializeApp(aClient, app, getAuth(user_auth), auth_debug_print, "🔐 authTask");

  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);

    if (!LittleFS.begin()) {
    Serial.println("❌ LittleFS nepodařilo se spustit.");
    return;
  }

  server.on("/", HTTP_GET, []() {
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  server.begin();

    
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("ESP BOOTED"); 
}

float bme280_temp_get;
float bme280_pressure_get;
float bme280_humidity_get;
float bh1750_lux_get;
float dht22_temp_get;
float dht22_humidity_get;

void loop()
{
    app.loop();
    webSocket.loop();
    server.handleClient();

    if (app.ready() && millis() - lastPushTime >= pushInterval)
    {
        lastPushTime = millis();
        Serial.println("Sending data");
        SensorData_push();
        Serial.println("Getting data");
        data_get();
        
        Serial.println(bme280_temp_get);
        Serial.println(bme280_pressure_get);
        Serial.println(bme280_humidity_get);
        Serial.println(bh1750_lux_get);
        Serial.println(dht22_temp_get);
        Serial.println(dht22_humidity_get);
        
        String message = "{";
        message += "\"bme280_temp_get\":\"" + String(bme280_temp_get) + "\",";
        message += "\"bme280_humidity_get\":\"" + String(bme280_pressure_get) + "\",";
        message += "\"bme280_pressure_get\":\"" + String(bme280_humidity_get) + "\",";
        message += "\"bh1750_lux_get\":\"" + String(bh1750_lux_get) + "\",";
        message += "\"dht22_temp_get\":\"" + String(dht22_temp_get) + "\",";
        message += "\"dht22_humidity_get\":\"" + String(dht22_humidity_get) + "\"";
        message += "}";

        webSocket.broadcastTXT(message);
    }
}

void SensorData_push()
{
  float bme280_temp_push = bme.readTemperature();       // °C
  float bme280_humidity_push = bme.readHumidity();           // %
  float bme280_pressure_push = bme.readPressure() / 100.0;  // hPa

  Serial.println("📤 Data odesílám na firebase");
  String name;
  name = Database.set<number_t>(aClient, "/david/bme280/teplota", number_t(bme280_temp_push, 2));
  name = Database.set<number_t>(aClient, "/david/bme280/vlhkost", number_t(bme280_humidity_push, 2));
  name = Database.set<number_t>(aClient, "/david/bme280/tlak", number_t(bme280_pressure_push, 2));
}

void data_get()
{
  bme280_temp_get = Database.get<float>(aClient, "/david/bme280/teplota");
  bme280_humidity_get = Database.get<float>(aClient, "/david/bme280/vlhkost");
  bme280_pressure_get = Database.get<float>(aClient, "/david/bme280/tlak");
  bh1750_lux_get = Database.get<float>(aClient, "/jakub/bh1750/intenzita");
  dht22_temp_get = Database.get<float>(aClient, "/krystof/dht22/teplota");
  dht22_humidity_get = Database.get<float>(aClient, "/krystof/dht22/vlhkost");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String msg = String((char*)payload);
    if (msg == "getValues") {
      String message = "{";
      message += "\"bme280_temp_get\":\"" + String(bme280_temp_get) + "\",";
      message += "\"bme280_humidity_get\":\"" + String(bme280_pressure_get) + "\",";
      message += "\"bme280_pressure_get\":\"" + String(bme280_humidity_get / 100.0F) + " \",";
      message += "\"bh1750_lux_get\":\"" + String(bh1750_lux_get) + "\",";
      message += "\"dht22_temp_get\":\"" + String(dht22_temp_get) + "\",";
      message += "\"dht22_humidity_get\":\"" + String(dht22_humidity_get) + "\"";
      message += "}";
      webSocket.sendTXT(num, message);
    }
  }
}
