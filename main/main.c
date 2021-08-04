/*
 * AWS IoT EduKit - Core2 for AWS IoT EduKit
 * Cloud Connected Blinky v1.3.0
 * main.c
 * 
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * Additions Copyright 2016 Espressif Systems (Shanghai) PTE LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
/**
 * @file main.c
 * @brief simple MQTT publish and subscribe for use with AWS IoT EduKit reference hardware.
 *
 * This example takes the parameters from the build configuration and establishes a connection to AWS IoT Core over MQTT.
 *
 * Some configuration is required. Visit https://edukit.workshop.aws
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

#include "core2forAWS.h"

#include "wifi.h"
#include "blink.h"
#include "ui.h"
#include "mqtt.h"
#include "curamSenes.h"

TaskHandle_t csHandle;
TaskHandle_t uiHandle;
TaskHandle_t IoTHandle;
TaskHandle_t speakHandle;
TaskHandle_t btScanHandle;

extern TaskHandle_t xBlink;

static const char *TAG = "MAINLOOP";


void app_main()
{
    Core2ForAWS_Init();
    Core2ForAWS_Display_SetBrightness(80);
    xTaskCreatePinnedToCore(&ui_task, "ui_task", 4096 * 1, NULL, 2, &uiHandle, tskNO_AFFINITY);
    
    
    Core2ForAWS_Sk6812_Clear();
    Core2ForAWS_Sk6812_Show();

    initialise_wifi();

    qCSQueue = xQueueCreate( 5, 128 );
    qSoundQueue = xQueueCreate( 1, 64 );
    qBlueScanQueue = xQueueCreate( 2, 64 );

    xTaskCreatePinnedToCore(&aws_iot_task, "aws_iot_task",  4596 * 1, NULL, 2, &IoTHandle, 1);
    xTaskCreatePinnedToCore(&blink_task, "blink_task",      2096 * 1, NULL, 2, &xBlink, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(&cs_task,       "cs_task",      3596 * 1, NULL, 2, &csHandle, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(&speakMe_task, "speakMe_task",  4096 * 1, NULL, 2,  &speakHandle, 1);

    xTaskCreatePinnedToCore(&blueScan_task, "blueScan_task", 4096 * 1, NULL, 2, &btScanHandle, 1);

    while(true){
        //control memory use
        vTaskDelay(pdMS_TO_TICKS(10000));
        // ESP_LOGI(TAG, " ##############################################################################."  );
        // ESP_LOGI(TAG, "MEMORY FREE %s:::%d.","ui_task      ",uxTaskGetStackHighWaterMark(uiHandle) );
        // ESP_LOGI(TAG, "MEMORY FREE %s:::%d.","aws_iot_task ",uxTaskGetStackHighWaterMark(IoTHandle) );
        // ESP_LOGI(TAG, "MEMORY FREE %s:::%d.","blink_task   ",uxTaskGetStackHighWaterMark(xBlink) );
        // ESP_LOGI(TAG, "MEMORY FREE %s:::%d.","cs_task      ",uxTaskGetStackHighWaterMark(csHandle) );
        // ESP_LOGI(TAG, "MEMORY FREE %s:::%d.","speakMe_task ",uxTaskGetStackHighWaterMark(speakHandle) );
        // ESP_LOGI(TAG, "MEMORY FREE %s:::%d.","blueScan_task",uxTaskGetStackHighWaterMark(btScanHandle) );
        
    }
}