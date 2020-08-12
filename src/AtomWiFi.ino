#include "AtomWiFi.h"

void wifiClientSetup() {

  WiFi.persistent(false);
  fixWifiPersistencyFlag();
  WiFi.disconnect(true);
  stateSince = getUptime();
  WiFi.onEvent(WiFiEvent);
  Serial.println("End of setup");
}

long getUptime() {
  return esp_timer_get_time() / 1000000L;
}

void changeState(wifiState_t state) {
  wifiStatus = state;
  stateSince = getUptime();
}

void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.printf("[WiFi-event] event: %d\n", event);

    switch (event) {
        case SYSTEM_EVENT_WIFI_READY: 
            Serial.println("WiFi interface ready");
            break;
        case SYSTEM_EVENT_SCAN_DONE:
            Serial.println("Completed scan for access points");
            break;
        case SYSTEM_EVENT_STA_START:
            Serial.println("WiFi client started");
            break;
        case SYSTEM_EVENT_STA_STOP:
            Serial.println("WiFi client stopped");
            changeState(WIFI_DISCONNECTED);
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            Serial.println("Connected to access point");
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            Serial.println("Disconnected from WiFi access point");
            break;
        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
            Serial.println("Authentication mode of access point has changed");
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            Serial.print("Obtained IP address: ");
            Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
            changeState(WIFI_CONNECTED);
            break;
        case SYSTEM_EVENT_STA_LOST_IP:
            Serial.println("Lost IP address and IP address is reset to 0");
            //changeState(MYSTATE_OFFLINE);
            break;
        case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
            Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
            break;
        case SYSTEM_EVENT_STA_WPS_ER_FAILED:
            Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
            break;
        case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
            Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
            break;
        case SYSTEM_EVENT_STA_WPS_ER_PIN:
            Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
            break;
        case SYSTEM_EVENT_AP_START:
            Serial.println("WiFi access point started");
            break;
        case SYSTEM_EVENT_AP_STOP:
            Serial.println("WiFi access point  stopped");
            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            Serial.println("Client connected");
            break;
        case SYSTEM_EVENT_AP_STADISCONNECTED:
            Serial.println("Client disconnected");
            break;
        case SYSTEM_EVENT_AP_STAIPASSIGNED:
            Serial.println("Assigned IP address to client");
            break;
        case SYSTEM_EVENT_AP_PROBEREQRECVED:
            Serial.println("Received probe request");
            break;
        case SYSTEM_EVENT_GOT_IP6:
            Serial.println("IPv6 is preferred");
            break;
        case SYSTEM_EVENT_ETH_START:
            Serial.println("Ethernet started");
            break;
        case SYSTEM_EVENT_ETH_STOP:
            Serial.println("Ethernet stopped");
            break;
        case SYSTEM_EVENT_ETH_CONNECTED:
            Serial.println("Ethernet connected");
            break;
        case SYSTEM_EVENT_ETH_DISCONNECTED:
            Serial.println("Ethernet disconnected");
            break;
        case SYSTEM_EVENT_ETH_GOT_IP:
            Serial.println("Obtained IP address");
            break;
        default: break;
    }
}

/**
 * Disable persistent mode, see https://github.com/espressif/arduino-esp32/issues/1393
 */
void fixWifiPersistencyFlag() {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  Serial.printf("cfg.nvs_enable before: %d\n", cfg.nvs_enable);
  cfg.nvs_enable = 0;
}

void connectWiFi() {
  long uptime = getUptime();
  if (wifiStatus == WIFI_DISCONNECTED) {
    Serial.println("Connecting NOW");
    changeState(WIFI_CONNECTING);
    if (firstConnect) {
      WiFi.begin(WIFI_CLIENT_SSID, WIFI_CLIENT_PASSWORD);
      firstConnect = false;
    } else {
      // doesn't work without WiFiGeneric.cpp patch below
      WiFi.mode(WIFI_STA);
      WiFi.reconnect();
    }
  } else if (wifiStatus == WIFI_CONNECTING && stateSince+WIFI_TIMEOUT_CONNECTING < uptime) {
    Serial.println("Cancelling NOW after no connect success");
    changeState(WIFI_DISCONNECTING);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }
}
