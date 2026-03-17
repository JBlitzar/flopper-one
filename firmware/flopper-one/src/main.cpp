#include <Arduino.h>

#include "menu_tree.h"
#include "menu_manager.h"
#include "input_dispatcher.h"
#include "log.h"

void setup()
{
    Serial.begin(115200);
    delay(100);
    flopper::build_menu_tree();
    flopper::MenuManager::get_instance().set_root(&flopper::root);
    flopper::InputDispatcher::get_instance().push(&flopper::MenuManager::get_instance());

    flopper::log::printf("BOOT", "boot %s %s", __DATE__, __TIME__);
}

void loop()
{
    flopper::InputDispatcher::get_instance().poll();
    flopper::MenuManager::get_instance().tick();
}
