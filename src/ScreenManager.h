#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "TFT_Handler.h"
#include "plants.h"

class ScreenManager
{
public:
    enum ScreenName
    {
        SCREEN_SPLASH,
        SCREEN_WELCOME,
        SCREEN_SYSTEM_READY,
        SCREEN_STATUS,
        SCREEN_DRYER_MENU,
        SCREEN_GRINDER_MENU,
        SCREEN_MIXER_MENU,
        SCREEN_SOIL_ANALYSIS,
        SCREEN_CHOOSE_PLANT,
        SCREEN_DISPENSING,
        SCREEN_COMPLETE
    };

    ScreenManager(TFT_Handler &tft) : _tft(tft), _currentScreen(SCREEN_SPLASH) {}

    void begin()
    {
        switchScreen(SCREEN_SPLASH);
    }

    void switchScreen(ScreenName newScreen)
    {
        _currentScreen = newScreen;

        switch (_currentScreen)
        {
        case SCREEN_SPLASH:
            drawSplash();
            break;

        case SCREEN_WELCOME:
            drawWelcomeScreen();
            break;

        case SCREEN_SYSTEM_READY:
            drawSystemReady();
            break;

        case SCREEN_STATUS:
            drawStatScreen();
            break;

        case SCREEN_DRYER_MENU:
            updateDryerMenu();
            break;

        case SCREEN_GRINDER_MENU:
            updateGrinderMenu();
            break;

        case SCREEN_MIXER_MENU:
            updateMixerMenu();
            break;

        case SCREEN_SOIL_ANALYSIS:
            drawSoilAnalysisResults();
            break;

        case SCREEN_CHOOSE_PLANT:
            drawChoosePlantMenu();
            break;

        case SCREEN_DISPENSING:
            drawNutrientsDispensing();
            break;

        case SCREEN_COMPLETE:
            drawMixingComplete();
            break;
        }
    }

    ScreenName getCurrentScreen() const { return _currentScreen; }

    // Update methods for dynamic data
    void updateStatusQuadrant(TFT_Handler::Quadrant q, bool value)
    {
        updateQuadrant(q, value);
    }

    void updateSoilData(float weight, float n, float p, float k)
    {
        _soilWeight = weight;
        _soilN = n;
        _soilP = p;
        _soilK = k;
        updateQuadrant(TFT_Handler::QUAD_SOIL, weight, n, p, k);
    }

    void updateDryerMenu(bool isOn)
    {
        _isDryerOn = isOn;
        if (_currentScreen == SCREEN_DRYER_MENU)
        {
            updateDryerMenu();
        }
    }

    void updateGrinderMenu(bool isOn)
    {
        _isGrinderOn = isOn;
        if (_currentScreen == SCREEN_GRINDER_MENU)
        {
            updateGrinderMenu();
        }
    }

    void updateMixerMenu(bool isOn)
    {
        _isMixerOn = isOn;
        if (_currentScreen == SCREEN_MIXER_MENU)
        {
            updateMixerMenu();
        }
    }

    void updateSoilAnalysis(float n, float p, float k, float pH, float moisture, float temp)
    {
        _soilN = n;
        _soilP = p;
        _soilK = k;
        _soilPH = pH;
        _soilMoisture = moisture;
        _soilTemp = temp;
        if (_currentScreen == SCREEN_SOIL_ANALYSIS)
        {
            drawSoilAnalysisResults();
        }
    }

    void updateDispensing(bool sufficient, const char *missing)
    {
        _nutrientsSufficient = sufficient;
        _missingNutrient = missing ? String(missing) : "";
        if (_currentScreen == SCREEN_DISPENSING)
        {
            drawNutrientsDispensing();
        }
    }

    // Button handlers
    bool isStartPressed()
    {
        if (_currentScreen != SCREEN_SYSTEM_READY)
            return false;
        const int w = 200, h = 60;
        const int x = (_tft.width() - w) / 2;
        const int y = _tft.height() - 100;
        return _tft.isButtonPressed(x, y, w, h);
    }

    bool isBackPressed()
    {
        if (_currentScreen != SCREEN_DRYER_MENU &&
            _currentScreen != SCREEN_GRINDER_MENU &&
            _currentScreen != SCREEN_MIXER_MENU)
            return false;
        const int w = 200, h = 50;
        const int centerX = _tft.width() / 2;
        const int x = centerX - w / 2;
        const int y = 150;
        return _tft.isButtonPressed(x, y, w, h);
    }

