#pragma once

#include "menu_node.h"
#include "input_handler.h"
#if defined(FLOPPER_DESKTOP)
#include "input_dispatcher_desktop.h"
#else
#include "input_dispatcher.h"
#endif

namespace flopper
{

    class App : public MenuNode, public InputHandler
    {
    public:
        App(std::string name) : MenuNode(name) { is_app_ = true; }

        virtual void on_enter() = 0;
        virtual void on_exit() = 0;
        virtual void tick() = 0;
        virtual void draw() = 0;
        virtual void on_input(InputEvent event) = 0;

        bool wants_exit_ = false;

        void exit()
        {
            on_exit();
            wants_exit_ = true;
            InputDispatcher::get_instance().pop();
        }
    };

}