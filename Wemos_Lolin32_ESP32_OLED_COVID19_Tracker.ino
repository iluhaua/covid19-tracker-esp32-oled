/*
 *  Sketch for Wemos Lolin32 board with oled display to display actual data.
 * 
 *  This sketch sends data via HTTP GET requests to 
 *  coronavirus-19-api.herokuapp.com service.
 *
 *  You need to set ssid and password according to your wifi network.
 *  
 *  Also set desired country code when calling displayCountry method.
 *  Available country codes could be found here:
 *  http://coronavirus-19-api.herokuapp.com/tabs/tab2
 *  
 *  Created by Illia Sopov
 *  05 april 2020
 *  
 *  Could be freely used.
 *
 */

#include <WiFi.h>
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
#define ARDUINOJSON_DECODE_UNICODE 1
#include <ArduinoJson.h>

SSD1306Wire display(0x3c, 5, 4);

const char* ssid     = "<your ssid here>";
const char* password = "<your ssid password here>";

const char* host = "coronavirus-19-api.herokuapp.com";

void setup()
{
    Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network

    display.init();

    display.flipScreenVertically();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);

    Serial.println();
    Serial.println();
    display.drawString(0, 0, "Connecting to "+ String(ssid));

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        display.drawString(0, 11, ".");
        display.display();
    }

    Serial.println("");
    display.drawString(0, 22, "WiFi connected");
    display.drawString(0, 33, "IP address: " + String(WiFi.localIP()));
    display.display();
    delay(1000);
}

void displayCountry(String countryUrl)
{
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        display.clear();
        display.drawString(0, 11, "connection failed");
        Serial.println("connection failed");
        display.display();
        return;
    }
    
  // We now create a URI for the request
    String url = countryUrl;

    Serial.print("Requesting URL: ");
    Serial.println(url);

    if(!client.connected()) {
      Serial.print("Not connected");
      client.stop();
      return;
    }

    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            display.clear();
            display.drawString(0, 22, ">>> Client Timeout !");
            display.display();
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }
    
    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
      Serial.println(F("Invalid response"));
      return;
    }
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        display.clear();
        String line = client.readStringUntil('\r');
        Serial.println("json string size: " + String(line.length()));
        if(line.length() > 1) {
          StaticJsonDocument<500> doc;
          deserializeJson(doc, line);
          
          const char* country = doc["country"]; // "Ukraine"
          int cases = doc["cases"]; 
          int todayCases = doc["todayCases"]; 
          int deaths = doc["deaths"]; 
          int todayDeaths = doc["todayDeaths"]; 
          int recovered = doc["recovered"]; 
          int active = doc["active"]; 
          int critical = doc["critical"]; 
          //int casesPerOneMillion = doc["casesPerOneMillion"]; 
          //int deathsPerOneMillion = doc["deathsPerOneMillion"]; 
          //int totalTests = doc["totalTests"]; 
          //int testsPerOneMillion = doc["testsPerOneMillion"];
          display.setFont(ArialMT_Plain_10);
          display.drawString(0, 0, "COVID19." + String(country) + ": " + String(cases));
          //display.drawString(0, 0, "COVID19 " + String(country));
          //display.drawString(0,11, "Cases: " + String(cases));
          display.drawString(0,11, "New: " + String(todayCases));
          //display.drawString(0,11, "New: " + String(todayCases) + " |Rec: " + String(recovered)+" |Crit: "+String(critical));
          display.drawString(0,22, "Rec: " + String(recovered)+" |Crit: "+String(critical));
          display.drawString(0,33, "Active: " + String(active));
          display.drawString(0,44, "Dths: " + String(deaths)+ " |Tdy: " + String(todayDeaths));
          //display.drawString(0,28, "Today: " + String(todayCases));
          //display.drawString(0,39, "Active: " + String(active));
          Serial.println(line);
          Serial.println(country);
          Serial.println("Cases: " + String(cases));
          display.display();
        }
    }

    Serial.println();
    Serial.println("closing connection");
    client.stop();
}


void loop()
{
    displayCountry("/countries/Ukraine");
    delay(5000);
    displayCountry("/countries/World");
    delay(5000);
}
