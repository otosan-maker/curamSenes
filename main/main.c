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
extern TaskHandle_t xBlink;

void app_main()
{
    Core2ForAWS_Init();
    Core2ForAWS_Display_SetBrightness(80);
    xTaskCreatePinnedToCore(&ui_task, "ui_task", 6096 * 1, NULL, 2, &uiHandle, tskNO_AFFINITY);
    
    
    Core2ForAWS_Sk6812_Clear();
    Core2ForAWS_Sk6812_Show();

    initialise_wifi();

    qCSQueue = xQueueCreate( 5, 128 );

    xTaskCreatePinnedToCore(&aws_iot_task, "aws_iot_task", 4096 * 2, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(&blink_task, "blink_task", 4096 * 1, NULL, 2, &xBlink, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(&cs_task, "cs_task", 4096 * 1, NULL, 2, &csHandle, tskNO_AFFINITY);
    
}