#include "ClosedCube_HDC1080.h"
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include "RTClib.h" 
#include "Wire.h"

ClosedCube_HDC1080 hdc1080; // temp
RTC_DS3231 rtc; // rtc

String ssid = "J.Teller"; // wifi ssid
String password = "j.tellller"; // wifi pass
String server_url = "http://192.168.50.6/status"; // send data endpoint

int lastMinutes = 0;

void setup () {
  pinMode(2, OUTPUT); // led isWorking

  // Serial.begin(9600); // uncomment for debug
  Wire.begin(4, 5); // init I2C

  hdc1080.begin(0x40); // init temp
  rtc.begin(); // init rtc
  
//  if (rtc.lostPower()) {
    // rtc, set time if unset
    //  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//  }

  // connect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(1000);
    ESP.restart();
  }

  // setup remote flash
  ArduinoOTA.setHostname("ArduinoMonitoring");
  ArduinoOTA.begin(); 
}

void loop () {
  ArduinoOTA.handle(); // handle remote flash

  // led isWorking
  digitalWrite(2, HIGH);
  delay(500);                   
  digitalWrite(2, LOW);
  delay(500);

  // get current minutes
  DateTime now = rtc.now();
  int Minutes = now.minute();

  // main logic to send data to server (in 0/15/30/45 minutes of each hour)
  // if (Minutes == 0 || Minutes == 15 || Minutes == 30 || Minutes == 45) {
    // if (Minutes != lastMinutes) {
      if(WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;

        // init http
        http.begin(client, server_url);
        http.addHeader("Content-Type", "application/json");

        // collect data
        StaticJsonDocument<200> obj;
        obj["temperature"] = hdc1080.readTemperature();
        delay(20);
        obj["humidity"] = hdc1080.readHumidity();
        delay(20);
        obj["timestamp_board"] = now.timestamp(DateTime::TIMESTAMP_FULL);

        // convert data to json
        String requestBody;
        serializeJson(obj, requestBody);

        // send request
        http.POST(requestBody);
        http.end();
      }
      // lastMinutes = Minutes;
    // }    
  // }
}
