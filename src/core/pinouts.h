#ifndef PINOUTS_H

#define PINOUT_NIZIPOS

/* -- Pins for TFT Display -- */
#ifdef PINOUT_ATTEND
#define TFT_CS 25        // Chip select control pin - 23
#define TFT_DC 26        // Data Command control pin - 15
#define TFT_RST 33       // Reset pin (could connect to RST pin) -33
#define LCD_BACKLIGHT 32 // backlight
#endif

#ifdef PINOUT_NIZIPOS
#define TFT_CS 18        // Chip select control pin - 23
#define TFT_DC 15        // Data Command control pin - 15
#define TFT_RST 33       // Reset pin (could connect to RST pin) -33
#define LCD_BACKLIGHT 32 // backlight
#endif

/* -- Pins for NFC module -- */
#define PN532_SCK (2)
#define PN532_MOSI (23)
#define PN532_SS (5)
#define PN532_MISO (19)

/* -- Pins for NFC module -- */
#define CAMERA_TRIGGER 21 //14      21
#define CAMERA_TX 4       //17      4
#define CAMERA_RX 4      //16       16

/* -- Pins for Buzzer -- */
#define BUZZER_PIN 22

#endif