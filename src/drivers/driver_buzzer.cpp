#include "driver_buzzer.h"

void initBuzzer() {
    // pinMode(BUZZER_PIN, OUTPUT);

    // initialize ledc
    ledcSetup(0, 2000, 8);
    ledcAttachPin(BUZZER_PIN, 0);
}

void beep() {
  tone(BUZZER_PIN, 1000, 50);
  vTaskDelay(pdMS_TO_TICKS(50));
  // tone(BUZZER_PIN, 2200, 75);
  // vTaskDelay(pdMS_TO_TICKS(100));
  tone(BUZZER_PIN, 1800, 150);
  vTaskDelay(pdMS_TO_TICKS(100));
}

void beepSuccess() {
  tone(BUZZER_PIN, 587, 110);
  tone(BUZZER_PIN, 740, 110);
  tone(BUZZER_PIN, 880, 110);
  tone(BUZZER_PIN, 1175, 110);
}

void beepFailure() {
  tone(BUZZER_PIN, 200, 70);
  vTaskDelay(pdMS_TO_TICKS(100));
  tone(BUZZER_PIN, 200, 250);
  vTaskDelay(pdMS_TO_TICKS(500));
}

void beepError() {
  tone(BUZZER_PIN, 200, 70);
  vTaskDelay(pdMS_TO_TICKS(100));
  tone(BUZZER_PIN, 200, 250);
  vTaskDelay(pdMS_TO_TICKS(500));
}