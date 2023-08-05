#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define TAG_MAIN "main"
#define TAG_TASK1 "task1"
#define TAG_TASK2 "task2"

#define STACK_SIZE 1024*2
#define QUEUE_SIZE 10

#define QUEUE_SEND_TIMEOUT 1000 // in ms
#define QUEUE_RECEIVE_TIMEOUT 1000 // in ms

#define TASK1_DELAY 1000
#define TASK2_DELAY 10



QueueHandle_t GlobalQueue = 0;
TaskHandle_t Task1Handle = 0;
TaskHandle_t Task2Handle = 0;

esp_err_t create_tasks(void);
void task1(void *pvParameter);
void task2(void *pvParameter);

esp_err_t create_tasks(void){

    // Create task 1
    xTaskCreate(task1, "task1", STACK_SIZE, NULL, 1, &Task1Handle);
    // Create task 2
    xTaskCreate(task2, "task2", STACK_SIZE, NULL, 1, &Task2Handle);

    return ESP_OK;
}

void task1(void *pvParameter){
    while(1){
        ESP_LOGI(TAG_TASK1, "Task 1");
        for(int i = 0; i < 10; i++){
            // Send to queue with timeout
            if(!xQueueSend(GlobalQueue, &i, QUEUE_SEND_TIMEOUT/portTICK_PERIOD_MS)){ 
                ESP_LOGE(TAG_TASK1, "Failed to send %i to queue", i);
                vTaskResume(Task2Handle);
            }
            else{
                ESP_LOGI(TAG_TASK1, "Sent %i to queue", i);
            }
            vTaskDelay(TASK1_DELAY / portTICK_PERIOD_MS);
            if(i == 5){
                // Suspend task 2
                vTaskSuspend(Task2Handle);
            }
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void task2(void *pvParameter){
    uint8_t value = 0;
    while(1){
        if(xQueueReceive(GlobalQueue, &value, QUEUE_RECEIVE_TIMEOUT/portTICK_PERIOD_MS)){
            ESP_LOGI(TAG_TASK2, "Received %i from queue", value);
        }
        else{
            ESP_LOGE(TAG_TASK2, "Failed to receive from queue");
        }
        vTaskDelay(TASK2_DELAY / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // Create a queue
    GlobalQueue = xQueueCreate(QUEUE_SIZE, sizeof(uint8_t));
    // Check if queue was created
    if(GlobalQueue == 0){
        ESP_LOGE(TAG_MAIN, "Queue creation failed");
    }
    else{
        ESP_LOGI(TAG_MAIN, "Queue creation successful");
    }
    
    // Create tasks
    create_tasks();

    while(1){
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}
