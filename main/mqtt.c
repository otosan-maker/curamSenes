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


#include "wifi.h"
#include "ui.h"
#include "mqtt.h"
#include "curamSenes.h"

bool bSendMQTT = false;
bool bSendMQTTMedicationEmpty = false;
bool bSendMQTTBeaconLost = false;
bool bSendMQTTHeartTest = false;
char cPayload[128];
char cPayloadMedicationEmpty[128];
char cPayloadHeartTest[128];
char *cPayloadBeaconLost="{msg:\"lost beacon contact\"}";


extern bool heartTaskRuning;

/* The time between each MQTT message publish in milliseconds */
#define PUBLISH_INTERVAL_MS 6000

/* The time prefix used by the logger. */
static const char *TAG = "MQTT";

/* The FreeRTOS task handler for the blink task that can be used to control the task later */
TaskHandle_t xBlink;

AWS_IoT_Client client;

/* CA Root certificate */
extern const uint8_t aws_root_ca_pem_start[] asm("_binary_aws_root_ca_pem_start");
extern const uint8_t aws_root_ca_pem_end[] asm("_binary_aws_root_ca_pem_end");

/* Default MQTT HOST URL is pulled from the aws_iot_config.h */
char HostAddress[255] = AWS_IOT_MQTT_HOST;

/* Default MQTT port is pulled from the aws_iot_config.h */
uint32_t port = AWS_IOT_MQTT_PORT;



void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData) {
    char szMedBuff[129];
    ESP_LOGI(TAG, "Subscribe callback");
    ESP_LOGI(TAG, "%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *)params->payload);
    if (strstr(topicName, "/blink") != NULL) {
        // Get state of the FreeRTOS task, "blinkTask", using it's task handle.
        // Suspend or resume the task depending on the returned task state
        eTaskState blinkState = eTaskGetState(xBlink);
        if (blinkState == eSuspended){
            vTaskResume(xBlink);
        } else{
            vTaskSuspend(xBlink);
        }
    }
    if (strstr(topicName, "/medication") != NULL) {
        // We receive a new medication notification
        sprintf(szMedBuff,"%.*s\n",(int) params->payloadLen,(char *)params->payload);
        xQueueSend( qCSQueue, ( void * ) &szMedBuff, ( TickType_t ) 1000 );
        ESP_LOGI(TAG, "Medication sent");
    }
    if (strstr(topicName, "/med_appointment") != NULL) {
        // We receive a new medication appointment
        sprintf(szMedBuff,"%.*s\n",(int) params->payloadLen,(char *)params->payload);
        //xQueueSend( qCSQueue, ( void * ) &szMedBuff, ( TickType_t ) 1000 );
        ESP_LOGI(TAG, "Medication appointment");
    }if (strstr(topicName, "/med_test") != NULL) {
        ESP_LOGI(TAG, "Medication test request");
        // We receive a new medication test request
        sprintf(szMedBuff,"%.*s\n",(int) params->payloadLen,(char *)params->payload);
        //launch test task only if it is not running, discard otherwise
        if(heartTaskRuning==false)
            heartTestRequest(szMedBuff);
        
    }

}

void disconnect_callback_handler(AWS_IoT_Client *pClient, void *data) {
    ESP_LOGW(TAG, "MQTT Disconnect");
    ui_mqtt_label_update(false);
    // ui_textarea_add("Disconnected from AWS IoT Core...", NULL, 0);
    IoT_Error_t rc = FAILURE;

    if(pClient == NULL) {
        return;
    }

    if(aws_iot_is_autoreconnect_enabled(pClient)) {
        ESP_LOGI(TAG, "Auto Reconnect is enabled, Reconnecting attempt will start now");
    } else {
        ESP_LOGW(TAG, "Auto Reconnect not enabled. Starting manual reconnect...");
        rc = aws_iot_mqtt_attempt_reconnect(pClient);
        if(NETWORK_RECONNECTED == rc) {
            ESP_LOGW(TAG, "Manual Reconnect Successful");
            ui_mqtt_label_update(true);
        } else {
            ESP_LOGW(TAG, "Manual Reconnect Failed - %d", rc);
            ui_mqtt_label_update(false);
        }
    }
}






static void publish(AWS_IoT_Client *client, char *base_topic,char *cLoad){

    IoT_Publish_Message_Params paramsQOS1;

    
    paramsQOS1.qos = QOS1;
    paramsQOS1.payload = (void *) cLoad;
    paramsQOS1.isRetained = 0;
    paramsQOS1.payloadLen = strlen(cLoad);
    IoT_Error_t rc = aws_iot_mqtt_publish(client, base_topic, strlen(base_topic), &paramsQOS1);
    if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
        ESP_LOGW(TAG, "QOS1 publish ack not received.");
        rc = SUCCESS;
    }
    
}


static void publishAck(AWS_IoT_Client *client, char *base_topic){
    publish(client, base_topic,cPayload);
    bSendMQTT=false;
}




static void publishEmpty(AWS_IoT_Client *client, char *base_topic){
    publish(client, base_topic,cPayloadMedicationEmpty);
    bSendMQTTMedicationEmpty=false;
}


static void publisBeaconLost(AWS_IoT_Client *client, char *base_topic){
    publish(client, base_topic,cPayloadBeaconLost);
    bSendMQTTBeaconLost=false;
}


static void publisHeartTestResult(AWS_IoT_Client *client, char *base_topic){
    publish(client, base_topic,cPayloadHeartTest);
    bSendMQTTHeartTest=false;
}




