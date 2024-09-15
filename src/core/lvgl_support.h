#ifndef LVGL_SUPPORT_H
#define LVGL_SUPPORT_H

#include "lvgl.h"

// display includes
#include "Arduino_ST7789.h"
#include "SPI.h"


enum {
    UI_RESULT_DEVICE_ERROR,
    UI_RESULT_PAYMENT_SUCCESS,
    UI_RESULT_PAYMENT_FAILURE,
    UI_RESULT_SCANNER_TIMEOUT
};

void initTFT();
void initLVGL();
void lvglTick();
void setRotation(lv_display_t *disp, lv_display_rotation_t rotation);
void sleepDisplay(bool shouldSleep);

lv_obj_t* create_screen();
void ui_displayQR(const char *data);

void ui_updateAmount(const char *amount);
void ui_updateWaitInfo(const char *info);
void ui_updateResult(uint8_t resultType);
void displayQR(const char *data);

extern Arduino_ST7789 tft;

#endif