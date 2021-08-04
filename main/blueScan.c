


#include <stdio.h> 
#include <string.h> 

#include "freertos/FreeRTOS.h" 
#include "freertos/task.h" 
#include "freertos/event_groups.h" 

#include "esp_wifi.h" 
#include "esp_event_loop.h" 
#include "esp_log.h" 

#include "esp_gap_ble_api.h" 
#include "esp_bt_main.h" 
#include "esp_bt.h" 
#include "esp_err.h"

#define TAG "BLESCAN" 



#define CRASHINTERVAL 0x10 //Disconnects occur in the first few minutes 
#define CRASHWINDOW 0x10   //Disconnects occur in the first few minutes 

// #define LESSCRASHINTERVAL 0x50 //Disconnects still occur 
// #define LESSCRASHWINDOW 0x30 //Disconnects still occur 
// #define ALMOSTNOCRASHINTERVAL 0x100 //Almost no disconnects occur 
// #define ALMOSTNOCRASHWINDOW 0x30 //Almost no disconnects occur 


const int CONNECTED_BIT = BIT0;
QueueHandle_t qBlueScanQueue;


void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	switch (event)
	{
		case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:  
			ESP_LOGI(TAG, "Scan param set completed event: %d", event);
			break;

		case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:  
			ESP_LOGI(TAG, "Scan started event: %d", event);
			break;

		case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT: /*!< When stop scan complete, the event comes */  
			ESP_LOGI(TAG, "Scan stopped event: %d", event);
			break;

		case ESP_GAP_BLE_SCAN_RESULT_EVT: // Called when one scan result is ready 
			ESP_LOGI(TAG, "Scan result event: %d", event);
			break;

		default:  ESP_LOGE(TAG, "Unhandled bt event: %d", event);
			break;
	}
}


esp_ble_scan_params_t scanparams = {
	.scan_type = BLE_SCAN_TYPE_ACTIVE,
	.own_addr_type = BLE_ADDR_TYPE_PUBLIC,
	.scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
	.scan_interval = CRASHINTERVAL,
	.scan_window = CRASHWINDOW 
};

static void initialise_bt(void)
{
	esp_err_t status;

	ESP_LOGI(TAG, "initBluetooth");

	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	status=esp_bt_controller_init(&bt_cfg);
	ESP_LOGI(TAG, "esp_bt_controller_enable: %d ::: %s", status, esp_err_to_name(status));
	status=esp_bt_controller_enable(ESP_BT_MODE_BLE);
	ESP_LOGI(TAG, "esp_bt_controller_enable: %d::: %s", status, esp_err_to_name(status));
	status=esp_bluedroid_init();
	ESP_LOGI(TAG, "esp_bluedroid_init: %d::: %s", status, esp_err_to_name(status));
	status=esp_bluedroid_enable();
	ESP_LOGI(TAG, "esp_bluedroid_enable: %d::: %s", status, esp_err_to_name(status));

	// Register callback 
	if ((status = esp_ble_gap_register_callback(esp_gap_cb)) != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to register callback (status: %x , %s)", status, esp_err_to_name(status));
		return;
	}
	ESP_ERROR_CHECK(esp_ble_gap_set_scan_params(&scanparams));
}


void blueScan_task(void *arg){
   	char szBuff[64];
	vTaskDelay(pdMS_TO_TICKS(20000));
    initialise_bt();

    while(true){
        // if (xQueueReceive( qBlueScanQueue , &szBuff ,  pdMS_TO_TICKS( 1000 ) ) == pdPASS ){
        //      ESP_LOGI(TAG, "scanning BT.");
        // }
        esp_ble_gap_start_scanning(10);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
