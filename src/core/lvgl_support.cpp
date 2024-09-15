#include "lvgl_support.h"
#include "pinouts.h"

#include "ui/screens.h"
#include "ui/ui.h"

/* LVGL GLOBAL */
/*Set to your screen resolution and rotation*/
#define TFT_HOR_RES 240
#define TFT_VER_RES 320
#define TFT_ROTATION LV_DISPLAY_ROTATION_270

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))

static void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
static uint32_t my_tick(void);
static void ui_updateResultIcon(lv_obj_t *icon);
static void ui_updateResultScreenData(const char *errorText, lv_obj_t *icon);

// uint32_t draw_buf[DRAW_BUF_SIZE / 4];
void *draw_buf;

SPIClass hspi = SPIClass(HSPI);
Arduino_ST7789 tft = Arduino_ST7789(TFT_DC, TFT_RST, TFT_CS, &hspi); // for display with CS pin

void initTFT()
{
    // turn on backlight of the tft display
    pinMode(LCD_BACKLIGHT, OUTPUT);
    digitalWrite(LCD_BACKLIGHT, HIGH);

    tft.init(240, 320); // initialize a ST7789 chip, 240x320 pixels
}

void initLVGL()
{
    lv_init();

    /*Set a tick source so that LVGL will know how much time elapsed. */
    lv_tick_set_cb(my_tick);

    // draw_buf = heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    draw_buf = heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_SPIRAM);

    lv_display_t *disp;
    disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
    setRotation(disp, TFT_ROTATION);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_buf, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
}

unsigned int lastTickMillis = 0;
void lvglTick()
{
    unsigned int tickPeriod = millis() - lastTickMillis;
    lv_tick_inc(tickPeriod);
    lastTickMillis = millis();

    lv_timer_handler(); /* let the GUI do its work */
}

void setRotation(lv_display_t *disp, lv_display_rotation_t rotation)
{
    lv_display_set_rotation(disp, rotation);
    tft.setRotation(rotation);
}

/* LVGL calls it when a rendered image needs to copied to the display*/
static void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint16_t *color = (uint16_t *)px_map;

    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);
    uint32_t wh = w * h;

    tft.setAddrWindow(area->x1, area->y1, area->x2, area->y2);
    tft.startWrite();
    tft.pushDataMultiple(color, wh);
    // while (wh--) tft.pushData(*color++);
    tft.endWrite();

    /*Call it to tell LVGL you are ready*/
    lv_display_flush_ready(disp);
}

/*use Arduinos millis() as tick source*/
static uint32_t my_tick(void)
{
    return millis();
}

void ui_displayQR(const char *data)
{
    lv_obj_clean(objects.container_qr);

    /* generate qr code */
    lv_obj_t *qr = lv_qrcode_create(objects.container_qr);

    lv_obj_update_layout(objects.container_qr);
    int width = lv_obj_get_width(objects.container_qr);

    lv_qrcode_set_dark_color(qr, lv_color_hex(0x000000));
    lv_qrcode_set_light_color(qr, lv_color_hex(0xffffff));
    lv_qrcode_set_size(qr, width - 5);

    // const char *data = "000201010212153137910524005204460000000NBQM:29226400011fonepay.com0104NBQM020329206061367695204541153035245402145802NP5911Fonepaytest6008District622107032 92021098418456336304";
    // const char *data = "th0mtq8rv2d-fufk5:4b3qeg4r43yaj48m4frzek97zqcyyrux4983at5n7ym-y19x:cmmds0aweiuded6l5gi5a72oz8z-w21s1hf46ezmx-0i8pwvifrjz20f50q7g:g0g1u5aoe7mox84-slze54buyp80y0v9qdh3b9:czcgxjdtnen:tjyy9uu5i4dh2592ty2dk03dqq5:r7fsmv8i948-5r3pe82k-e-qubwk2pxc8uvluptta5lacy0m";
    lv_qrcode_update(qr, data, strlen(data));
    lv_obj_center(qr);
}

/*
void displayQRAnimation(lv_obj_t *parent) {
    lv_obj_t *obj = lv_animimg_create(parent);
    lv_obj_set_pos(obj, 0, 0);

    lv_obj_update_layout(parent);
    int width = lv_obj_get_width(parent);
    int height = lv_obj_get_height(parent);

    lv_obj_set_size(obj, width, height);
    static const lv_image_dsc_t *images[3] = {
        &img1,
        &img30,
        &img100
    };
    lv_animimg_set_src(obj, (const void **)images, 3);
    lv_animimg_set_duration(obj, 1000);
    lv_animimg_set_repeat_count(obj, LV_ANIM_REPEAT_INFINITE);
    lv_animimg_start(obj);
}
*/

// update display ui
void ui_updateAmount(const char *amount)
{

    char amountWithRS[50];
    sprintf(amountWithRS, "Rs. %s", amount);

    lv_label_set_text(objects.label_amount_main, amountWithRS);
    lv_label_set_text(objects.label_amount_result, amountWithRS);
}

void ui_updateWaitInfo(const char *info)
{

    if (lv_screen_active() != objects.main)
    {
        loadScreen(SCREEN_ID_MAIN);
    }
    lv_label_set_text(objects.label_wait, info);
}

void ui_updateResult(uint8_t resultType)
{

    if (lv_screen_active() != objects.result)
    {
        loadScreen(SCREEN_ID_RESULT);
    }
    switch (resultType)
    {
    case UI_RESULT_DEVICE_ERROR:
    {
        ui_updateResultScreenData("Error", objects.icon_failed);
        lv_label_set_text(objects.label_amount_result, "Device Failure");
        break;
    }
    case UI_RESULT_PAYMENT_SUCCESS:
    {
        ui_updateResultScreenData("Payment Success", objects.icon_checked);
        break;
    }
    case UI_RESULT_PAYMENT_FAILURE:
    {
        ui_updateResultScreenData("Payment Failure", objects.icon_failed);
        break;
    }
    case UI_RESULT_SCANNER_TIMEOUT:
    {
        ui_updateResultScreenData("ERROR", objects.icon_failed);
        lv_label_set_text(objects.label_amount_result, "Scanner Timeout");
        break;
    }
    }
}

void sleepDisplay(bool shouldSleep) {
    digitalWrite(LCD_BACKLIGHT, !shouldSleep);
}

static void ui_updateResultIcon(lv_obj_t *icon)
{
    if (icon == objects.icon_checked)
    {
        lv_obj_add_flag(objects.icon_failed, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(objects.icon_checked, LV_OBJ_FLAG_HIDDEN);
    }
    else if (icon == objects.icon_failed)
    {
        lv_obj_add_flag(objects.icon_checked, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(objects.icon_failed, LV_OBJ_FLAG_HIDDEN);
    }
}

static void ui_updateResultScreenData(const char *errorText, lv_obj_t *icon)
{
    lv_label_set_text(objects.label_payment_res, errorText);

    // update container
    ui_updateResultIcon(icon);
}