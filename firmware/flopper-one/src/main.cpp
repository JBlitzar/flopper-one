#include <Arduino.h>

#include "menu_tree.h"
#include "menu_manager.h"
#include "input_dispatcher.h"

void setup()
{
    flopper::build_menu_tree();
    flopper::MenuManager::get_instance().set_root(&flopper::root);
    flopper::InputDispatcher::get_instance().push(&flopper::MenuManager::get_instance());
}

void loop()
{
    flopper::InputDispatcher::get_instance().poll();
    flopper::MenuManager::get_instance().tick();
}