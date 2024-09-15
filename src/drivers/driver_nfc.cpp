#include "driver_nfc.h"

#include "Adafruit_PN532.h"

static bool chipFound = false; // bool to track PN532 chip presence

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

bool initNFC()
{
    bool ret = true;
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();

    if (!versiondata)
    {
        ret = false;
        chipFound = false;
    }
    else
    {
        // Serial.println("SAM Config");
        nfc.SAMConfig();
        chipFound = true;
    }

    return ret;
}

bool isNfcWorking()
{
    return chipFound;
}

bool sendNfcData(uint8_t *apduMessage, uint8_t apduMessageLength, uint8_t *_response, uint8_t *_responseLength)
{
    // Serial.print("APDU:");
    // for (int i = 0; i < apduMessageLength; i++)
    // {
    //     Serial.printf(" %02x", apduMessage[i]);
    // }
    // Serial.println();
    bool success = nfc.inDataExchange(apduMessage, apduMessageLength, _response, _responseLength);
    return success;
}

bool isNfcDetected()
{
    return nfc.inListPassiveTarget();
}