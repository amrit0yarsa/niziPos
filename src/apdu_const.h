#ifndef APDU_CONST_H
#define APDU_CONST_H

#include <Arduino.h>

uint8_t SELECT_APDU[] = {
  0x00,                                     /* CLA */
  0xA4,                                     /* INS */
  0x04,                                     /* P1  */
  0x00,                                     /* P2  */
  0x07,                                     /* Length of AID  */
  // 0xF2, 0x60, 0x06, 0x09, 0x64, 0x05, 0x39, /* AID F2600609640539 (fonepay) */
  0xF0, 0x01, 0x09, 0x09, 0x07, 0x09, 0x01,
  0x00                                      /* Le  */
};

uint8_t SUCCESS_RESPONSE[] = {
  0x90, /* response */
  0x00, /* SW */
};

#endif