#include <Arduino.h>
#include <WiFi.h>

#include "core/supports.h"
#include "core/pinouts.h"
#include "core/lvgl_support.h"

// include commands
#include "command_defs.h"

#include "apdu_const.h"

// include eez studio generated code
#include "ui/ui.h"
#include "ui/actions.h"
#include "ui/screens.h"

// include drivers
#include "drivers/driver_buzzer.h"
#include "drivers/driver_camera.h"
#include "drivers/driver_nfc.h"

// include services
#include "core/services.h"

#define USER_TIMEOUT 35000

// globals related to message received from the scannerPoll
uint8_t response[255] = {0};
uint8_t responseLength = sizeof(response);
char amountGlobal[50];
char qrDataGlobal[257];

// store temporarily
// i need to make it global as nfc is not working for local ones. I don't know why
// uint8_t responseTemp[255] = {0};
// uint8_t responseLengthTemp = sizeof(responseTemp);

uint8_t back[2];
uint8_t backLength = sizeof(back);

void initializeDrivers();
void hostMessageHandler();
void scannerPoll();
void lvglProcess();

// receiving scanner data
// @returns true if data is available to read
static bool nfcReceive();

// receiving scanner data
// @returns true if data is available to read
static bool qrReceive();

static bool decodeAmountandQR(const char *message, char *amount, char *qrData);

static void resetDevice();

lv_obj_t *scr;

void setup()
{

  Serial.begin(115200);
  while (!Serial)
    ;

  initPSRAM();

  // Disable Wi-Fi to save power
  WiFi.disconnect(true); // Disconnect and clear configuration
  WiFi.mode(WIFI_OFF);   // Turn off Wi-Fi mode

  initTFT();
  tft.fillScreen(BLUE);

  initLVGL();

  // initialize the eez ui
  ui_init();
  loadScreen(SCREEN_ID_MAIN);

  initializeDrivers();

  // start services
  serviceHostCommBegin();

  sleepDisplay(true);

  // displayHomeQR(objects.container_qr);
}

void loop()
{
  lvglTick();
  lvglProcess();

  // host message handling
  if (hostMessageArrived())
  {
    hostMessageHandler();
  }

  if (scannerState != SCANNER_STATE_IDLE) {
    // scanner state is something else
    if (scannerState == SCANNER_STATE_SCANNED) {
      hostSendMessage(DATA_RTS);

      ui_updateWaitInfo("Payment\nProcessing...");
    } else if (scannerState == SCANNER_STATE_TIMEOUT) {
      hostSendMessage(TIME_OUT);

      ui_updateResult(UI_RESULT_SCANNER_TIMEOUT);
    }
    
    scannerState = SCANNER_STATE_IDLE;
  }
}

void initializeDrivers()
{
  initBuzzer();
  initCamera();
  initNFC(); // @todo: what to do if nfc module is not working?
}

void hostMessageHandler()
{
  // make a copy so that service will be released to receive another message
  String message = hostMessageRead();
  if (message == ASK_DEVICE_ID)
  {
    // Send DATA_PAYLOAD when requested by  IMS
    hostSendMessage(DEVICE_ID);
  }
  else if (message == HEARTBEAT_CMD)
  {
    // Send HEARTBEAT_ACK to the driver
    hostSendMessage(HEARTBEAT_ACK);
  }
  else if (message.startsWith("INIT_NFC,"))
  {
    if (isNfcWorking())
    {
      hostSendMessage(INIT_NFC_ACK);

      // decode the message
      bool ret = decodeAmountandQR(message.c_str(), amountGlobal, qrDataGlobal);
      if (ret)
      {
        // @todo display amount and qr
        // Serial.printf("amount: %s, qrData: %s\n", amountGlobal, qrDataGlobal);
        beep();

        // update ui
        ui_updateWaitInfo("Waiting for\nPayment...");
        ui_updateAmount(amountGlobal);
        ui_displayQR(qrDataGlobal);

        sleepDisplay(false);

        bool isScannerFree = serviceScannerStart(scannerPoll);
        if (!isScannerFree)
        {
          hostSendMessage(ERROR_SCANNER_BUSY);
        }
      }
      else
      {
        memset(amountGlobal, 0, sizeof(amountGlobal));
        memset(qrDataGlobal, 0, sizeof(qrDataGlobal));
        hostSendMessage(ERROR_DECODE);
        beepError();
        ui_updateResult(UI_RESULT_DEVICE_ERROR);
        sleepDisplay(false);
      }
    } else {
      hostSendMessage(ERROR_NFC_INIT);
      ui_updateResult(UI_RESULT_DEVICE_ERROR);
      sleepDisplay(false);
    }
  }
  else if (message == DEVICE_RESET)
  {
    resetDevice();
  }
  else if (message == DEVICE_RESTART)
  {
    ESP.restart();
  }
  else if (message == DATA_RTR)
  {
    hostSendMessage((char *)response);
  }
  else if (message == DATA_ACK)
  {
    memset(response, 0, sizeof(response));
    responseLength = sizeof(response);
  }
  else if (message == PAYMENT_SUCCESS)
  {
    serviceScannerStop();
    beepSuccess();
    ui_updateResult(UI_RESULT_PAYMENT_SUCCESS);
    sleepDisplay(false);
  }
  else if (message == PAYMENT_FAILURE)
  {
    serviceScannerStop();
    beepFailure();
    ui_updateResult(UI_RESULT_PAYMENT_FAILURE);
    sleepDisplay(false);
  }
  else
  {
    hostSendMessage(INVALID_COMMAND);
  }
}

