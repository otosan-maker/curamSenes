
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "driver/gpio.h"

#include "core2forAWS.h"





typedef struct wav_header {
    // RIFF Header
    char riff_header[4]; // Contains "RIFF"
    int wav_size; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4]; // Contains "WAVE"
    
    // Format Header
    char fmt_header[4]; // Contains "fmt " (includes trailing space)
    int fmt_chunk_size; // Should be 16 for PCM
    short audio_format; // Should be 1 for PCM. 3 for IEEE Float
    short num_channels;
    int sample_rate;
    int byte_rate; // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    short sample_alignment; // num_channels * Bytes Per Sample
    short bit_depth; // Number of bits per sample
    
    // Data
    char data_header[4]; // Contains "data"
    int data_bytes; // Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[]; // Remainder of wave file is bytes
} wav_header;
// thanks to Owner/Author  Jon-Schneider 
// https://gist.github.com/Jon-Schneider/8b7c53d27a7a13346a643dac9c19d34f

wav_header myHeader;





#define MOUNT_POINT "/sdcard"
static const char *TAG = "SPEAK";
uint8_t* soundBuff;



void writeWavHeader(wav_header myHeader){
    char szTmp[5];
    strncpy(szTmp,myHeader.riff_header,4);
    szTmp[4]=0;
    ESP_LOGI(TAG, " WAV HEADER %s type              ::::  ",szTmp);
    ESP_LOGI(TAG, " WAV HEADER %d size              ::::  ",myHeader.wav_size);
    strncpy(szTmp,myHeader.wave_header,4);
    ESP_LOGI(TAG, " WAV HEADER %s wave header       ::::  ",szTmp);
    strncpy(szTmp,myHeader.fmt_header,4);
    ESP_LOGI(TAG, " WAV HEADER %s fmt_header        ::::  ",szTmp);
    ESP_LOGI(TAG, " WAV HEADER %d fmt_chunk_size    ::::  ",myHeader.fmt_chunk_size);
    ESP_LOGI(TAG, " WAV HEADER %d audio_format      ::::  ",myHeader.audio_format);
    ESP_LOGI(TAG, " WAV HEADER %d num_channels      ::::  ",myHeader.num_channels);
    ESP_LOGI(TAG, " WAV HEADER %d sample_rate       ::::  ",myHeader.sample_rate);
    ESP_LOGI(TAG, " WAV HEADER %d byte_rate         ::::  ",myHeader.byte_rate);
    ESP_LOGI(TAG, " WAV HEADER %d sample_alignment  ::::  ",myHeader.sample_alignment);
    ESP_LOGI(TAG, " WAV HEADER %d bit_depth         ::::  ",myHeader.bit_depth);
    strncpy(szTmp,myHeader.data_header,4);
    ESP_LOGI(TAG, " WAV HEADER %s data header       ::::  ",szTmp);
    ESP_LOGI(TAG, " WAV HEADER %d data_bytes        ::::  ",myHeader.data_bytes);
}


esp_err_t readFile(char *file_path){
    
    sdmmc_card_t* card;
    esp_err_t err;
        
    xSemaphoreTake(spi_mutex, portMAX_DELAY);
    spi_poll();
 
    err = Core2ForAWS_SDcard_Mount(MOUNT_POINT, &card);

    if(err == ESP_OK){
        ESP_LOGI(TAG, "SD Card mounted");
    } 
    else{
        ESP_LOGI(TAG, "SD Card mount error code: %d", err);
        xSemaphoreGive(spi_mutex);
        return 0;
    }
 
 
    ESP_LOGI(TAG, "Reading file %s", file_path);
    
    FILE* f = fopen(file_path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        xSemaphoreGive(spi_mutex);
        return 0;
    }
    size_t numBytes=fread(&myHeader,sizeof(wav_header),1,f);
    writeWavHeader(myHeader);

    int length  = myHeader.data_bytes;
    ESP_LOGI(TAG, " #############   ###  ## #  El fichero es de  %d bytes de longitud",length );
     
    soundBuff=heap_caps_malloc(length, MALLOC_CAP_SPIRAM);
    if (soundBuff==NULL){
        ESP_LOGI(TAG, "MALLOC ERROR");
        fclose(f);
        Core2ForAWS_SDcard_Unmount(MOUNT_POINT, card);
        xSemaphoreGive(spi_mutex);
        return 0;
    }

    int totalLong=0;
    //while (!feof(f)){
        numBytes = fread(soundBuff+totalLong,sizeof(uint8_t),length-1,f);
        totalLong += numBytes;
        ESP_LOGI(TAG, " #############   ###  ## #  READ %d bytes de longitud:%d",numBytes,sizeof(uint8_t));
    //}
    
    fclose(f);
 
    err = Core2ForAWS_SDcard_Unmount(MOUNT_POINT, card);
    xSemaphoreGive(spi_mutex);
 
    if(err == ESP_OK){
        ESP_LOGI(TAG, "Ejected the SDCard");
    } 
    else{
        ESP_LOGI(TAG, "SDCard eject error code: %d", err);
    }
    return length;
}




void speakMe_task(void *arg){
   

    vTaskDelay(pdMS_TO_TICKS(200));
    char file_path[] = MOUNT_POINT"/speech/aud001a.wav";
    int sound_len = readFile(file_path);
    

    Speaker_Init();
    Core2ForAWS_Speaker_Enable(1);
    ESP_LOGI(TAG, "portMAX_DELAY: %d", portMAX_DELAY);
    Speaker_WriteBuff((uint8_t*)soundBuff, sound_len, 100);
    Core2ForAWS_Speaker_Enable(0);
    Speaker_Deinit();
    free(soundBuff);
    ESP_LOGI(TAG, "FINISHED");
    vTaskDelay(pdMS_TO_TICKS(10000));
    vTaskDelete(NULL);

    while(true){
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
