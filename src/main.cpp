#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

// Display pins
#define TFT_CS 5
#define TFT_RST 27
#define TFT_DC 21
#define TFT_MOSI 13
#define TFT_CLK 14
#define TFT_LED 25
#define TFT_MISO 12

// Touch pins
#define TOUCH_CS 26
#define TOUCH_DIN 23
#define TOUCH_DO 19
#define TOUCH_CLK 18

// EEPROM configuration for ESP32
#define EEPROM_SIZE 64
#define CALIBRATION_MAGIC 0xCAFE
#define CALIBRATION_ADDR_MAGIC 0
#define CALIBRATION_ADDR_XMIN 4
#define CALIBRATION_ADDR_XMAX 8
#define CALIBRATION_ADDR_YMIN 12
#define CALIBRATION_ADDR_YMAX 16

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
XPT2046_Touchscreen ts(TOUCH_CS);

bool newCommandReceived = false;
bool lastCommandFulfilled = false;

#define ERASE_BTN_X 200
#define ERASE_BTN_Y 0
#define ERASE_BTN_W 120
#define ERASE_BTN_H 40
#define ROTATION 3

bool calibrated = false;
int xMinVal, xMaxVal, yMinVal, yMaxVal;

void drawCross(int x, int y)
{
  tft.fillCircle(x, y, 8, ILI9341_RED);
  tft.drawLine(x - 15, y, x + 15, y, ILI9341_RED);
  tft.drawLine(x, y - 15, x, y + 15, ILI9341_RED);
  delay(500);
}

TS_Point getTouchPoint()
{
  while (!ts.touched())
    delay(10);
  TS_Point p = ts.getPoint();
  while (ts.touched())
    delay(10);
  return p;
}

void saveCalibrationToEEPROM()
{
  Serial.println("Saving calibration to EEPROM...");
  Serial.printf("Values to save - Xmin:%d Xmax:%d Ymin:%d Ymax:%d\n", xMinVal, xMaxVal, yMinVal, yMaxVal);

  // Write magic number
  EEPROM.writeUShort(CALIBRATION_ADDR_MAGIC, CALIBRATION_MAGIC);

  // Write calibration values
  EEPROM.writeInt(CALIBRATION_ADDR_XMIN, xMinVal);
  EEPROM.writeInt(CALIBRATION_ADDR_XMAX, xMaxVal);
  EEPROM.writeInt(CALIBRATION_ADDR_YMIN, yMinVal);
  EEPROM.writeInt(CALIBRATION_ADDR_YMAX, yMaxVal);

  // Commit changes to flash
  if (EEPROM.commit())
  {
    Serial.println("✓ Calibration data saved to EEPROM successfully");
  }
  else
  {
    Serial.println("✗ Failed to save calibration data to EEPROM");
  }
}

bool loadCalibrationFromEEPROM()
{
  Serial.println("Attempting to load calibration from EEPROM...");

  // Read magic number
  uint16_t magic = EEPROM.readUShort(CALIBRATION_ADDR_MAGIC);
  Serial.printf("Magic number read: 0x%04X (expected: 0x%04X)\n", magic, CALIBRATION_MAGIC);

  if (magic == CALIBRATION_MAGIC)
  {
    // Read calibration values exactly as saved
    xMinVal = EEPROM.readInt(CALIBRATION_ADDR_XMIN);
    xMaxVal = EEPROM.readInt(CALIBRATION_ADDR_XMAX);
    yMinVal = EEPROM.readInt(CALIBRATION_ADDR_YMIN);
    yMaxVal = EEPROM.readInt(CALIBRATION_ADDR_YMAX);

    Serial.printf("Loaded values - Xmin:%d Xmax:%d Ymin:%d Ymax:%d\n", xMinVal, xMaxVal, yMinVal, yMaxVal);

    // Simple validation - just check they're non-zero
    if (xMinVal != 0 && xMaxVal != 0 && yMinVal != 0 && yMaxVal != 0)
    {
      calibrated = true;
      Serial.println("✓ Calibration data loaded from EEPROM");
      return true;
    }
    else
    {
      Serial.println("✗ Loaded calibration data contains zeros");
      calibrated = false;
    }
  }
  else
  {
    Serial.println("✗ No valid calibration magic number found");
    calibrated = false;
  }

  return false;
}

