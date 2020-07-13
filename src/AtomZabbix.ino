#include "AtomZabbix.h"

void zabbixSetup() {
	zabbixAttempts = ZABBIX_MAX_RETRIES;
	zSender.Init(IPAddress(ZABBIX_IP), ZABBIX_PORT, ZABBIX_METRIC_HOST);
	zabbixQueue = xQueueCreate( 1, sizeof( float ) );
}

void zabbixSetupHostname() {
	IPAddress zabbixIP;
	if (WiFi.hostByName(ZABBIX_HOST, zabbixIP) == 1) {
		zabbixAttempts = ZABBIX_MAX_RETRIES;
		zSender.Init(zabbixIP, ZABBIX_PORT, ZABBIX_METRIC_HOST);
	}
}

void doZabbix(void *parameter) {
	float value;
	while (zabbixAttempts > 0) {
        xQueueReceive(zabbixQueue, &value, portMAX_DELAY);
		zSender.ClearItem();
		zSender.AddItem(ZABBIX_METRIC_NAME, value);
		if ((wifiStatus == WIFI_CONNECTED) && (bleConnected)) {
			if (zSender.Send() == EXIT_SUCCESS) {
				Serial.println("ZABBIX SEND: OK");
				drawDot(TFT_GREEN);
				zabbixAttempts = ZABBIX_MAX_RETRIES;
				vTaskDelay(500 / portTICK_RATE_MS);
			} else {
				zabbixAttempts--;
				Serial.println("ZABBIX SEND: NOT OK. Remaining attempts: " + String(zabbixAttempts));
				drawDot(TFT_RED);
				vTaskDelay(500 / portTICK_RATE_MS);
			}
		}
	}
	vTaskDelete( NULL );
}