#ifndef DRIVER_NFC_H
#define DRIVER_NFC_H

#include <Arduino.h>
#include "core/pinouts.h"

bool initNFC();
bool isNfcDetected();
bool isNfcWorking();
bool sendNfcData(uint8_t *apduMessage, uint8_t apduMessageLength, uint8_t *_response, uint8_t *_responseLength);

#endif