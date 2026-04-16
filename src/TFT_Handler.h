#ifndef TFT_HANDLER_H
#define TFT_HANDLER_H

#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

// Pins
#define TFT_CS 5
#define TFT_RST 27
#define TFT_DC 21
#define TFT_MOSI 13
#define TFT_CLK 14
#define TFT_LED 25
#define TFT_MISO 12
#define TOUCH_CS 26

// EEPROM
#define EEPROM_SIZE 64
#define CAL_MAGIC 0xCAFE
#define CAL_ADDR_MAGIC 0
#define CAL_ADDR_XMIN 4
#define CAL_ADDR_XMAX 8
#define CAL_ADDR_YMIN 12
#define CAL_ADDR_YMAX 16

// UI
#define ERASE_BTN_X 200
#define ERASE_BTN_Y 0
#define ERASE_BTN_W 120
#define ERASE_BTN_H 40
#define TOUCH_ROTATION 3
#define BTN_DEBOUNCE_MS 50

#define TOUCH_FEEDBACK_RADIUS 6
#define TOUCH_FEEDBACK_COLOR ILI9341_BLUE
#define TOUCH_FEEDBACK_DELAY_MS 60

// Struct definitions
struct School
{
    const char *name;
    const char *campus;
};

struct Project
{
    const char *nickname;
    const char *fullname;
    const char *version;
};

class TFT_Handler
{
public:
    TFT_Handler()
        : _tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO),
          _ts(TOUCH_CS), _calibrated(false), _lastTouch(0), _touchFeedback(true) {}

    bool begin()
    {
        pinMode(TFT_LED, OUTPUT);
        digitalWrite(TFT_LED, HIGH);
        _tft.begin();
        _tft.setRotation(TOUCH_ROTATION);
        _ts.begin();
        _ts.setRotation(TOUCH_ROTATION);
        EEPROM.begin(EEPROM_SIZE);
        if (!_loadCalibration())
            calibrate();
        return _calibrated;
    }

    void calibrate()
    {
        _tft.fillScreen(ILI9341_BLACK);
        _tft.setCursor(20, 140);
        _tft.setTextColor(ILI9341_WHITE);
        _tft.setTextSize(2);
        _tft.print("TOUCH THE CORNERS");
        _drawCross(20, 20);
        TS_Point p = _waitTouch();
        _xMin = p.x;
        _yMin = p.y;
        _drawCross(_tft.width() - 20, 20);
        p = _waitTouch();
        _xMax = p.x;
        _drawCross(20, _tft.height() - 20);
        p = _waitTouch();
        _yMax = p.y;
        _calibrated = true;
        _saveCalibration();
        _tft.fillScreen(ILI9341_BLACK);
        _tft.setCursor(20, 140);
        _tft.setTextColor(ILI9341_GREEN);
        _tft.print("CALIBRATION COMPLETE!");
        delay(1500);
    }

    void clear(uint16_t color = ILI9341_BLACK) { _tft.fillScreen(color); }

    bool isTouched() { return _ts.touched(); }

    bool getTouchPoint(int16_t &x, int16_t &y)
    {
        if (!_ts.touched())
            return false;
        uint32_t now = millis();
        if (now - _lastTouch < BTN_DEBOUNCE_MS)
            return false;
        _lastTouch = now;
        TS_Point raw = _ts.getPoint();
        if (!_calibrated)
        {
            x = raw.x;
            y = raw.y;
        }
        else
        {
            _mapTouch(raw.x, raw.y, x, y);
        }
        if (_touchFeedback)
            showTouchFeedback(x, y);
        return true;
    }

    void showTouchFeedback(int16_t x, int16_t y)
    {
        fillCircle(x, y, TOUCH_FEEDBACK_RADIUS, TOUCH_FEEDBACK_COLOR);
        delay(TOUCH_FEEDBACK_DELAY_MS);
    }

    bool isButtonPressed(int16_t bx, int16_t by, int16_t bw, int16_t bh)
    {
        int16_t tx, ty;
        if (!getTouchPoint(tx, ty))
            return false;
        return (tx >= bx && tx <= bx + bw && ty >= by && ty <= by + bh);
    }

    void enableTouchFeedback(bool enable) { _touchFeedback = enable; }

    // Drawing primitives
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { _tft.fillRect(x, y, w, h, color); }
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { _tft.drawRect(x, y, w, h, color); }
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) { _tft.drawLine(x0, y0, x1, y1, color); }
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) { _tft.fillCircle(x, y, r, color); }
    void setTextSize(uint8_t s) { _tft.setTextSize(s); }
    void setTextColor(uint16_t fg, uint16_t bg = ILI9341_BLACK) { _tft.setTextColor(fg, bg); }
    void setCursor(int16_t x, int16_t y) { _tft.setCursor(x, y); }

    void print(const String &s) { _tft.print(_toUpper(s)); }
    void print(const char *s) { _tft.print(_toUpper(String(s))); }
    void print(int n) { _tft.print(n); }
    void print(float f, int digits = 2) { _tft.print(f, digits); }
    void println(const String &s) { _tft.println(_toUpper(s)); }
    void println(const char *s) { _tft.println(_toUpper(String(s))); }
    void println(int n) { _tft.println(n); }
    void println(float f, int digits = 2) { _tft.println(f, digits); }
    void println() { _tft.println(); }

    int16_t width() const { return _tft.width(); }
    int16_t height() const { return _tft.height(); }

    void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
    {
        _tft.getTextBounds(str, x, y, x1, y1, w, h);
    }

    // Splash screen
    void drawSplash(
        const School &school,
        const char *college,
        const char *course,
        const Project &project,
        const char *developers[],
        uint8_t developerCount,
        bool printVersion = false)
    {
        clear(ILI9341_BLACK);
        int16_t x1, y1;
        uint16_t w, h;
        int currentY = 20;
        int lineHeight = 15;

        // University (Campus)
        setTextSize(1);
        setTextColor(ILI9341_BLUE);
        String universityText = String(school.name);
        if (strlen(school.campus) > 0)
        {
            universityText += " (" + String(school.campus) + ")";
        }
        _tft.getTextBounds(universityText, 0, 0, &x1, &y1, &w, &h);
        setCursor((width() - w) / 2, currentY);
        print(universityText);
        currentY += h + 2;

        // College
        _tft.getTextBounds(college, 0, 0, &x1, &y1, &w, &h);
        setCursor((width() - w) / 2, currentY);
        print(college);
        currentY += h + 2;

        // Course
        _tft.getTextBounds(course, 0, 0, &x1, &y1, &w, &h);
        setCursor((width() - w) / 2, currentY);
        print(course);
        currentY += h + 15;

        // Project Nickname
        setTextSize(4);
        setTextColor(ILI9341_WHITE);
        _tft.getTextBounds(project.nickname, 0, 0, &x1, &y1, &w, &h);
        setCursor((width() - w) / 2, currentY);
        print(project.nickname);
        currentY += h + 10;

        // Project Fullname
        setTextSize(2);
        setTextColor(ILI9341_YELLOW);
        _tft.getTextBounds(project.fullname, 0, 0, &x1, &y1, &w, &h);
        setCursor((width() - w) / 2, currentY);
        print(project.fullname);
        currentY += h + 10;

        // Version

        if (printVersion)
        {
            setTextSize(1);
            setTextColor(ILI9341_WHITE);
            String versionText = "VERSION " + String(project.version);
            _tft.getTextBounds(versionText, 0, 0, &x1, &y1, &w, &h);
            setCursor((width() - w) / 2, currentY);
            print(versionText);
        }

        // DEVELOPERS title (positioned at bottom area)
        int devTitleY = height() - 90;
        setTextSize(1);
        setTextColor(ILI9341_WHITE);
        String devTitle = "DEVELOPERS";
        _tft.getTextBounds(devTitle, 0, 0, &x1, &y1, &w, &h);
        setCursor((width() - w) / 2, devTitleY);
        print(devTitle);

        // Developer names - equally spaced
        int devStartY = devTitleY + 20;
        int availableHeight = height() - devStartY - 10;
        int spacing = availableHeight / developerCount;

        if (spacing > 25)
            spacing = 25; // Cap maximum spacing
        if (spacing < 15)
            spacing = 15; // Minimum spacing

        if (developerCount <= 3)
        {
            setTextSize(2);
        }
        else
        {
            setTextSize(1);
        }
        for (uint8_t i = 0; i < developerCount; i++)
        {
            int devY = devStartY + (i * spacing);
            _tft.getTextBounds(developers[i], 0, 0, &x1, &y1, &w, &h);
            setCursor((width() - w) / 2, devY);
            print(developers[i]);
        }
    }

