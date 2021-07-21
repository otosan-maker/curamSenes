
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



#define MOUNT_POINT "/sdcard"


void speakMe(char *szFile){
    sdmmc_card_t* card;
    esp_err_t ret;

    ret = Core2ForAWS_SDcard_Mount(MOUNT_POINT, &card);
}
