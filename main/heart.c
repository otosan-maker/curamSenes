/**
 * @file example.c
 * 
 * @author
 * Angelo Elias Dalzotto (150633@upf.br)
 * GEPID - Grupo de Pesquisa em Cultura Digital (http://gepid.upf.br/)
 * Universidade de Passo Fundo (http://www.upf.br/)
 * 
 * @copyright
 * Copyright (c) 2017 Raivis Strogonovs (https://morf.lv)
 * 
 * @brief This is an example file to the MAX30100 library for the ESP32.
 * It initializes the IDF-SDK I2C driver and then initializes the sensor.
 * It crates a task to update the readings at a rate of 100Hz and prints
 * the bpm and oxigen saturation results.
*/
/**
 * Pin assignment:
 * - i2c:
 *    GPIO12: SDA
 *    GPIO14: SDL
 * - no need to add external pull-up resistors.
 */

#include <stdio.h>
#include "driver/i2c.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "max30100.h"

static const char *TAG = "HEART";
bool heartTaskRuning = false;
float fBPM =0.0;
float fSatO2 = 0.0;

extern TaskHandle_t HeartHandle;
extern bool bSendMQTTHeartTest ;
extern char cPayloadHeartTest[128];
extern int id_ths;

#define I2C_SDA 32
#define I2C_SCL 33
#define I2C_FRQ 100000
#define I2C_PORT I2C_NUM_0

max30100_config_t max30100 = {};

esp_err_t i2c_master_init(i2c_port_t i2c_port){
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA;
    conf.scl_io_num = I2C_SCL;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_FRQ;
    i2c_param_config(i2c_port, &conf);
    return i2c_driver_install(i2c_port, I2C_MODE_MASTER, 0, 0, 0);
}

void heart_task(void* param) {
    heartTaskRuning = true;
    vTaskDelay(pdMS_TO_TICKS(10000));

    ESP_LOGI(TAG,"MAX30100 id_test : %d",id_ths);
    ESP_ERROR_CHECK(i2c_master_init(I2C_PORT));
    //Init sensor at I2C_NUM_0
    if(max30100_init( &max30100, I2C_PORT,
                   MAX30100_DEFAULT_OPERATING_MODE,
                   MAX30100_DEFAULT_SAMPLING_RATE,
                   MAX30100_DEFAULT_LED_PULSE_WIDTH,
                   MAX30100_DEFAULT_IR_LED_CURRENT,
                   MAX30100_DEFAULT_START_RED_LED_CURRENT,
                   MAX30100_DEFAULT_MEAN_FILTER_SIZE,
                   MAX30100_DEFAULT_PULSE_BPM_SAMPLE_SIZE,
                   true, false ) == -1){
                       //sensor not connected
                       ESP_LOGI(TAG,"NO Sensor connected. Stopping the task.");
                        heartTaskRuning = false;
                        vTaskDelete(HeartHandle);
                   }
    max30100_data_t result = {};
    int idata=0;
    while(true) {
        //Update sensor, saving to "result"
        ESP_ERROR_CHECK(max30100_update(&max30100, &result));
        if(result.pulse_detected) {
            fBPM    += result.heart_bpm;
            fSatO2  += result.spO2 ;
            idata++;
            ESP_LOGI(TAG,"BPM: %f | SpO2: %f%%", result.heart_bpm, result.spO2);
            if(idata>1)
                break;
        }
        //Update rate: 100Hz
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG,"MEDIA idata:%d",idata);
    ESP_LOGI(TAG,"BPM: %.02f | SpO2: %.02f %%\n", fBPM/idata, fSatO2/idata);

    //send message to test backoffice function
    sprintf(cPayloadHeartTest,"{\"id_ths\":%d,\"bpm\":%.02f,\"spo2\":%.02f}", id_ths, fBPM/idata, fSatO2/idata );
    ESP_LOGI(TAG,"Mandamos este mensaje %s",cPayloadHeartTest);
    bSendMQTTHeartTest=true;
    heartTaskRuning = false;
    vTaskDelete(HeartHandle);
}


/**
 * MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/