private:
    Adafruit_ILI9341 _tft;
    XPT2046_Touchscreen _ts;
    bool _calibrated;
    int16_t _xMin, _xMax, _yMin, _yMax;
    uint32_t _lastTouch;
    bool _touchFeedback;

    void _drawCross(int16_t x, int16_t y)
    {
        _tft.fillCircle(x, y, 8, ILI9341_RED);
        _tft.drawLine(x - 15, y, x + 15, y, ILI9341_RED);
        _tft.drawLine(x, y - 15, x, y + 15, ILI9341_RED);
        delay(500);
    }

    TS_Point _waitTouch()
    {
        while (!_ts.touched())
            delay(10);
        TS_Point p = _ts.getPoint();
        while (_ts.touched())
            delay(10);
        return p;
    }

    void _mapTouch(int16_t rawX, int16_t rawY, int16_t &x, int16_t &y)
    {
        x = map(rawX, _xMin, _xMax, 0, _tft.width());
        y = map(rawY, _yMin, _yMax, 0, _tft.height());
        if (x < 0)
            x = 0;
        if (x > _tft.width())
            x = _tft.width();
        if (y < 0)
            y = 0;
        if (y > _tft.height())
            y = _tft.height();
    }

    void _saveCalibration()
    {
        EEPROM.writeUShort(CAL_ADDR_MAGIC, CAL_MAGIC);
        EEPROM.writeInt(CAL_ADDR_XMIN, _xMin);
        EEPROM.writeInt(CAL_ADDR_XMAX, _xMax);
        EEPROM.writeInt(CAL_ADDR_YMIN, _yMin);
        EEPROM.writeInt(CAL_ADDR_YMAX, _yMax);
        EEPROM.commit();
    }

    bool _loadCalibration()
    {
        if (EEPROM.readUShort(CAL_ADDR_MAGIC) != CAL_MAGIC)
            return false;
        _xMin = EEPROM.readInt(CAL_ADDR_XMIN);
        _xMax = EEPROM.readInt(CAL_ADDR_XMAX);
        _yMin = EEPROM.readInt(CAL_ADDR_YMIN);
        _yMax = EEPROM.readInt(CAL_ADDR_YMAX);
        if (_xMin == 0 && _xMax == 0 && _yMin == 0 && _yMax == 0)
            return false;
        _calibrated = true;
        return true;
    }

    String _toUpper(const String &s)
    {
        String res = s;
        res.toUpperCase();
        return res;
    }
};

#endif