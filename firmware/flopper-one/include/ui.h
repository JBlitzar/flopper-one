#pragma once
#include "display.h"
#include "input_event.h"
#include <vector>

namespace flopper::ui
{

    constexpr int16_t MARGIN_X = 10;
    constexpr int16_t MARGIN_Y = 10;
    constexpr int16_t HEADER_MARGIN = 30;

    constexpr size_t LIST_VISIBLE_ROWS = 4;
    constexpr int LIST_TEXT_SIZE = 2;

    constexpr int16_t LINE_H = (240 - HEADER_MARGIN) / (int16_t)LIST_VISIBLE_ROWS;

    inline size_t clamp_index(size_t idx, size_t count)
    {
        if (count == 0)
            return 0;
        return idx < count ? idx : (count - 1);
    }

    inline bool apply_list_nav(InputEvent e, size_t &selected_index, size_t count)
    {
        if (e == InputEvent::UP)
        {
            if (selected_index > 0)
            {
                selected_index--;
                return true;
            }
            return false;
        }
        if (e == InputEvent::DOWN)
        {
            if (count && selected_index + 1 < count)
            {
                selected_index++;
                return true;
            }
            return false;
        }
        return false;
    }

    inline void draw_list_at(Display &display,
                             const std::vector<const char *> &items,
                             size_t selected_index,
                             int16_t y_start,
                             size_t window_start = 0)
    {
        const int16_t h = 240 - y_start;
        int16_t y = y_start;
        display.fill_rect(MARGIN_X, y, 240, h, TFT_BLACK);

        const size_t max_visible = h > 0 ? (size_t)(h / LINE_H) : 0;
        const size_t end = (window_start + max_visible < items.size()) ? (window_start + max_visible) : items.size();

        for (size_t i = window_start; i < end; i++)
        {
            const bool selected = i == selected_index;
            const uint32_t fg = selected ? TFT_BLACK : TFT_WHITE;
            const uint32_t bg = selected ? TFT_CYAN : TFT_BLACK;
            display.fill_rect(MARGIN_X - 4, y - 2, 240, LINE_H, bg);
            display.draw_text(MARGIN_X, y, items[i], fg, LIST_TEXT_SIZE, bg);
            y += LINE_H;
        }
    }

    inline void draw_list(Display &display,
                          const std::vector<const char *> &items,
                          size_t selected_index,
                          size_t window_start = 0)
    {
        draw_list_at(display, items, selected_index, HEADER_MARGIN, window_start);
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