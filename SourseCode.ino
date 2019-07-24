#include <HCSR04.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"                      //for dht11 or dht22 or other dht sensors

/****************************pin declearation********************************/
#define ledpin 5                      //led pin
#define DHTPIN 2                      //dht22 sensor data pin
#define DHTTYPE DHT22                 //dht22 humidity and temprature sensor
#define photocellpin A0               //lDR for light intensity
/************************* Adafruit.io Setup *********************************/
//b76ad032758947d9aa0813d017e8982c
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                                      // use 8883 for SSL
#define AIO_USERNAME    "vaibhavhayaran"                          //adafruit username
#define AIO_KEY         "b76ad032758947d9aa0813d017e8982c"        //your adafruit secret key

/************ Global State (you don't need to change this!) ******************/
WiFiClient client;



Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish dht11 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/dht11");
Adafruit_MQTT_Publish dht11_temprature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/dht11_temp");
Adafruit_MQTT_Publish distance1 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/dist");
Adafruit_MQTT_Publish light = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lightintensity");
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");




/*************************** Sketch Code ************************************/
void MQTT_connect();            //connects to MQTT Broker
DHT dht(DHTPIN, DHTTYPE);       //DHT Sensor initialization
UltraSonicDistanceSensor distanceSensor(13, 12);      //distance sensor, can be used for water level in a tank
void setup() 
{
  WiFiManager wifiManager;
  wifiManager.autoConnect("HOME", "7898642001");      //default AP credentials to configure WiFi in ESP(connect to it and enter WiFi creds.)
  pinMode(ledpin,OUTPUT);
  Serial.begin(115200);
  delay(10);
  
  dht.begin();
  
  Serial.println(F("Adafruit MQTT demo"));
  Serial.println(); 
  Serial.println();
  Serial.print("Connecting to WiFi ");
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
  
  mqtt.subscribe(&onoffbutton);

  
}
void loop() 
{
  delay(10000);
  unsigned int photocellreading = analogRead(photocellpin);
  float dist = distanceSensor.measureDistanceCm();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  
  MQTT_connect();
  
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) 
  {
    if (subscription == &onoffbutton) 
    {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
      if (strcmp((char *)onoffbutton.lastread, "ON") == 0) 
      {
        digitalWrite(ledpin, HIGH); 
      }
      if (strcmp((char *)onoffbutton.lastread, "OFF") == 0) 
      {
        digitalWrite(ledpin, LOW); 
      }
    }
  }

  // Now we can publish stuff!
  Serial.print(F("\nSending humidity val "));
  Serial.print(h);
  Serial.print("...");
  if (! dht11.publish(h)) 
  {
    Serial.println(F("Failed"));
  } 
  else 
  {
    Serial.println(F("OK!"));
  }

  
  Serial.print(F("\nSending Temprature(c) val "));
  Serial.print(t);
  Serial.print("...");
  if (! dht11_temprature.publish(t))  //send temprature
  {
    Serial.println(F("Failed"));
  } 
  else 
  {
    Serial.println(F("OK!"));
  }
  
  Serial.print(F("\nSending Distance(cm) val "));
  Serial.print(dist);
  Serial.print("...");
  if (! distance1.publish(dist))  //send distance
  {
    Serial.println(F("Failed"));
  } 
  else 
  {
    Serial.println(F("OK!"));
  }


  Serial.print(F("\nSending LIGHT INTENSITY (LUX) val "));
  Serial.print(photocellreading);
  Serial.print("...");
  if (! light.publish(photocellreading))  //send distance
  {
    Serial.println(F("Failed"));
  } 
  else 
  {
    Serial.println(F("OK!"));
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}
// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.


void MQTT_connect() 
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) 
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) 
  { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) 
       {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
