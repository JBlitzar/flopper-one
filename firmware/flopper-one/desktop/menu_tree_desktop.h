#pragma once

#include "menu_node.h"
#include "app.h"
#include "ui.h"

#include <vector>
#include <string>

namespace flopper
{
    class DesktopTextApp : public App
    {
    public:
        DesktopTextApp(const char *name, const char *msg) : App(name), msg_(msg ? msg : "") {}

        void on_enter() override {}
        void on_exit() override {}
        void tick() override { draw(); }

        void draw() override
        {
            flopper::ui::draw_status(Display::get_instance(), name.c_str());
            Display::get_instance().fill_rect(0, flopper::ui::HEADER_MARGIN, 240, 240 - flopper::ui::HEADER_MARGIN, TFT_BLACK);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, msg_.c_str(), TFT_WHITE, 2, TFT_BLACK);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 100, "LEFT=back", TFT_CYAN, 2, TFT_BLACK);
        }

        void on_input(InputEvent e) override
        {
            if (e == InputEvent::LEFT)
                exit();
        }

    private:
        std::string msg_;
    };

    class DesktopListDemoApp : public App
    {
    public:
        DesktopListDemoApp() : App("List Demo") {}

        void on_enter() override
        {
            selected_ = 0;
        }

        void on_exit() override {}

        void tick() override
        {
            draw();
        }

        void draw() override
        {
            flopper::ui::draw_status(Display::get_instance(), "Desktop UI (u/d/l/r/c, arrows, enter)");

            items_.clear();
            for (auto &s : lines_)
                items_.push_back(s.c_str());

            flopper::ui::draw_list(Display::get_instance(), items_, flopper::ui::clamp_index(selected_, items_.size()));
        }

        void on_input(InputEvent e) override
        {
            if (e == InputEvent::LEFT)
            {
                exit();
                return;
            }

            if (flopper::ui::apply_list_nav(e, selected_, lines_.size()))
                return;

            if (e == InputEvent::CENTER)
            {
                // Toggle a line to prove redraws.
                toggled_ = !toggled_;
                lines_[2] = toggled_ ? "Row 3 (toggled ON)" : "Row 3 (toggled OFF)";
                return;
            }
        }

    private:
        size_t selected_ = 0;
        bool toggled_ = false;
        std::vector<std::string> lines_ = {
            "Row 1",
            "Row 2",
            "Row 3 (toggled OFF)",
            "Row 4",
            "Row 5",
            "Row 6",
            "Row 7",
            "Row 8",
            "Row 9 (scroll test)",
            "Row 10",
            "Row 11",
            "Row 12",
        };
        std::vector<const char *> items_;
    };

    class DesktopConfirmDemoApp : public App
    {
    public:
        DesktopConfirmDemoApp() : App("Confirm Demo") {}

        void on_enter() override
        {
            status_ = "CENTER=open confirm";
            confirm_.close();
        }

        void on_exit() override {}

        void tick() override
        {
            draw();
        }

        void draw() override
        {
            if (confirm_.active)
            {
                confirm_.draw(Display::get_instance());
                return;
            }

            flopper::ui::draw_status(Display::get_instance(), "Confirm Demo");
            Display::get_instance().fill_rect(0, flopper::ui::HEADER_MARGIN, 240, 240 - flopper::ui::HEADER_MARGIN, TFT_BLACK);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, status_.c_str(), TFT_WHITE, 2, TFT_BLACK);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 100, "LEFT=back", TFT_CYAN, 2, TFT_BLACK);
        }

        void on_input(InputEvent e) override
        {
            if (confirm_.active)
            {
                const auto r = confirm_.on_input(e);
                if (r == flopper::ui::ConfirmResult::Yes)
                    status_ = "confirmed";
                else if (r == flopper::ui::ConfirmResult::No)
                    status_ = "cancelled";
                return;
            }

            if (e == InputEvent::LEFT)
            {
                exit();
                return;
            }
            if (e == InputEvent::CENTER)
            {
                confirm_.open("Confirm?");
                return;
            }
        }

    private:
        flopper::ui::ConfirmDialog confirm_;
        std::string status_;
    };

    inline DesktopListDemoApp desktop_list_demo;
    inline DesktopConfirmDemoApp desktop_confirm_demo;

    inline DesktopTextApp app1{"App One", "Stub app 1"};
    inline DesktopTextApp app2{"App Two", "Stub app 2"};
    inline DesktopTextApp app3{"App Three", "Stub app 3"};
    inline DesktopTextApp app4{"App Four", "Stub app 4"};
    inline DesktopTextApp app5{"App Five", "Stub app 5"};
    inline DesktopTextApp app6{"App Six", "Stub app 6"};
    inline DesktopTextApp app7{"App Seven", "Stub app 7"};
    inline DesktopTextApp app8{"App Eight", "Stub app 8"};

    inline MenuNode desktop_menu("Desktop");
    inline MenuNode deep1("VeryLongFolderName_DesktopHarness_WithAbsurdlyVerboseNaming");
    inline MenuNode deep2("AnotherRidiculouslyLongSubfolderName_ToForceOverflowInBreadcrumbs");
    inline MenuNode deep3("YetAnotherDeepFolder_With_EvenMoreCharacters_And_More");
    inline MenuNode apps_menu("Apps (8)");
    inline MenuNode root("Root");

    inline void build_menu_tree()
    {
        root.children.clear();
        desktop_menu.children.clear();
        deep1.children.clear();
        deep2.children.clear();
        deep3.children.clear();
        apps_menu.children.clear();

        root.add_child(&desktop_menu);

        desktop_menu.add_child(&deep1);
        deep1.add_child(&deep2);
        deep2.add_child(&deep3);
        deep3.add_child(&apps_menu);

        apps_menu.add_child(&desktop_list_demo);
        apps_menu.add_child(&desktop_confirm_demo);
        apps_menu.add_child(&app1);
        apps_menu.add_child(&app2);
        apps_menu.add_child(&app3);
        apps_menu.add_child(&app4);
        apps_menu.add_child(&app5);
        apps_menu.add_child(&app6);
        apps_menu.add_child(&app7);
        apps_menu.add_child(&app8);
    }
}
