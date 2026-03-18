#pragma once
#include "menu_node.h"
#include "app.h"
#include "input_handler.h"
#include "input_event.h"
#include <vector>
#include <string>
#include "display.h"
#include "ui.h"

namespace flopper
{

    class MenuManager : public InputHandler
    {
    public:
        static MenuManager &get_instance()
        {
            static MenuManager instance;
            return instance;
        }

        MenuManager *set_root(MenuNode *root)
        {
            this->root_ = root;

            history_.clear();

            this->history_.push_back(root);
            selected_index_ = 0;
            return this;
        }

        void on_input(InputEvent evt)
        {
            MenuNode *current = history_.back();
            auto &children = current->children;

            if (flopper::ui::apply_list_nav(evt, selected_index_, children.size()))
            {
                return;
            }

            if (evt == InputEvent::LEFT)
            {
                if (history_.size() > 1)
                {
                    history_.pop_back();
                    selected_index_ = 0;
                }
            }
            else if (evt == InputEvent::RIGHT)
            {
                // do nothing lol
            }
            else if (evt == InputEvent::CENTER)
            {
                MenuNode *selected = children[selected_index_];
                if (selected->is_app_)
                {
                    active_app_ = static_cast<App *>(selected);
                    active_app_->wants_exit_ = false;
                    active_app_->on_enter();
                    InputDispatcher::get_instance().push(active_app_);
                }
                else
                {
                    if (!selected->children.empty())
                    { // make sure subdir has children
                        history_.push_back(selected);
                        selected_index_ = 0;
                    }
                }
            }
        }

        void tick()
        {
            if (active_app_ && active_app_->wants_exit_)
            {
                active_app_ = nullptr;

                draw_menu_();
            }
            else if (active_app_)
            {
                active_app_->tick();
            }
            else
            {
                draw_menu_();
            }
        }

    private:
        MenuNode *root_ = nullptr;
        std::vector<MenuNode *> history_;
        size_t selected_index_ = 0;
        App *active_app_ = nullptr;
        MenuManager() = default;
        std::string breadcrumbs = "";
        std::vector<const char *> names;

        // copied + edited from google AI overview lol

        MenuManager(const MenuManager &) = delete;
        MenuManager &operator=(const MenuManager &) = delete;

        MenuManager(MenuManager &&) = delete;
        MenuManager &operator=(MenuManager &&) = delete;

        void draw_menu_()
        {
            breadcrumbs = "";

            for (int i = 0; i < history_.size(); i++)
            {
                if (i > 0)
                {
                    breadcrumbs += ">";
                }

                breadcrumbs += history_[i]->name;
            }

            // draw breadcrumbs at top
            flopper::ui::draw_status(Display::get_instance(), breadcrumbs.c_str());

            auto &children = history_.back()->children;
            /*
            for (int i = 0; i < children.size(); i++)
            {
                auto item = children[i];
                if (i == selected_index_)
                {
                    // draw highlighted
                }
                else
                {
                    // draw normal
                }
            }*/
            
            names.clear();
            for (auto &child : children)
                names.push_back(child->name.c_str());
            flopper::ui::draw_list(Display::get_instance(), names, flopper::ui::clamp_index(selected_index_, names.size()));
        }
    };
}