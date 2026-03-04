#pragma once

#include "menu_node.h"
#include "input_handler.h"
#include "input_dispatcher.h"

namespace flopper
{

    class App : public MenuNode, public InputHandler
    {
    public:
        bool is_app = true;
        App(std::string name) : MenuNode(name) {}

        virtual void on_enter() = 0;
        virtual void on_exit() = 0;
        virtual void tick() = 0;
        virtual void draw() = 0;
        virtual void on_input(InputEvent event) = 0;

        void exit()
        {
            on_exit();
            InputDispatcher::get_instance().pop();
        }
    };

}