#include "services.h"

#include "core/lvgl_support.h"

#define SERVICE_NAME_HOST_COMM "HostComm Service"
#define SERVICE_STACKSIZE_HOST_COMM (4 * 1024)

#define SERVICE_NAME_SCANNER "Scanner Service"
#define SERVICE_STACKSIZE_SCANNER (4 * 1024)

#define SERVICE_NAME_LVGL "LVGL Service"
#define SERVICE_STACKSIZE_LVGL (20 * 1024)

// task handlers for different services
static TaskHandle_t taskHandle_serviceScanner = NULL;

static void hostCommService_Func(void *parameters);
void serviceScanner_func(void *parameter);
void serviceLVGL_func(void *parameter);

uint8_t scannerState = SCANNER_STATE_IDLE;

static String receivedHostMsg; // Variable to store received messages
static bool newMsgArrived = false;

// starts a host communication service that can run in background
void serviceHostCommBegin()
{
    xTaskCreate(hostCommService_Func, SERVICE_NAME_HOST_COMM, SERVICE_STACKSIZE_HOST_COMM, NULL, 0, NULL);
}

// starts a scanner service that can run in background
bool serviceScannerStart(void (*_scannerPoll)())
{
    if (taskHandle_serviceScanner == NULL)
    {
        xTaskCreatePinnedToCore(
            serviceScanner_func,        // Task function
            SERVICE_NAME_SCANNER,       // Name of the task (for debugging)
            SERVICE_STACKSIZE_SCANNER,                       // Stack size (in bytes)
            (void *)_scannerPoll,       // Task input parameter
            1,                          // Task priority
            &taskHandle_serviceScanner, // Task handle
            0
        );

        return true;
    }
    else
    {
        return false;
    }
}

void serviceScannerStop()
{
    if (taskHandle_serviceScanner != NULL)
    {
        vTaskDelete(taskHandle_serviceScanner);
        if (eTaskGetState(taskHandle_serviceScanner) == eDeleted)
        {
            scannerState = SCANNER_STATE_IDLE;
            taskHandle_serviceScanner = NULL;
        }
    }
}

// Function for the task that does processing
void serviceScanner_func(void *parameter)
{
    void (*scannerPoll)() = (void (*)())parameter;

    // Serial.println("Task started");

    scannerPoll();

    // Serial.println("Task finished");

    // Task finished, delete the task
    // taskHandle_serviceScanner = NULL;
    vTaskDelete(NULL);
}

void serviceProcess()
{
    // Clear the task handle when task is finished
    if (taskHandle_serviceScanner != NULL && eTaskGetState(taskHandle_serviceScanner) == eDeleted)
    {
        taskHandle_serviceScanner = NULL;
    }
}

static void hostCommService_Func(void *parameters)
{
    while (1)
    {
        // wait until recent message is read before reading next data
        if (!newMsgArrived && Serial.available() > 0)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
            receivedHostMsg = Serial.readStringUntil('\n');
            newMsgArrived = true;
        }

        // process service related tasks
        serviceProcess();


        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// check if message is arrived from the host
// @return true on message arrival
bool hostMessageArrived()
{
    return newMsgArrived;
}

// read the message received from the host
// @return message : String
String hostMessageRead()
{
    newMsgArrived = false;
    return receivedHostMsg;
}

// send message to the host
void hostSendMessage(const char *message)
{
    Serial.println(message);
}
