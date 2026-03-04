#pragma once
#include "display.h"
#include <vector>

namespace flopper::ui
{

    constexpr int16_t MARGIN_X = 10;
    constexpr int16_t MARGIN_Y = 10;
    constexpr int16_t LINE_H = 18;

    constexpr int16_t HEADER_MARGIN = 30;

    inline void draw_list(Display &display,
                          const std::vector<const char *> &items,
                          size_t selected_index,
                          size_t window_start = 0)
    {
        int16_t y = HEADER_MARGIN;
        display.fill_rect(MARGIN_X, y, 240, 240, TFT_BLACK);
        for (size_t i = 0; i < items.size(); i++)
        {
            const bool selected = i == selected_index;
            const uint32_t fg = selected ? TFT_BLACK : TFT_WHITE;
            const uint32_t bg = selected ? TFT_CYAN : TFT_BLACK;
            display.fill_rect(MARGIN_X - 4, y - 2, 240, LINE_H, bg);
            display.draw_text(MARGIN_X, y, items[i], fg, 1, bg);
            y += LINE_H;
        }
    }
    inline void draw_confirm(Display &display, const char *message)
    {
        display.clear(TFT_BLACK);
        display.draw_text(MARGIN_X, MARGIN_Y, message, TFT_WHITE, 1);
        display.draw_text(MARGIN_X, MARGIN_Y + 20, "CENTER = yes", TFT_GREEN, 1);
        display.draw_text(MARGIN_X, MARGIN_Y + 38, "LEFT   = no", TFT_RED, 1);
    }
    inline void draw_status(Display &display, const char *text)
    {
        display.fill_rect(0, 0, 240, HEADER_MARGIN, TFT_BLACK);
        display.draw_text(MARGIN_X, MARGIN_Y, text, TFT_WHITE, 1, TFT_BLACK);
    }

}