void scannerPoll()
{
  qrFlush(); // flush previous data if available in the buffer before reading again

  scannerState = SCANNER_STATE_IDLE;

  // copy the temporary response to the global response
  memset(response, 0, sizeof(response));
  responseLength = sizeof(response);

  float timestamp = millis();
  // start polling the nfc and qr data
  while ((millis() - timestamp) < USER_TIMEOUT)
  {
    if (nfcReceive() || qrReceive())
    {
      scannerState = SCANNER_STATE_SCANNED;
      break;
    }
  }

  if ((millis() - timestamp) > USER_TIMEOUT)
  {
    scannerState = SCANNER_STATE_TIMEOUT;

    memset(response, 0, sizeof(response));
    responseLength = sizeof(response);
  }
}

// update graphics here
void lvglProcess()
{
}

static bool nfcReceive()
{
  uint8_t responseTemp[255] = {0};
  uint8_t responseLengthTemp = sizeof(responseTemp);
  memset(responseTemp, 0, sizeof(responseTemp));

  if (isNfcDetected())
  {
    if (sendNfcData(SELECT_APDU, sizeof(SELECT_APDU), responseTemp, &responseLengthTemp))
    {
      // Serial.printf("data; %s, %d\n", (char *)responseTemp, responseLengthTemp);
      if (responseLengthTemp > 1)
      {
        responseTemp[responseLengthTemp - 2] = '\0';
        responseTemp[responseLengthTemp - 1] = '\0';

        if (responseLengthTemp >= 16 && memcmp(responseTemp, "{\"data\":\"", 9) == 0)
        {
          // copy the temporary response to the global response
          memcpy(response, responseTemp, sizeof(response));
          responseLength = responseLengthTemp;

          sendNfcData(SUCCESS_RESPONSE, sizeof(SUCCESS_RESPONSE), back, &backLength);
          beep();

          return true;
        }
        else
        {
          hostSendMessage(ERROR_FORMAT);
          beepError();
          ui_updateResult(UI_RESULT_DEVICE_ERROR);

          // @todo on payload format error - display

          // error has occured stop the scanner service
          serviceScannerStop();

          return false;
        }
      }
      else
      {
        return false;
      }
    }
    else
    {
      hostSendMessage(SELECT_APDU_FAILED);
      return false;
    }
  }

  return false;
}

static bool qrReceive()
{
  if (isQrDataAvailable())
  {

    uint8_t responseTemp[255] = {0};
    uint8_t responseLengthTemp = sizeof(responseTemp);
    memset(responseTemp, 0, sizeof(responseTemp));

    qrDataRead(responseTemp, &responseLengthTemp);
    // Serial.printf("data: \"%s\"\n", (char *)responseTemp);

    beep();

    if (responseLengthTemp >= 16 && memcmp(responseTemp, "{\"data\":\"", 9) == 0)
    {
      // copy the temporary response to the global response
      memcpy(response, responseTemp, sizeof(response));
      responseLength = responseLengthTemp;

      return true;
    }
    else
    {
      hostSendMessage(ERROR_FORMAT);

      beepError();
      ui_updateResult(UI_RESULT_DEVICE_ERROR);

      // @todo on payload format error - display

      // error has occured stop the scanner service
      serviceScannerStop();

      return false;
    }
  }
  else
  {
    return false;
  }
}

static bool decodeAmountandQR(const char *message, char *amount, char *qrData)
{
  char *firstComma = strstr(message, ",");
  if (firstComma)
  {
    char *secondComma = strstr(firstComma + 1, ",");
    if (secondComma)
    {
      strncpy(amount, firstComma + 1, secondComma - firstComma - 1);
      amount[secondComma - firstComma - 1] = '\0';
      strcpy(qrData, secondComma + 1);
      return true;
    }
  }
  return false;
}

static void resetDevice()
{
  memset(response, 0, sizeof(response));
  responseLength = sizeof(response);

  memset(amountGlobal, 0, sizeof(amountGlobal));
  memset(qrDataGlobal, 0, sizeof(qrDataGlobal));

  serviceScannerStop(); // stops the scanner service if it is running

  sleepDisplay(true);
}