#include "driver_camera.h"

#define BAUDRATE_CAMERA 9600

void initCamera()
{
    Serial2.begin(BAUDRATE_CAMERA, SERIAL_8N1, CAMERA_RX, CAMERA_TX);
    Serial2.setTimeout(250);
}

bool isQrDataAvailable()
{
    if (Serial2.available() > 0)
    {
        delay(10);
        return true;
    }
    else
    {
        return false;
    }
}

void qrDataRead(uint8_t *_response, uint8_t *_responseLength)
{
    String tempResponse = Serial2.readStringUntil('\n');
    // uint8_t response[tempResponse.length()];  // Add 1 for null termination

    tempResponse.getBytes(_response, tempResponse.length()); // Null-terminate the array
    *_responseLength = tempResponse.length();
}

void qrFlush() {
  Serial2.flush();  // don't know but chatGPT says that it only flushes the outgoing messages
  while (Serial2.available() > 0) {
    char c = Serial2.read();  // Read and discard the character
  }
}