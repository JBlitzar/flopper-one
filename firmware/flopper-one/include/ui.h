#pragma once
#if defined(FLOPPER_DESKTOP)
#include "display_desktop.h"
#else
#include "display.h"
#endif
#include "input_event.h"
#include <string>
#include <vector>

namespace flopper::ui
{

    constexpr int16_t MARGIN_X = 10;
    constexpr int16_t MARGIN_Y = 10;
    constexpr int16_t HEADER_MARGIN = 30;

    constexpr int16_t LIST_TOP_GAP = 2;
    constexpr int16_t ROW_PAD_Y = 4;

    constexpr size_t LIST_VISIBLE_ROWS = 8;
    constexpr int LIST_TEXT_SIZE = 2;

    constexpr int STATUS_TEXT_SIZE = 2;
    constexpr int CONFIRM_TEXT_SIZE = 2;

    constexpr int16_t LINE_H = (240 - HEADER_MARGIN - LIST_TOP_GAP) / (int16_t)LIST_VISIBLE_ROWS;

    inline size_t status_max_chars()
    {
        const size_t char_w = 6u * (size_t)STATUS_TEXT_SIZE;
        if (char_w == 0)
            return 0;
        const size_t max_chars = (240u / char_w);
        return max_chars > 0 ? max_chars : 0;
    }

    inline std::string ellipsize_left(const std::string &s, size_t max_chars)
    {
        if (max_chars == 0)
            return "";
        if (s.size() <= max_chars)
            return s;
        if (max_chars <= 3)
            return std::string(max_chars, '.');

        // Keep the end of the string (useful for deeply nested paths).
        return "..." + s.substr(s.size() - (max_chars - 3));
    }

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

    inline void draw_confirm(Display &display, const char *message);

    enum class ConfirmResult
    {
        None,
        Yes,
        No,
    };

    struct ConfirmDialog
    {
        bool active = false;
        const char *message = nullptr;

        void open(const char *msg)
        {
            active = true;
            message = msg;
        }

        void close()
        {
            active = false;
            message = nullptr;
        }

        ConfirmResult on_input(InputEvent e)
        {
            if (!active)
                return ConfirmResult::None;

            if (e == InputEvent::CENTER)
            {
                close();
                return ConfirmResult::Yes;
            }
            if (e == InputEvent::LEFT)
            {
                close();
                return ConfirmResult::No;
            }
            return ConfirmResult::None;
        }

        void draw(Display &display) const
        {
            if (!active)
                return;
            draw_confirm(display, message ? message : "Confirm?");
        }
    };

    inline void draw_list_at(Display &display,
                             const std::vector<const char *> &items,
                             size_t selected_index,
                             int16_t y_start,
                             size_t window_start = 0)
    {
        const int16_t y0 = y_start + LIST_TOP_GAP;
        const int16_t h = 240 - y0;
        int16_t y = y0;
        display.fill_rect(0, y, 240, h, TFT_BLACK);

        const size_t max_visible = h > 0 ? (size_t)(h / LINE_H) : 0;
        if (max_visible == 0)
            return;

        // Ensure the selected item stays within the visible window.
        size_t start = window_start;
        if (selected_index < start)
            start = selected_index;
        else if (selected_index >= start + max_visible)
            start = selected_index - max_visible + 1;

        if (items.size() > max_visible)
        {
            const size_t max_start = items.size() - max_visible;
            if (start > max_start)
                start = max_start;
        }
        else
        {
            start = 0;
        }

        const size_t end = (start + max_visible < items.size()) ? (start + max_visible) : items.size();

        for (size_t i = start; i < end; i++)
        {
            const bool selected = i == selected_index;
            const uint32_t fg = selected ? TFT_BLACK : TFT_WHITE;
            const uint32_t bg = selected ? TFT_CYAN : TFT_BLACK;
            display.fill_rect(0, y, 240, LINE_H, bg);
            display.draw_text(MARGIN_X, y + ROW_PAD_Y, items[i], fg, LIST_TEXT_SIZE, bg);
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
        display.draw_text(MARGIN_X, MARGIN_Y, message, TFT_WHITE, CONFIRM_TEXT_SIZE);
        display.draw_text(MARGIN_X, MARGIN_Y + 28, "CENTER = yes", TFT_GREEN, CONFIRM_TEXT_SIZE);
        display.draw_text(MARGIN_X, MARGIN_Y + 52, "LEFT   = no", TFT_RED, CONFIRM_TEXT_SIZE);
    }
    inline void draw_status(Display &display, const char *text)
    {
        display.fill_rect(0, 0, 240, HEADER_MARGIN, TFT_BLACK);
        display.draw_text(MARGIN_X, MARGIN_Y, text, TFT_WHITE, STATUS_TEXT_SIZE, TFT_BLACK);
        // Divider line between header and list.
        display.fill_rect(0, HEADER_MARGIN - 1, 240, 1, TFT_DARKGREY);
    }

}