#include <Arduino.h>
#include <FirebaseClient.h>
#include "ExampleFunctions.h"
#include <DHT.h>

const unsigned long pushInterval = 1000; //aktualizace hodnot každých 10s

#define WIFI_SSID "Jakub"
#define WIFI_PASSWORD "Telekom147"
#define API_KEY "xxxxx"
#define USER_EMAIL "espmesh@gmail.com"
#define USER_PASSWORD "123456789"
#define DATABASE_URL "https://esp-mesh-fd66a-default-rtdb.europe-west1.firebasedatabase.app/"
 
#define DHT_PIN 4  // Pin G4 (GPIO4)
#define DHT_TYPE DHT22
 
DHT dht(DHT_PIN, DHT_TYPE);
 
SSL_CLIENT ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD, 3000);
FirebaseApp app;
RealtimeDatabase Database;
AsyncResult databaseResult;
 
unsigned long lastPushTime = 0;
 
void pushSensorData();
 
void setup()
{
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    set_ssl_client_insecure_and_buffer(ssl_client);
    initializeApp(aClient, app, getAuth(user_auth), auth_debug_print, "🔐 authTask");
    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);
    dht.begin();
}
 
void loop()
{
    app.loop();
    if (app.ready() && millis() - lastPushTime >= pushInterval)
    {
        lastPushTime = millis();
        Serial.println("Sending data");
        pushSensorData();
    }
}
 
void pushSensorData()
{
    float temperature = dht.readTemperature(); // Teplota v °C
    float humidity = dht.readHumidity(); // Vlhkost v %
    String name;
    name = Database.set<number_t>(aClient, "/krystof/dht22/teplota", number_t(temperature, 2));
    name = Database.set<number_t>(aClient, "/krystof/dht22/vlhkost", number_t(humidity, 2));
}
