#pragma once

#include <Arduino.h>
#include "input_event.h"
#include "input_handler.h"
#include "flopper_pins.h"
#include <stack>

namespace flopper
{

    struct Button
    {
        uint8_t pin;
        InputEvent event;
        bool stable_pressed = false;
        bool last_raw_pressed = false;
        uint32_t last_change_ms = 0;

        Button(uint8_t p, InputEvent e) : pin(p), event(e) {}
    };

    inline Button buttons_[5] = {
        {pins::DPAD_UP, InputEvent::UP},
        {pins::DPAD_DOWN, InputEvent::DOWN},
        {pins::DPAD_CENTER, InputEvent::CENTER},
        {pins::DPAD_LEFT, InputEvent::LEFT},
        {pins::DPAD_RIGHT, InputEvent::RIGHT}};

    class InputDispatcher
    {
    public:
        static InputDispatcher &get_instance()
        {
            static InputDispatcher instance;
            return instance;
        }

        void push(InputHandler *handler)
        {
            handlers.push(handler);
        }

        void pop()
        {
            if (!handlers.empty())
            {
                handlers.pop();
            }
        }

        void poll()
        {
            uint32_t now = millis();
            if (handlers.empty())
                return;

            for (auto &btn : buttons_)
            {
                const bool raw_pressed = (digitalRead(btn.pin) == LOW);

                if (raw_pressed != btn.last_raw_pressed)
                {
                    btn.last_raw_pressed = raw_pressed;
                    btn.last_change_ms = now;
                }

                // Only accept a state transition once the raw level has been stable
                // for a minimum amount of time.
                if ((uint32_t)(now - btn.last_change_ms) < kDebounceMs)
                    continue;

                if (raw_pressed == btn.stable_pressed)
                    continue;

                btn.stable_pressed = raw_pressed;
                if (btn.stable_pressed)
                    dispatch_(btn.event); // fire only on press
            }
        }

    private:
        static constexpr uint32_t kDebounceMs = 30;

        InputDispatcher()
        {
            // init...
            for (auto &btn : buttons_)
            {
                pinMode(btn.pin, INPUT_PULLUP);
                const bool raw_pressed = (digitalRead(btn.pin) == LOW);
                btn.last_raw_pressed = raw_pressed;
                btn.stable_pressed = raw_pressed;
                btn.last_change_ms = millis();
            }
        };
        std::stack<InputHandler *> handlers;

        void dispatch_(InputEvent event)
        {
            if (!handlers.empty())
            {
                // magic fix instead of using . notation idk
                handlers.top()->on_input(event);
            }
        }

        // copied + edited from google AI overview lol

        InputDispatcher(const InputDispatcher &) = delete;
        InputDispatcher &operator=(const InputDispatcher &) = delete;

        InputDispatcher(InputDispatcher &&) = delete;
        InputDispatcher &operator=(InputDispatcher &&) = delete;
    };
}