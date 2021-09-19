#define DEBUG 1
#define USE_SOFTWARE_SERIAL 0
#define USE_MQTT_PASS 1
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif
#if USE_SOFTWARE_SERIAL == 1
#define uartSerial softSerial
#else
#define uartSerial Serial
#endif
