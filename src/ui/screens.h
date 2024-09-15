#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *result;
    lv_obj_t *container_icon_res;
    lv_obj_t *container_qr;
    lv_obj_t *icon_checked;
    lv_obj_t *icon_failed;
    lv_obj_t *label_amount_main;
    lv_obj_t *label_amount_result;
    lv_obj_t *label_payment_res;
    lv_obj_t *label_wait;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_RESULT = 2,
};

void create_screen_main();
void tick_screen_main();

void create_screen_result();
void tick_screen_result();

void create_screens();
void tick_screen(int screen_index);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/