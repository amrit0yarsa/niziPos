#ifndef SERVICES_H
#define SERVICES_H

#include <Arduino.h>

extern uint8_t scannerState;

enum {
    SCANNER_STATE_IDLE,
    SCANNER_STATE_TIMEOUT,
    SCANNER_STATE_SCANNED
};

void serviceHostCommBegin();
bool hostMessageArrived();
String hostMessageRead();
void hostSendMessage(const char *message);

void serviceProcess();
bool serviceScannerStart(void (*_scannerPoll)());
void serviceScannerStop();

#endif