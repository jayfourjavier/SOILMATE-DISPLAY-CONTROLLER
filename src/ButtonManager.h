#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include "TFT_Handler.h"

class ButtonManager
{
public:
    ButtonManager(TFT_Handler &tft) : _tft(tft) {}

    void drawButton(const char *label, int16_t x, int16_t y, int16_t w, int16_t h,
                    uint16_t outlineColor = ILI9341_GREEN, uint16_t textColor = ILI9341_WHITE)
    {
        _tft.fillRect(x, y, w, h, ILI9341_BLACK);
        _tft.drawRect(x, y, w, h, outlineColor);
        _tft.setTextSize(2);
        _tft.setTextColor(textColor);

        String up = String(label);
        up.toUpperCase();

        int16_t x1, y1;
        uint16_t tw, th;
        _tft.getTextBounds(up, 0, 0, &x1, &y1, &tw, &th);
        _tft.setCursor(x + (w - tw) / 2, y + (h - th) / 2);
        _tft.print(up);
    }

    bool isPressed(int16_t x, int16_t y, int16_t w, int16_t h)
    {
        return _tft.isButtonPressed(x, y, w, h);
    }

private:
    TFT_Handler &_tft;
};

#endif