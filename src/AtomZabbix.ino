#include "AtomZabbix.h"

void zabbixSetup() {
	//zabbixAttempts = ZABBIX_MAX_RETRIES;
	zSender.Init(IPAddress(ZABBIX_IP), ZABBIX_PORT, ZABBIX_METRIC_HOST);
	zabbixQueue = xQueueCreate( 1, sizeof( float ) );

	if (wifiStatus == WIFI_DISCONNECTED) {
			wifiClientSetup();
			zabbixSetupHostname();
	}
}

void zabbixSetupHostname() {
	IPAddress zabbixIP;
	if (WiFi.hostByName(ZABBIX_HOST, zabbixIP) == 1) {
		//zabbixAttempts = ZABBIX_MAX_RETRIES;
		zSender.Init(zabbixIP, ZABBIX_PORT, ZABBIX_METRIC_HOST);
	}
}

void doZabbix(void *parameter) {
	float value;
	String metric;

	while (true) {
		while (wifiStatus != WIFI_CONNECTED) {
			connectWiFi();
			zabbixSetupHostname();
			//zabbixAttempts = ZABBIX_MAX_RETRIES;
			vTaskDelay(200 / portTICK_RATE_MS);
		}		
        xQueueReceive(zabbixQueue, &value, portMAX_DELAY);
        if (value > 0) {
        	metric = ZABBIX_METRIC_NAME;
		} else {
			metric = ZABBIX_STATUS_NAME;
		}
		zSender.ClearItem();
		zSender.AddItem(metric, value);
		if (wifiStatus == WIFI_CONNECTED) {
			if (zSender.Send() == EXIT_SUCCESS) {
				Serial.println("ZABBIX SEND: OK");
				//zabbixAttempts = ZABBIX_MAX_RETRIES;
				vTaskDelay(500 / portTICK_RATE_MS);
			} else {
				//zabbixAttempts--;
				Serial.println("ZABBIX SEND: NOT OK. Remaining attempts: " + String(zabbixAttempts));
				vTaskDelay(500 / portTICK_RATE_MS);
			}
		} 
	}
	vTaskDelete( NULL );
}