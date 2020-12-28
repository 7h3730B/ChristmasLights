#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <FS.h>

const char *ssid = "YOUR-WIFI-SSID-GOES-HERE";
const char *password = "YOUR-WIFI-PASSWORD-GOES-HERE";

ESP8266WebServer server(80);

const uint16_t kIrLed = 4; // ESP8266 D2

IRsend irsend(kIrLed);

boolean connectWifi();
void handleNotFound();
String getContentType(String filename);
boolean handleFileRead(String path);

void setup() {
    Serial.begin(115200);
    connectWifi();
    
    irsend.begin();

    SPIFFS.begin();

    server.on("/api", [](){
        server.send(200, "text/plain", "OK");
        uint16_t raw_data[20];
        for (int i = 0; i < server.args(); i++) {
            raw_data[i] = server.arg(i).toInt();
        }
        for (int i = 0; i <= 6; i++) {
            irsend.sendRaw(raw_data, 19, 35);
        }
    });
    server.onNotFound([]() {
        if (!handleFileRead(server.uri())) {
            handleNotFound();
        }
    });
    server.begin();

    if (MDNS.begin("SuperDuperCooleLights")) {
        Serial.println("mDNS responder started");
    } else {
        Serial.println("Error setting up mDNS responder");
    }
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

String getContentType(String filename) {
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    return "text/plain";
}

boolean handleFileRead(String path) {
    if (path.endsWith("/")) path += "index.html";
    String contenttype = getContentType(path);
    if (SPIFFS.exists(path)) {
        File file = SPIFFS.open(path, "r");
        size_t sent = server.streamFile(file, contenttype);
        file.close();
        return true;
    }
    return false;
}