void calibrateScreen()
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(20, 140);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Touch the corners");

  // Corner 1: top left
  drawCross(20, 20);
  TS_Point p = getTouchPoint();
  xMinVal = p.x;
  yMinVal = p.y;
  Serial.printf("Top-left - X:%d Y:%d\n", xMinVal, yMinVal);

  // Corner 2: top right
  drawCross(tft.width() - 20, 20);
  p = getTouchPoint();
  xMaxVal = p.x;
  Serial.printf("Top-right - X:%d\n", xMaxVal);

  // Corner 3: bottom left
  drawCross(20, tft.height() - 20);
  p = getTouchPoint();
  yMaxVal = p.y;
  Serial.printf("Bottom-left - Y:%d\n", yMaxVal);

  calibrated = true;

  // Save the calibration data automatically
  saveCalibrationToEEPROM();

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(20, 140);
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(2);
  tft.print("Calibration Complete!");
  delay(1500);
}

void drawEraseBtn()
{
  tft.fillRect(ERASE_BTN_X, ERASE_BTN_Y, ERASE_BTN_W, ERASE_BTN_H, ILI9341_BLUE);
  tft.setCursor(ERASE_BTN_X + 10, ERASE_BTN_Y + 10);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.print("ERASE ALL");
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(9600);

  delay(1000);
  Serial.println("\n\n=== ESP32 Touch Screen Starting ===");

  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  tft.begin();
  ts.begin();
  ts.setRotation(ROTATION);
  tft.setRotation(ROTATION);

  // Initialize EEPROM with size
  EEPROM.begin(EEPROM_SIZE);
  Serial.printf("EEPROM initialized with size: %d bytes\n", EEPROM_SIZE);

  drawEraseBtn();

  // Try to load existing calibration
  if (!loadCalibrationFromEEPROM())
  {
    Serial.println("No valid calibration found, starting calibration...");
    delay(2000);
    calibrateScreen();
  }
  else
  {
    Serial.println("Using saved calibration data");
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(20, 140);
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextSize(2);
    tft.print("Calibration Loaded!");
    delay(1500);
  }

  drawEraseBtn();

  Serial.println("Setup complete, ready for touch input");
}

int command = -1;

int getCommandFromSerial()
{
  if (Serial2.available())
  {
    String msg = Serial2.readStringUntil('\n');
    int index = msg.indexOf(':');

    if (index != -1)
    {
      String num = msg.substring(index + 1);
      num.trim();
      return num.toInt();
    }
  }
  return -1;
}

void processCommand(int cmd)
{
  switch (cmd)
  {
  case 1:
    Serial.println("Recalibration command received");
    calibrateScreen();
    drawEraseBtn();
    break;
  case 2:
    Serial.println("Executing command 2");
    break;
  default:
    break;
  }
}

void processTouch()
{
  if (!ts.touched())
    return;

  TS_Point raw = ts.getPoint();

  // Map raw to screen using the calibration values directly
  int sx = map(raw.x, xMinVal, xMaxVal, 0, tft.width());
  int sy = map(raw.y, yMinVal, yMaxVal, 0, tft.height());

  // Bound check
  if (sx < 0)
    sx = 0;
  if (sy < 0)
    sy = 0;
  if (sx > tft.width())
    sx = tft.width();
  if (sy > tft.height())
    sy = tft.height();

  // Erase button
  if (sx >= ERASE_BTN_X && sx <= ERASE_BTN_X + ERASE_BTN_W &&
      sy >= ERASE_BTN_Y && sy <= ERASE_BTN_Y + ERASE_BTN_H)
  {
    tft.fillScreen(ILI9341_BLACK);
    drawEraseBtn();
    delay(200);
    return;
  }

  // Draw point
  tft.fillCircle(sx, sy, 2, ILI9341_YELLOW);
  delay(10);
}

void loop()
{
  command = getCommandFromSerial();

  if (command != -1)
  {
    Serial.print("Received command: ");
    Serial.println(command);
    newCommandReceived = true;
  }

  if (newCommandReceived)
  {
    processCommand(command);
    newCommandReceived = false;
  }

  if (lastCommandFulfilled)
  {
    Serial.println("Last command fulfilled, ready for next command.");
    command = -1;
  }

  processTouch();
}