#define DEBUG 1
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
// Cấu hình wifi
const char *ssid = "HuongThuy", *password = "0378521725", *ssidAP = "ESP8266WiFi";
// Cấu hình mqtt
const char *mqtt_host = "103.195.237.120", *mqtt_user = "notekunn", *mqtt_pass = "tieulinh123";
const unsigned int mqtt_port = 1883;

WiFiClient wifiClient;
PubSubClient client(wifiClient);
void setupWifi();
void reconnectMqtt();
void callback(char *topic, byte *payload, unsigned int length);
void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.println("NodeMCU is ready to connect");
    setupWifi();
    client.setServer(mqtt_host, mqtt_port);
    client.setCallback(callback);
}

void loop()
{
    // put your main code here, to run repeatedly:
    if (!client.connected())
    {
        reconnectMqtt();
    }
    client.loop();
}
void setupWifi()
{
    debug("Connecting to wifi ");
    debugln(ssid);
    debug("Waiting for connection");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        debug(".");
    }
    debugln();
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    debugln("Create access point...");
    debugln(WiFi.softAP(ssidAP) ? "Ready" : "Failed!");
    Serial.print("SoftAP IP address:");
    Serial.println(WiFi.softAPIP());
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
}
void reconnectMqtt()
{
    while (!client.connected())
    {

        String clientId = "client-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass))
        {
            Serial.println("Connected to MQTT");
            client.subscribe("mqtt/lcd");
            client.subscribe("mqtt/gate");
        }
        else
        {
            Serial.print("Error when connect mqtt:, rc=");
            Serial.println(client.state());
            Serial.println("Try again in 5 seconds");
            // Đợi 5s
            delay(5000);
        }
    }
}
void callback(char *topic, byte *pl, unsigned int length)
{
    debug("Message arrived [");
    debug(topic);
    debug("] ");
    char payload[length + 1];
    memcpy(payload, pl, length);
    payload[length] = '\0';
    debugln(payload);

    DynamicJsonDocument doc(300);
    // Lệnh in lcd
    if (strcmp("mqtt/lcd", topic) == 0)
    {
        deserializeJson(doc, payload);
        // const char *action = doc["action"];
        const char *lcd = doc["payload"]["lcd"]; // IN|OUT
        const char *lineOne = doc["payload"]["message"][0];
        const char *lineTwo = doc["payload"]["message"][1];
        Serial.print("LCD");
        Serial.print(lcd);
        Serial.print("?");
        Serial.print(lineOne);
        Serial.print("?");
        Serial.print(lineTwo);
        Serial.println();
    }
    // Lệnh mở cổng
    if (strcmp("mqtt/gate", topic) == 0)
    {
        deserializeJson(doc, payload);
        // const char *action = doc["action"];
        const char *gate = doc["payload"]; // IN|OUT
        Serial.print("OPENGATE");
        Serial.print("?");
        Serial.print(gate);
        Serial.println();
    }
}
