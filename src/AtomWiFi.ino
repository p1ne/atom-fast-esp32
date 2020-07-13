#include "AtomWiFi.h"

void wifiAPSetup() {
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  IPAddress ipAddress = WiFi.softAPIP();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(String(ipAddress[0])+"."+String(ipAddress[1])+"."+String(ipAddress[2])+"."+String(ipAddress[3]),6,0,4);
  delay(1000);
  server.begin();
}

void wifiClientSetup() {
  wifiClient = WiFiClient();
  Serial.println("[WiFi-Client] Connecting (WiFI)...");
  wifiStatus = WIFI_CONNECTING;

  drawWiFi();
  WiFi.begin(WIFI_CLIENT_SSID, WIFI_CLIENT_PASSWORD);

  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    wifiStatus = WIFI_CONNECTED; 
    Serial.print("[WiFi-Client] WiFi connected. IP address: ");
    Serial.println(WiFi.localIP());
    //WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), WiFi.gatewayIP()); 
    drawWiFi();
  } else {
    wifiStatus = WIFI_DISCONNECTED;
    drawWiFi();
  }
}

void doWiFiServer() {
  wifiAPSetup();

  WiFiClient client = server.available(); 
  
  while (true) {
    if (client) {  
      Serial.println("[WiFi-Server] got client");
      String currentLine = "";
      while (client.connected()) {
        Serial.println("[WiFi-Server] client connected");
        if (client.available()) {
          Serial.println("WiFi-Server] client data available");
          char c = client.read();
          header += c;
          if (c == '\n') {
            if (currentLine.length() == 0) {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();


              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<link rel=\"icon\" href=\"data:,\">");
              client.println("<body><h1>ESP32 Web Server</h1>");
              client.println("</body></html>");
              client.println();
              break;
            } else {
              currentLine = "";
            }
          } else if (c != '\r') {
            currentLine += c;
          }
        }
      }
      header = "";
      client.stop();
    }
  }
}