
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "driver/gpio.h"

// #include "esp_bt.h"

#include "core2forAWS.h"


/* The time prefix used by the logger. */
static const char *TAG = "BLUESCAN";



QueueHandle_t qBlueScanQueue;



void blueScan_task(void *arg){
   char szBuff[64];

    while(true){
        if (xQueueReceive( qBlueScanQueue , &szBuff ,  pdMS_TO_TICKS( 1000 ) ) == pdPASS ){
             ESP_LOGI(TAG, "scanning BT.");
        }
        //vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
