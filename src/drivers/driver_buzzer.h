#ifndef DRIVER_BUZZER_H
#define DRIVER_BUZZER_H

#include <Arduino.h>
#include "core/pinouts.h"

void initBuzzer();
void beep();
void beepSuccess();
void beepFailure();
void beepError();

#endif