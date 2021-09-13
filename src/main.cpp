#include <debug.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SerialCommand.h>
#include <SoftwareSerial.h>
// Cấu hình wifi
const char *ssid = "HuongThuy", *password = "0378521725", *ssidAP = "ESP8266WiFi";
// Cấu hình mqtt
const char *mqtt_host = "103.195.237.120", *mqtt_user = "notekunn", *mqtt_pass = "tieulinh123";
const unsigned int mqtt_port = 1883;
const byte RX = D1;
const byte TX = D2;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
SerialCommand sCmd;
SoftwareSerial softSerial(RX, TX);
void setupWifi();
void reconnectMqtt();
void callback(char *topic, byte *payload, unsigned int length);
void onScanRFID();
void onIRChange();
void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.println("NodeMCU is ready to connect");
    setupWifi();
    client.setServer(mqtt_host, mqtt_port);
    client.setCallback(callback);
    sCmd.begin(uartSerial);
    sCmd.addCommand((char *)"SCAN", NULL, onScanRFID, NULL, NULL);
    sCmd.addCommand((char *)"IR", NULL, onIRChange, NULL, NULL);
}

void loop()
{
    // put your main code here, to run repeatedly:
    if (!client.connected())
    {
        reconnectMqtt();
    }
    client.loop();
    sCmd.loop();
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
    Serial.print("SoftAP IP address: ");
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
        uartSerial.print("LCD");
        uartSerial.print(lcd);
        uartSerial.print("?");
        uartSerial.print(lineOne);
        uartSerial.print("?");
        uartSerial.print(lineTwo);
        uartSerial.println();
    }
    // Lệnh mở cổng
    if (strcmp("mqtt/gate", topic) == 0)
    {
        deserializeJson(doc, payload);
        // const char *action = doc["action"];
        const char *gate = doc["payload"]; // IN|OUT
        uartSerial.print("OPENGATE");
        uartSerial.print("?");
        uartSerial.print(gate);
        uartSerial.println();
    }
}
void onScanRFID()
{
    char *uuid = sCmd.next();
    debug("SCAN ");
    debugln(uuid);
    if (uuid != NULL)
    {
        DynamicJsonDocument doc(100);
        String json;
        doc["action"] = "read";
        doc["payload"] = uuid;
        serializeJson(doc, json);
        debugln(json);
        client.publish("mqtt/scan", json.c_str());
    }
}

void onIRChange()
{
    char *args;
    debug("IR ");
    args = sCmd.next();
    debug(args);
    if (args == NULL)
        return;
    int id = atoi(args);
    args = sCmd.next();
    debug(" ");
    debugln(args);
    if (args == NULL)
        return;
    int value = atoi(args);
    DynamicJsonDocument doc(100);
    String json;
    doc["action"] = "change";
    doc["payload"]["id"] = id;
    doc["payload"]["serving"] = value == 1;
    serializeJson(doc, json);
    debugln(json);
    client.publish("mqtt/ir", json.c_str());
}