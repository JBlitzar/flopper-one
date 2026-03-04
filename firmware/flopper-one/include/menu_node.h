#pragma once

#include "input_event.h"
#include <string>
#include <vector>

namespace flopper
{

    class MenuNode
    {
    public:
        std::string name = "MenuNode";
        std::vector<MenuNode *> children = {};
        bool is_app_ = false;

        MenuNode(std::string name)
        {
            this->name = name;
        }
        void add_child(MenuNode *item)
        {
            this->children.push_back(item);
        }
    };

}