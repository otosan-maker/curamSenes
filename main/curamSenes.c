/*
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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "core2forAWS.h"
#include "curamSenes.h"

#include "ui.h"
#include "mqtt.h"
#include "json_parser.h"

static const char *TAG = "cs";
extern int iBotLedColor;
extern TaskHandle_t xBlink;



QueueHandle_t qCSQueue;

struct medic   medPending[MAX_MED];
int     i_medPending =0;




void csNewMedication(char *szBuffer){
    if (i_medPending == (MAX_MED-1) ){
        ESP_LOGW(TAG, "Max num medicine, skipping.");
        return;
    }

    ESP_LOGI(TAG, "%s", (char*)szBuffer );
    jparse_ctx_t myCTX;

    if (json_parse_start(&myCTX, szBuffer, strlen(szBuffer)) != 0) {
        ESP_LOGI(TAG, "JSON PARSER ERROR.");
    }else{
       int val_size = 0;
       if (json_obj_get_strlen(&myCTX, "m_name", &val_size) == 0) {
           val_size++; /* For NULL termination */
           char *s = calloc(1, val_size);
            if (!s) {
               ESP_LOGI(TAG, "CALLOC ERROR.");
            }
    
            json_obj_get_string(&myCTX,  "m_name", s, val_size);
            strncpy(medPending[i_medPending].m_name,s,64);
            ESP_LOGI(TAG, "m_name vale ... :%s",s);
            free(s);

            int iTmp;
            json_obj_get_int(&myCTX,  "id_dsm", &iTmp);
            medPending[i_medPending].id_dsm=iTmp;

            
            medPending[i_medPending].timestamp=xTaskGetTickCount();
            i_medPending++;
        }

    
    json_parse_end(&myCTX);

    //informamos de que hemos recogido el dato

    }
    iBotLedColor = 0xffA500;
    vTaskResume(xBlink);
    releaseMedText();
    
    sprintf(cPayload,"{\"id_dsm\":[%d],\"status\":0}",medPending[i_medPending-1].id_dsm);
    ESP_LOGI(TAG, "Return ACK: %s", cPayload );
    bSendMQTT=true;
    xQueueSend( qSoundQueue, ( void * ) &"/speech/aud001.wav", ( TickType_t ) 1000 );
}


void stopLedNotification(){
    vTaskSuspend(xBlink);
    Core2ForAWS_Sk6812_Clear();
    Core2ForAWS_Sk6812_Show();
}


void csMedicationClear(){
    char szTmpVID[5*MAX_MED];  
    char szTmpID[6];
    
    stopLedNotification();
    strcpy(szTmpVID,"");
    for(int j=0;j<i_medPending;j++){
        sprintf(szTmpID,"%d",medPending[j].id_dsm);
        strcat(szTmpVID,szTmpID);
        if(j<i_medPending-1)
            strcat(szTmpVID,",");
    }

    sprintf(cPayloadMedicationEmpty,"{\"id_dsm\":[%s],\"status\":1}",szTmpVID);
    ESP_LOGI(TAG, "Empty medication: %s", cPayloadMedicationEmpty );
    i_medPending=0;
    bSendMQTTMedicationEmpty=true;
    releaseMedText();
}


void deleteFromMed(int  idToDelete){
    for(int j=idToDelete;j<i_medPending-1;j++){
        strcpy (medPending[j].m_name    , medPending[j+1].m_name );
                medPending[j].id_dsm    = medPending[j+1].id_dsm;
                medPending[j].timestamp = medPending[j+1].timestamp;
    }
    i_medPending--;
}



void cs_task(void *arg) {  
    char szBuff[129];

    while (1) {
        //red blink if any medication overpass timeout ... 10 sec
        TickType_t  mTime = xTaskGetTickCount();

        for (int i=0;i<i_medPending;i++){
            //ESP_LOGI(TAG, "%d timeout :%u, %u,%u",i,medPending[i].timestamp,mTime, (mTime - medPending[i].timestamp ));
            if ((mTime - medPending[i].timestamp) > PAST_DUE_TIME ){
                iBotLedColor = 0xff0000;
                xQueueSend( qSoundQueue, ( void * ) &"/speech/aud002.wav", ( TickType_t ) 1000 );
            }
        }

        //if we pulse the text box, we select it and release de pending medication
        if ( isPulsed()){
            csMedicationClear();
        }
        
         if (xQueueReceive( qCSQueue , &szBuff ,  pdMS_TO_TICKS( 1000 ) ) == pdPASS ){
             ESP_LOGI(TAG, "New Medication.");
             csNewMedication(szBuff);
         }
    }
    // Should never get here. FreeRTOS tasks loop forever.
    ESP_LOGE(TAG, "Error in CS task. Out of loop.");
    abort();
}



