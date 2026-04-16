#include <Arduino.h>
#include "TFT_Handler.h"
#include "ProjectConfig.h"
#include "ButtonManager.h"

// Touch feedback control
#define ENABLE_TOUCH_FEEDBACK true

TFT_Handler tft;
ButtonManager buttonManager(tft);

void setup()
{
  Serial.begin(115200);
  delay(1000);

  tft.begin();
  tft.enableTouchFeedback(ENABLE_TOUCH_FEEDBACK);

  // Draw splash screen
  tft.drawSplash(
      SCHOOL_INFO,
      COLLEGE,
      COURSE,
      PROJECT_INFO,
      DEVELOPERS,
      DEVELOPER_COUNT);

  delay(5000);
  // tft.clear();
}

void loop()
{
  // Your main code here
  delay(20);
}