    bool isTurnOnPressed()
    {
        if (_currentScreen != SCREEN_DRYER_MENU &&
            _currentScreen != SCREEN_GRINDER_MENU &&
            _currentScreen != SCREEN_MIXER_MENU)
            return false;
        if (_isDryerOn && _currentScreen == SCREEN_DRYER_MENU)
            return false;
        if (_isGrinderOn && _currentScreen == SCREEN_GRINDER_MENU)
            return false;
        if (_isMixerOn && _currentScreen == SCREEN_MIXER_MENU)
            return false;

        const int w = 200, h = 50;
        const int centerX = _tft.width() / 2;
        const int x = centerX - w / 2;
        const int y = 80;
        return _tft.isButtonPressed(x, y, w, h);
    }

    bool isTurnOffPressed()
    {
        if (_currentScreen != SCREEN_DRYER_MENU &&
            _currentScreen != SCREEN_GRINDER_MENU &&
            _currentScreen != SCREEN_MIXER_MENU)
            return false;
        if (!_isDryerOn && _currentScreen == SCREEN_DRYER_MENU)
            return false;
        if (!_isGrinderOn && _currentScreen == SCREEN_GRINDER_MENU)
            return false;
        if (!_isMixerOn && _currentScreen == SCREEN_MIXER_MENU)
            return false;

        const int w = 200, h = 50;
        const int centerX = _tft.width() / 2;
        const int x = centerX - w / 2;
        const int y = 80;
        return _tft.isButtonPressed(x, y, w, h);
    }

    bool isOkPressed()
    {
        if (_currentScreen != SCREEN_SOIL_ANALYSIS)
            return false;
        return _tft.isButtonPressed(40, _tft.height() - 60, 80, 40);
    }

    bool isChoosePlantPressed()
    {
        if (_currentScreen != SCREEN_SOIL_ANALYSIS)
            return false;
        return _tft.isButtonPressed(_tft.width() - 180, _tft.height() - 60, 160, 40);
    }

    int getSelectedPlant()
    {
        if (_currentScreen != SCREEN_CHOOSE_PLANT)
            return -1;
        const int btnW = _tft.width() - 40;
        const int btnH = 40;
        const int startX = 20;
        int startY = 60;
        for (uint8_t i = 0; i < _plantCount; i++)
        {
            int btnY = startY + i * (btnH + 10);
            if (_tft.isButtonPressed(startX, btnY, btnW, btnH))
            {
                return i;
            }
        }
        return -1;
    }

    bool isErasePressed()
    {
        return _tft.isButtonPressed(ERASE_BTN_X, ERASE_BTN_Y, ERASE_BTN_W, ERASE_BTN_H);
    }

private:
    TFT_Handler &_tft;
    ScreenName _currentScreen;

    // Dynamic data
    bool _isDryerOn = false;
    bool _isGrinderOn = false;
    bool _isMixerOn = false;
    float _soilWeight = 0.0;
    float _soilN = 0.0, _soilP = 0.0, _soilK = 0.0;
    float _soilPH = 0.0, _soilMoisture = 0.0, _soilTemp = 0.0;
    bool _nutrientsSufficient = false;
    String _missingNutrient = "";

    // Plant data
    const char *_plantNames[plantCount];
    uint8_t _plantCount = plantCount;

