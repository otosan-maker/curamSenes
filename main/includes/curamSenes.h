/*
 * curamSenes
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
#pragma once


struct medic {
    char        m_name[64];
    int         id_dsm;
    TickType_t  timestamp;
    lv_obj_t *  btn;
};

#define MAX_MED 10
#define PAST_DUE_TIME 1000 //600*100 //10 minutes

extern QueueHandle_t qCSQueue;
extern struct medic   medPending[MAX_MED];
extern int     i_medPending ;
extern bool bSendMQTT ;
extern char cPayload[128];
extern bool bSendMQTTMedicationEmpty ;
extern char cPayloadMedicationEmpty[128];
extern bool bSendMQTTBeaconLost;
extern char *cPayloadBeaconLost;

void deleteFromMed(int  idToDelete);
void stopLedNotification();
void cs_task(void *arg); 


//Speech function
extern QueueHandle_t qSoundQueue;
void speakMe_task(void *arg);


//bluetooth Scan Function
void blueScan_task(void *arg);

//heart scant 
void heartTestRequest(char * jsonMSG);
void launch_heart_test(int id_test);
void heart_task(void* param);