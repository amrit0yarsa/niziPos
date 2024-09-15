#ifndef DRIVER_CAMERA_H
#define DRIVER_CAMERA_H

#include <Arduino.h>
#include "core/pinouts.h"

void initCamera();
bool isQrDataAvailable();
void qrDataRead(uint8_t *_response, uint8_t *_responseLength);
void qrFlush();

#endif