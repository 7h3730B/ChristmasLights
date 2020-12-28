#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

const char *ssid = "Wlan-Repeater-Schulze";
const char *password = "1cVcMBsgUt2ifR5lTdpXe6Td3";

ESP8266WebServer server(80);

const uint16_t kIrLed = 4; // ESP8266 D2

IRsend irsend(kIrLed);

boolean connectWifi();
void handleNotFound();

void setup() {
    Serial.begin(115200);
    connectWifi();
    
    irsend.begin();

    server.on("/api", [](){
        server.send(200, "text/plain", "OK");
        uint16_t raw_data[20];
        for (int i = 0; i < server.args(); i++) {
            raw_data[i] = server.arg(i).toInt();
        }
        irsend.sendRaw(raw_data, 19, 35);
    });
    server.onNotFound(handleNotFound);
    server.begin();
}

void loop() {
    server.handleClient();
}

boolean connectWifi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(5000);
        Serial.print(".");
    }
    Serial.println("WiFi Connected....IP Adress is:");
    Serial.println(WiFi.localIP());
    return true;
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}