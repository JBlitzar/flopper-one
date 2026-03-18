#define FLOPPER_DESKTOP 1

#include "menu_manager.h"
#include "input_dispatcher_desktop.h"
#include "display_desktop.h"

#include "cocoa_bridge.h"

#include "menu_tree_desktop.h"

#include <chrono>
#include <thread>

int main()
{
    flopper::build_menu_tree();
    flopper::MenuManager::get_instance().set_root(&flopper::root);
    flopper::InputDispatcher::get_instance().push(&flopper::MenuManager::get_instance());

    // Match ui.h's assumptions (240x240, header at 30px, 4 list rows).
    flopper::Display::get_instance().set_geometry(240, 240, flopper::ui::HEADER_MARGIN, (int)flopper::ui::LIST_VISIBLE_ROWS);
    flopper::Display::get_instance().init_window(2.0 / 3.0);

    while (flopper::desktop::pump_events())
    {
        flopper::InputDispatcher::get_instance().poll();
        flopper::MenuManager::get_instance().tick();
        flopper::Display::get_instance().present();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