void aws_iot_task(void *param) {
    IoT_Error_t rc = FAILURE;

    
    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
    IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

    ESP_LOGI(TAG, "AWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    mqttInitParams.enableAutoReconnect = false; // We enable this later below
    mqttInitParams.pHostURL = HostAddress;
    mqttInitParams.port = port;    
    mqttInitParams.pRootCALocation = (const char *)aws_root_ca_pem_start;
    mqttInitParams.pDeviceCertLocation = "#";
    mqttInitParams.pDevicePrivateKeyLocation = "#0";
    
#define CLIENT_ID_LEN (ATCA_SERIAL_NUM_SIZE * 2)
#define SUBSCRIBE_TOPIC_LEN (CLIENT_ID_LEN + 3)
#define BASE_PUBLISH_TOPIC_LEN (CLIENT_ID_LEN + 2)

    char *client_id = malloc(CLIENT_ID_LEN + 1);
    ATCA_STATUS ret = Atecc608_GetSerialString(client_id);
    if (ret != ATCA_SUCCESS)
    {
        printf("Failed to get device serial from secure element. Error: %i", ret);
        abort();
    }

    char subscribe_topic[SUBSCRIBE_TOPIC_LEN];
    char base_publish_topic[BASE_PUBLISH_TOPIC_LEN+12];
    snprintf(subscribe_topic, SUBSCRIBE_TOPIC_LEN, "%s/#", client_id);
    snprintf(base_publish_topic, BASE_PUBLISH_TOPIC_LEN, "%s/", client_id);

    mqttInitParams.mqttCommandTimeout_ms = 20000;
    mqttInitParams.tlsHandshakeTimeout_ms = 5000;
    mqttInitParams.isSSLHostnameVerify = true;
    mqttInitParams.disconnectHandler = disconnect_callback_handler;
    mqttInitParams.disconnectHandlerData = NULL;

    rc = aws_iot_mqtt_init(&client, &mqttInitParams);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "aws_iot_mqtt_init returned error : %d ", rc);
        abort();
    }

    /* Wait for WiFI to show as connected */
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);    

    connectParams.keepAliveIntervalInSec = 10;
    connectParams.isCleanSession = true;
    connectParams.MQTTVersion = MQTT_3_1_1;

    connectParams.pClientID = client_id;
    connectParams.clientIDLen = CLIENT_ID_LEN;
    connectParams.isWillMsgPresent = false;
    //ui_textarea_add("Connecting to AWS IoT Core...\n", NULL, 0);
    ESP_LOGI(TAG, "Connecting to AWS IoT Core at %s:%d", mqttInitParams.pHostURL, mqttInitParams.port);
    ESP_LOGI(TAG, "CLIENT ID %s", client_id);
    do {
        rc = aws_iot_mqtt_connect(&client, &connectParams);
        if(SUCCESS != rc) {
            ESP_LOGE(TAG, "Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    } while(SUCCESS != rc);
    // ui_textarea_add("Successfully connected!\n", NULL, 0);
    ESP_LOGI(TAG, "Successfully connected to AWS IoT Core!");
    ui_mqtt_label_update(true);
    /*
     * Enable Auto Reconnect functionality. Minimum and Maximum time for exponential backoff for retries.
     *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
     *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
     */
    rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
    if(SUCCESS != rc) {
        // ui_textarea_add("Unable to set Auto Reconnect to true\n", NULL, 0);
        ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d", rc);
        abort();
    }

    ESP_LOGI(TAG, "Subscribing to '%s'", subscribe_topic);
    rc = aws_iot_mqtt_subscribe(&client, subscribe_topic, strlen(subscribe_topic), QOS0, iot_subscribe_callback_handler, NULL);
    if(SUCCESS != rc) {
        // ui_textarea_add("Error subscribing\n", NULL, 0);
        ESP_LOGE(TAG, "Error subscribing : %d ", rc);
        abort();
    } else{
        // ui_textarea_add("Subscribed to topic: %s\n\n", subscribe_topic, SUBSCRIBE_TOPIC_LEN) ;
        ESP_LOGI(TAG, "Subscribed to topic '%s'", subscribe_topic);
    }
    
    ESP_LOGI(TAG, "\n****************************************\n*  AWS client Id - %s  *\n****************************************\n\n",client_id);
    
    // ui_textarea_add("Attempting publish to: %s\n", base_publish_topic, BASE_PUBLISH_TOPIC_LEN) ;
    while((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc)) {

        //Max time the yield function will wait for read messages
        rc = aws_iot_mqtt_yield(&client, 100);
        if(NETWORK_ATTEMPTING_RECONNECT == rc) {
            // If the client is attempting to reconnect we will skip the rest of the loop.
            continue;
        }

        ESP_LOGD(TAG, "Stack remaining for task '%s' is %d bytes", pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(pdMS_TO_TICKS(PUBLISH_INTERVAL_MS));
        
        //publisher(&client, base_publish_topic, BASE_PUBLISH_TOPIC_LEN);
        // ui_textarea_add("No Message\n", "N", 0);
        if(bSendMQTT){
            sprintf(base_publish_topic,  "%s/rtn", client_id);
            publishAck(&client, base_publish_topic);
        }
        if(bSendMQTTMedicationEmpty){
            sprintf(base_publish_topic,  "%s/rtn", client_id);
            publishEmpty(&client, base_publish_topic);
        }
        if(bSendMQTTBeaconLost){
            sprintf(base_publish_topic,  "%s/beacon", client_id);
            publisBeaconLost(&client, base_publish_topic);
        }
        if(bSendMQTTHeartTest){
            sprintf(base_publish_topic,  "%s/test_heart_rtn", client_id);
            publisHeartTestResult(&client, base_publish_topic);
        }

    }

    ESP_LOGE(TAG, "An error occurred in the main loop.");
    abort();
}

