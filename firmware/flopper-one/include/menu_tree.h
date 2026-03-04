#pragma once
#include "apps/hello_world_app.h"
#include "apps/hello_world_app_two.h"
#include "menu_node.h"
namespace flopper
{

    inline HelloWorldApp hello;
    inline HelloWorldAppTwo hello_two;

    inline MenuNode dir("Dir");
    inline MenuNode subdir("subdir");
    inline MenuNode sub_sub_dir("inside again!");
    inline MenuNode root("Root");

    inline void build_menu_tree()
    {
        root.add_child(&dir);
        root.add_child(&hello);
        dir.add_child(&hello_two);
        dir.add_child(&subdir);
        subdir.add_child(&sub_sub_dir);
    }

}