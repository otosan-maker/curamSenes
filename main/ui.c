/*
 * AWS IoT EduKit - Core2 for AWS IoT EduKit
 * Cloud Connected Blinky v1.3.0
 * .c
 * 
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "core2forAWS.h"
#include "ui.h"
#include "curamSenes.h"

#define MAX_TEXTAREA_LENGTH 1024

static lv_obj_t *out_txtarea;
static lv_obj_t *wifi_label;
static lv_obj_t *mqtt_label;
static lv_obj_t *beacon_label;
extern bool bAlertDueTime; 

static char *TAG = "UI";





void ui_textarea_add(char *baseTxt, char *param, size_t paramLen) {
    if( baseTxt != NULL ){
        xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
        lv_textarea_set_text(out_txtarea, baseTxt); 
        xSemaphoreGive(xGuiSemaphore);
    } 
    else{
        ESP_LOGE(TAG, "Textarea baseTxt is NULL!");
    }
}





void ui_wifi_label_update(bool state){
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    if (state == false) {
        lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    } 
    else{
        char buffer[25];
        sprintf (buffer, "#0000ff %s #", LV_SYMBOL_WIFI);
        lv_label_set_text(wifi_label, buffer);
    }
    xSemaphoreGive(xGuiSemaphore);
}

void ui_mqtt_label_update(bool state){
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    if (state == false) {
        lv_label_set_text(mqtt_label, "MQTT");
    } 
    else{
        char buffer[25];
        sprintf (buffer, "#0000ff %s #", "MQTT");
        lv_label_set_text(mqtt_label, buffer);
    }
    xSemaphoreGive(xGuiSemaphore);
}


void ui_beacon_label_update(bool state){
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    if (state == false) {
        lv_label_set_text(beacon_label, "BT");
    } 
    else{
        char buffer[25];
        sprintf (buffer, "#0000ff %s #", "BT");
        lv_label_set_text(beacon_label, buffer);
    }
    xSemaphoreGive(xGuiSemaphore);
}


bool isPulsed(){
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    lv_state_t lv_stt = lv_obj_get_state(out_txtarea,0);
    xSemaphoreGive(xGuiSemaphore);

    if (lv_stt == LV_STATE_FOCUSED)
        return true;
    else
        return false;
}

void releaseMedText(){
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    lv_obj_set_state(out_txtarea,LV_STATE_DEFAULT );
    //lv_obj_set_state(mqtt_label,LV_STATE_FOCUSED );
    xSemaphoreGive(xGuiSemaphore);
}

void hideMedText(){
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    lv_obj_set_hidden(out_txtarea,true );
    //lv_obj_set_state(mqtt_label,LV_STATE_FOCUSED );
    xSemaphoreGive(xGuiSemaphore);
}
void showMedText(){
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    lv_obj_set_hidden(out_txtarea,false );
    //lv_obj_set_state(mqtt_label,LV_STATE_FOCUSED );
    xSemaphoreGive(xGuiSemaphore);
}

void ui_init() {
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    wifi_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(wifi_label,NULL,LV_ALIGN_IN_TOP_RIGHT, 0, 6);
    lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    lv_label_set_recolor(wifi_label, true);
    
    mqtt_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(mqtt_label,NULL,LV_ALIGN_IN_TOP_LEFT, 0, 6);
    lv_label_set_text(mqtt_label, "____");
    lv_label_set_recolor(mqtt_label, true);

    beacon_label= lv_label_create(lv_scr_act(), NULL);
    //lv_obj_align(beacon_label,NULL,LV_ALIGN_IN_TOP_CENTER, 0, 6);
    lv_obj_set_pos(beacon_label, 150, 5);	 
    lv_label_set_text(beacon_label, "bt");
    lv_label_set_recolor(beacon_label, true);


    out_txtarea = lv_textarea_create(lv_scr_act(), NULL);
    lv_obj_set_size(out_txtarea, 300, 190);
    lv_obj_align(out_txtarea, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -12);
    lv_textarea_set_max_length(out_txtarea, MAX_TEXTAREA_LENGTH);
    lv_textarea_set_text_sel(out_txtarea, false);
    lv_textarea_set_cursor_hidden(out_txtarea, true);
    lv_textarea_set_text(out_txtarea, "    Startup   \n");
    
    xSemaphoreGive(xGuiSemaphore);
}



void ui_task(void *arg) {
    
    ui_init();
    int lasNumMed=-1;


    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        TickType_t  mTime = xTaskGetTickCount();
        if(lasNumMed!=i_medPending){
            if (i_medPending == 0){
                ui_textarea_add("                                    \n", "N", 0);
                stopLedNotification();
                bAlertDueTime = false;
            }else{
                char sztmp[1024];
                strcpy(sztmp,"         MEDICATION PENDING\n\n");
                for(int i=0;i<i_medPending;i++){
                    
                    //ESP_LOGI(TAG, " %u Medic %s ",i,medPending[i].m_name);
                    strcat(sztmp," * ");
                    strcat(sztmp,medPending[i].m_name);
                    //mark past DUE 
                    if ((mTime  - medPending[i].timestamp) > PAST_DUE_TIME){
                        strcat(sztmp,"   .....  PAST DUE ");
                    }
                    strcat(sztmp,"\n");
                }
                ui_textarea_add( sztmp , NULL, 0);
            }
            lasNumMed=i_medPending;
        }

    }
    // Should never get here. FreeRTOS tasks loop forever.
    ESP_LOGE(TAG, "Error in UI task. Out of loop.");
    abort();
}