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

lv_obj_t *heart_button=NULL;
lv_obj_t *med_appointment_button=NULL;
lv_obj_t *empty_button=NULL;

extern bool bAlertDueTime; 

static char *TAG = "UI";

extern bool heartTaskRuning;

static void empty_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
        ESP_LOGE(TAG, "Clicked EMPTY handler: %s\n", lv_list_get_btn_text(obj));
        xSemaphoreGive(xGuiSemaphore);
    }
}


static void med_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
        ESP_LOGE(TAG, "Clicked med event handler: %s", lv_list_get_btn_text(obj));
        xSemaphoreGive(xGuiSemaphore);
    }
}


static void medApp_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
        ESP_LOGE(TAG, "Clicked med appointment event handler: %s", lv_list_get_btn_text(obj));
        xSemaphoreGive(xGuiSemaphore);
    }
}


static void heart_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
        ESP_LOGE(TAG, "Clicked heart event handler: %s", lv_list_get_btn_text(obj));
        xSemaphoreGive(xGuiSemaphore);
    }
}

char szHeartLabel[64];
void setHeartButton(bool state){
    ESP_LOGE(TAG, "setHeartButton ");
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    if ((heart_button==NULL) && (state==true)){
        ESP_LOGE(TAG, "((heart_button==NULL) && (state==true)) ");
        strcpy(szHeartLabel,"HEART READING");
        heart_button= lv_list_add_btn(out_txtarea, LV_SYMBOL_AUDIO,szHeartLabel );
        lv_obj_set_event_cb(heart_button, heart_event_handler);
    }

    if( (heart_button!=NULL) &&(state==false)){
        ESP_LOGE(TAG, "((state==false)) ");
        lv_list_remove(out_txtarea,lv_list_get_btn_index(out_txtarea, heart_button));
        heart_button=NULL;
    }
    xSemaphoreGive(xGuiSemaphore);
}


void setMsgHeartButton(float bpm ,float spo2){
    ESP_LOGE(TAG, "setMsgHeartButton ");
    if (heart_button!=NULL){
        sprintf(szHeartLabel,"BPM: %.02f SPO2: %.02f %%",bpm,spo2);
        ESP_LOGE(TAG, "setMsgHeartButton: %s  ",szHeartLabel);
        xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
        lv_obj_t * label;
        label = lv_label_create(heart_button, NULL);
        lv_label_set_text(label, szHeartLabel );
        xSemaphoreGive(xGuiSemaphore);
    }
}





char szMedAppLabel[64];
void setMedAppButton(bool state){
    ESP_LOGE(TAG, "setHeartButton ");
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    if ((med_appointment_button==NULL) && (state==true)){
        ESP_LOGE(TAG, "setMedAppButton: ((med_appointment_button==NULL) && (state==true)) ");
        strcpy(szHeartLabel,"Medical Appointment");
        med_appointment_button= lv_list_add_btn(out_txtarea, LV_SYMBOL_AUDIO,szHeartLabel );
        lv_obj_set_event_cb(med_appointment_button, heart_event_handler);
    }

    if( (med_appointment_button!=NULL) &&(state==false)){
        ESP_LOGE(TAG, "setMedAppButton: ((state==false)) ");
        lv_list_remove(out_txtarea,lv_list_get_btn_index(out_txtarea, heart_button));
        med_appointment_button=NULL;
    }
    xSemaphoreGive(xGuiSemaphore);
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
        lv_label_set_text(beacon_label, LV_SYMBOL_BLUETOOTH);
    } 
    else{
        char buffer[25];
        sprintf (buffer, "#0000ff %s #", LV_SYMBOL_BLUETOOTH);
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
    lv_label_set_text(beacon_label, LV_SYMBOL_BLUETOOTH  );
    lv_label_set_recolor(beacon_label, true);


    
    out_txtarea = lv_list_create(lv_scr_act(), NULL);
    lv_obj_set_size(out_txtarea, 300, 190);
    lv_obj_align(out_txtarea, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -12);
    
    
    xSemaphoreGive(xGuiSemaphore);
}


void deleteMedButtons(){
    for (int i = 0; i < i_medPending; i++)
        lv_list_remove(out_txtarea,lv_list_get_btn_index(out_txtarea, medPending[i].btn)); 
    }
}



void drawBox(){
    lv_obj_t * list_btn;
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    TickType_t  mTime = xTaskGetTickCount();
    //delete medical bottons
    //lv_list_clean(out_txtarea);
    deleteMedButtons();
    if (i_medPending == 0)    {
        //Let the screen empty for now
        //list_btn = lv_list_add_btn(out_txtarea, LV_SYMBOL_FILE, "    EMPTY");
        //lv_obj_set_event_cb(list_btn, empty_event_handler);
        stopLedNotification();
        bAlertDueTime = false; //Revisar
    }
    else    {
        char sztmp[112];
        strcpy(sztmp, "");
        for (int i = 0; i < i_medPending; i++)        {
            strcat(sztmp, medPending[i].m_name);
            //mark past DUE
            if ((mTime - medPending[i].timestamp) > PAST_DUE_TIME)
            {
                strcat(sztmp, "   .....  PAST DUE ");
            }
            list_btn = lv_list_add_btn(out_txtarea, LV_SYMBOL_UPLOAD, sztmp);
            lv_obj_set_event_cb(list_btn, med_event_handler);
            medPending[i].btn=list_btn;
        }
    }
    //if heart task is running
    if(heartTaskRuning == true){
        heart_button= lv_list_add_btn(out_txtarea, LV_SYMBOL_AUDIO,szHeartLabel );
        lv_obj_set_event_cb(heart_button, heart_event_handler);
    }
    

    xSemaphoreGive(xGuiSemaphore);
}


void ui_task(void *arg) {
    
    ui_init();
    int lasNumMed=-1;
    


    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        //any change in the number of medicines ...
        if(lasNumMed!=i_medPending){
            drawBox();
            lasNumMed=i_medPending;
        }
        

    }
    // Should never get here. FreeRTOS tasks loop forever.
    ESP_LOGE(TAG, "Error in UI task. Out of loop.");
    abort();
}