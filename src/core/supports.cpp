#include "supports.h"

void initPSRAM() {
  if (!psramInit())
  {
    // Serial.println("PSRAM Init Failed");
  }
  while (1)
  {
    int a = ESP.getFreePsram();
    if (a)
    {
      break;
    }
    delay(10);
  }
  // Serial.println("PSRAM Ready");
}