    // Drawing methods
    void drawSplash()
    {
        _tft.clear(ILI9341_BLACK);
        int16_t x1, y1;
        uint16_t w, h;

        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_BLUE);
        String line0 = "PANGASINAN STATE UNIVERSITY - URDANETA CITY CAMPUS";
        _tft.getTextBounds(line0, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, 20);
        _tft.print(line0);

        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_BLUE);
        String line1 = "COLLEGE OF ENGINEERING AND ARCHITECTURE";
        _tft.getTextBounds(line1, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, 35);
        _tft.print(line1);

        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_BLUE);
        String line2 = "BS COMPUTER ENGINEERING";
        _tft.getTextBounds(line2, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, 50);
        _tft.print(line2);

        _tft.setTextSize(5);
        _tft.setTextColor(ILI9341_WHITE);
        String line3 = "SOILMATE";
        _tft.getTextBounds(line3, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, 80);
        _tft.print(line3);

        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_YELLOW);
        String line4 = "SMART FERTILIZER SYSTEM";
        _tft.getTextBounds(line4, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, 120);
        _tft.print(line4);

        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_CYAN);
        String line5 = "DEVELOPED BY:";
        _tft.getTextBounds(line5, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, 150);
        _tft.print(line5);

        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_WHITE);
        String line6 = "MICHELLE ORLANDA";
        _tft.getTextBounds(line6, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, 170);
        _tft.print(line6);

        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_WHITE);
        String line7 = "MICCAH POQUIZ";
        _tft.getTextBounds(line7, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, 180);
        _tft.print(line7);

        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_WHITE);
        String line8 = "AILA DATANAGAN";
        _tft.getTextBounds(line8, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, 190);
        _tft.print(line8);

        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_WHITE);
        String line9 = "VERSION 1.0.0";
        _tft.getTextBounds(line9, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, _tft.height() - 25);
        _tft.print(line9);
    }

    void drawWelcomeScreen()
    {
        _tft.clear(ILI9341_BLACK);
        int16_t x1, y1;
        uint16_t w, h;

        _tft.setTextSize(3);
        _tft.setTextColor(ILI9341_WHITE);
        String title = "SOIL MATE";
        _tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, _tft.height() / 2 - 40);
        _tft.print(title);

        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_YELLOW);
        String subtitle = "SMART FERTILIZER SYSTEM";
        _tft.getTextBounds(subtitle, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, _tft.height() / 2);
        _tft.print(subtitle);

        _tft.setTextSize(1);
        _tft.setTextColor(ILI9341_CYAN);
        String tagline = "YOUR SOIL HEALTH PARTNER";
        _tft.getTextBounds(tagline, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, _tft.height() / 2 + 40);
        _tft.print(tagline);
    }

    void drawSystemReady()
    {
        _tft.clear();
        _tft.setTextSize(3);
        _tft.setTextColor(ILI9341_GREEN);
        _tft.setCursor(40, _tft.height() / 2 - 20);
        _tft.print("SYSTEM READY");
        _tft.drawEraseButton();
        drawStartButton();
    }

    void drawStartButton()
    {
        const int w = 200, h = 60;
        const int x = (_tft.width() - w) / 2;
        const int y = _tft.height() - 100;
        drawButton("START DRYING", x, y, w, h, ILI9341_GREEN, ILI9341_WHITE);
    }

    void drawButton(const char *label, int16_t x, int16_t y, int16_t w, int16_t h,
                    uint16_t outlineColor = ILI9341_GREEN, uint16_t textColor = ILI9341_WHITE)
    {
        _tft.fillRect(x, y, w, h, ILI9341_BLACK);
        _tft.drawRect(x, y, w, h, outlineColor);
        _tft.setTextSize(2);
        _tft.setTextColor(textColor);
        String up = String(label);
        up.toUpperCase();
        int16_t tx, ty;
        uint16_t tw, th;
        _tft.getTextBounds(up, 0, 0, &tx, &ty, &tw, &th);
        _tft.setCursor(x + (w - tw) / 2, y + (h - th) / 2);
        _tft.print(up);
    }

    void drawStatScreen()
    {
        _tft.clear(ILI9341_BLACK);
        int halfW = _tft.width() / 2;
        int halfH = _tft.height() / 2;

        _tft.drawLine(halfW, 0, halfW, _tft.height(), ILI9341_BLUE);
        _tft.drawLine(0, halfH, _tft.width(), halfH, ILI9341_BLUE);

        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_YELLOW);

        int16_t x1, y1;
        uint16_t w, h;

        String title1 = "DRYER";
        _tft.getTextBounds(title1, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((halfW - w) / 2, 10);
        _tft.print(title1);

        String title2 = "GRINDER";
        _tft.getTextBounds(title2, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor(halfW + (halfW - w) / 2, 10);
        _tft.print(title2);

        String title3 = "MIXER";
        _tft.getTextBounds(title3, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((halfW - w) / 2, halfH + 10);
        _tft.print(title3);

        String title4 = "PRODUCT";
        _tft.getTextBounds(title4, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor(halfW + (halfW - w) / 2, halfH + 10);
        _tft.print(title4);

        updateQuadrant(TFT_Handler::QUAD_DRYER, false);
        updateQuadrant(TFT_Handler::QUAD_GRINDER, false);
        updateQuadrant(TFT_Handler::QUAD_MIXER, false);
        updateQuadrant(TFT_Handler::QUAD_SOIL, 0.0f, 0.0f, 0.0f, 0.0f);
    }

    void updateQuadrant(TFT_Handler::Quadrant q, bool value)
    {
        int halfW = _tft.width() / 2;
        int halfH = _tft.height() / 2;

        int contentX, contentY, contentW, contentH;
        switch (q)
        {
        case TFT_Handler::QUAD_DRYER:
            contentX = 0;
            contentY = 35;
            contentW = halfW - 1;
            contentH = halfH - 35 - 1;
            break;
        case TFT_Handler::QUAD_GRINDER:
            contentX = halfW + 1;
            contentY = 35;
            contentW = halfW - 1;
            contentH = halfH - 35 - 1;
            break;
        case TFT_Handler::QUAD_MIXER:
            contentX = 0;
            contentY = halfH + 35;
            contentW = halfW - 1;
            contentH = halfH - 35 - 1;
            break;
        default:
            return;
        }

        _tft.fillRect(contentX, contentY, contentW, contentH, ILI9341_BLACK);
        String line = "STATUS: " + String(value ? "ON" : "OFF");
        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_WHITE);
        int16_t x1, y1;
        uint16_t w, h;
        _tft.getTextBounds(line, 0, 0, &x1, &y1, &w, &h);
        int centerX = contentX + (contentW - w) / 2;
        int centerY = contentY + (contentH - h) / 2;
        _tft.setCursor(centerX, centerY);
        _tft.print(line);
    }

    void updateQuadrant(TFT_Handler::Quadrant q, float weight, float n, float p, float k)
    {
        if (q != TFT_Handler::QUAD_SOIL)
            return;

        int halfW = _tft.width() / 2;
        int halfH = _tft.height() / 2;
        int contentX = halfW + 1;
        int contentY = halfH + 35;
        int contentW = halfW - 1;
        int contentH = halfH - 35 - 1;

        _tft.fillRect(contentX, contentY, contentW, contentH, ILI9341_BLACK);

        String lineW = "W:" + String(weight, 1) + "g";
        String lineN = "N:" + String(n, 2) + "ppm";
        String lineP = "P:" + String(p, 2) + "ppm";
        String lineK = "K:" + String(k, 1) + "ppm";

        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_WHITE);

        int16_t x1, y1;
        uint16_t wW, hW, wN, hN, wP, hP, wK, hK;
        _tft.getTextBounds(lineW, 0, 0, &x1, &y1, &wW, &hW);
        _tft.getTextBounds(lineN, 0, 0, &x1, &y1, &wN, &hN);
        _tft.getTextBounds(lineP, 0, 0, &x1, &y1, &wP, &hP);
        _tft.getTextBounds(lineK, 0, 0, &x1, &y1, &wK, &hK);

        int gap = hW / 2;
        int totalH = hW + hN + hP + hK + 3 * gap;
        int startY = contentY + (contentH - totalH) / 2;

        int leftMargin = 10;
        _tft.setCursor(contentX + leftMargin, startY);
        _tft.print(lineW);
        _tft.setCursor(contentX + leftMargin, startY + hW + gap);
        _tft.print(lineN);
        _tft.setCursor(contentX + leftMargin, startY + hW + gap + hN + gap);
        _tft.print(lineP);
        _tft.setCursor(contentX + leftMargin, startY + hW + gap + hN + gap + hP + gap);
        _tft.print(lineK);
    }

    void updateDryerMenu()
    {
        _tft.clear();
        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_WHITE);
        String title = "DRYING CHAMBER";
        int16_t x1, y1;
        uint16_t w, h;
        _tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, 20);
        _tft.print(title);

        int btnW = 200, btnH = 50;
        int centerX = _tft.width() / 2;

        if (!_isDryerOn)
        {
            int btnOnX = centerX - btnW / 2;
            drawButton("TURN ON", btnOnX, 80, btnW, btnH, ILI9341_GREEN, ILI9341_WHITE);
        }
        else
        {
            int btnOffX = centerX - btnW / 2;
            drawButton("TURN OFF", btnOffX, 80, btnW, btnH, ILI9341_RED, ILI9341_WHITE);
        }

        int btnBackX = centerX - btnW / 2;
        drawButton("BACK", btnBackX, 150, btnW, btnH, ILI9341_BLUE, ILI9341_WHITE);

        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_WHITE);
        String statusText = _isDryerOn ? "STATUS: RUNNING" : "STATUS: IDLE";
        _tft.getTextBounds(statusText, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, 220);
        _tft.print(statusText);
    }

    void updateGrinderMenu()
    {
        _tft.clear();
        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_WHITE);
        String title = "GRINDING CHAMBER";
        int16_t x1, y1;
        uint16_t w, h;
        _tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, 20);
        _tft.print(title);

        int btnW = 200, btnH = 50;
        int centerX = _tft.width() / 2;

        if (!_isGrinderOn)
        {
            int btnOnX = centerX - btnW / 2;
            drawButton("TURN ON", btnOnX, 80, btnW, btnH, ILI9341_GREEN, ILI9341_WHITE);
        }
        else
        {
            int btnOffX = centerX - btnW / 2;
            drawButton("TURN OFF", btnOffX, 80, btnW, btnH, ILI9341_RED, ILI9341_WHITE);
        }

        int btnBackX = centerX - btnW / 2;
        drawButton("BACK", btnBackX, 150, btnW, btnH, ILI9341_BLUE, ILI9341_WHITE);

        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_WHITE);
        String statusText = _isGrinderOn ? "STATUS: RUNNING" : "STATUS: IDLE";
        _tft.getTextBounds(statusText, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, 220);
        _tft.print(statusText);
    }

    void updateMixerMenu()
    {
        _tft.clear();
        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_WHITE);
        String title = "MIXING CHAMBER";
        int16_t x1, y1;
        uint16_t w, h;
        _tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, 20);
        _tft.print(title);

        int btnW = 200, btnH = 50;
        int centerX = _tft.width() / 2;

        if (!_isMixerOn)
        {
            int btnOnX = centerX - btnW / 2;
            drawButton("TURN ON", btnOnX, 80, btnW, btnH, ILI9341_GREEN, ILI9341_WHITE);
        }
        else
        {
            int btnOffX = centerX - btnW / 2;
            drawButton("TURN OFF", btnOffX, 80, btnW, btnH, ILI9341_RED, ILI9341_WHITE);
        }

        int btnBackX = centerX - btnW / 2;
        drawButton("BACK", btnBackX, 150, btnW, btnH, ILI9341_BLUE, ILI9341_WHITE);

        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_WHITE);
        String statusText = _isMixerOn ? "STATUS: RUNNING" : "STATUS: IDLE";
        _tft.getTextBounds(statusText, 0, 0, &x1, &y1, &w, &h);
        _tft.setCursor((_tft.width() - w) / 2, 220);
        _tft.print(statusText);
    }

    void drawSoilAnalysisResults()
    {
        _tft.clear();
        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_WHITE);
        _tft.setCursor(10, 10);
        _tft.println("SOIL ANALYSIS:");
        _tft.setCursor(20, 50);
        _tft.print("N: ");
        _tft.print(_soilN, 1);
        _tft.println(" KG/HA");
        _tft.print("P: ");
        _tft.print(_soilP, 1);
        _tft.println(" KG/HA");
        _tft.print("K: ");
        _tft.print(_soilK, 1);
        _tft.println(" KG/HA");
        _tft.print("PH: ");
        _tft.print(_soilPH, 1);
        _tft.print("  MOISTURE: ");
        _tft.print(_soilMoisture, 0);
        _tft.println("%");
        _tft.print("TEMP: ");
        _tft.print(_soilTemp, 1);
        _tft.println(" C");
        drawButton("OK", 40, _tft.height() - 60, 80, 40, ILI9341_GREEN, ILI9341_WHITE);
        drawButton("CHOOSE PLANT", _tft.width() - 180, _tft.height() - 60, 160, 40, ILI9341_BLUE, ILI9341_WHITE);
        _tft.drawEraseButton();
    }

    void drawChoosePlantMenu()
    {
        _tft.clear();
        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_WHITE);
        _tft.setCursor(10, 10);
        _tft.println("SELECT PLANT:");
        const int btnW = _tft.width() - 40;
        const int btnH = 40;
        int startY = 60;
        for (uint8_t i = 0; i < _plantCount && i < 10; i++)
        {
            drawButton(plantList[i].name, 20, startY + i * (btnH + 10), btnW, btnH, ILI9341_DARKCYAN, ILI9341_WHITE);
        }
        _tft.drawEraseButton();
    }

    void drawNutrientsDispensing()
    {
        _tft.clear();
        _tft.setTextSize(2);
        if (_nutrientsSufficient)
        {
            _tft.setTextColor(ILI9341_GREEN);
            _tft.setCursor(20, _tft.height() / 2 - 20);
            _tft.println("NUTRIENTS SUFFICIENT!");
        }
        else
        {
            _tft.setTextColor(ILI9341_RED);
            _tft.setCursor(20, _tft.height() / 2 - 40);
            _tft.print("ADDING: ");
            _tft.println(_missingNutrient.length() > 0 ? _missingNutrient : "ADDITIONAL NUTRIENTS");
        }
        _tft.drawEraseButton();
    }

    void drawMixingComplete()
    {
        _tft.clear();
        _tft.setTextSize(2);
        _tft.setTextColor(ILI9341_YELLOW);
        _tft.setCursor(20, _tft.height() / 2 - 20);
        _tft.println("MIXING COMPLETE");
        _tft.setCursor(20, _tft.height() / 2 + 20);
        _tft.println("PLEASE CURE FOR 1-3 DAYS");
        _tft.drawEraseButton();
    }
};

#endif