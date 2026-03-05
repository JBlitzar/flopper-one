#pragma once
#include "apps/hello_world_app.h"
#include "apps/hello_world_app_two.h"
#include "apps/ir_sniffer_app.h"
#include "apps/ir_emitter_app.h"
#include "apps/ble_scan_app.h"
#include "apps/ble_sink_app.h"
#include "apps/ble_hid_app.h"
#include "apps/ble_mitm_app.h"
#include "menu_node.h"
namespace flopper
{

    inline HelloWorldApp hello;
    inline HelloWorldAppTwo hello_two;

    inline IrSnifferApp ir_sniff;
    inline IrEmitterApp ir_emit;

    inline BleScanApp ble_scan;
    inline BleSinkApp ble_sink;
    inline BleHidApp ble_hid;
    inline BleMitmApp ble_mitm;

    inline MenuNode dir("Dir");
    inline MenuNode subdir("subdir");
    inline MenuNode sub_sub_dir("inside again!");

    inline MenuNode ir_menu("IR");
    inline MenuNode ble_menu("BLE");
    inline MenuNode root("Root");

    inline void build_menu_tree()
    {
        root.children.clear();
        dir.children.clear();
        subdir.children.clear();
        sub_sub_dir.children.clear();
        ir_menu.children.clear();
        ble_menu.children.clear();

        root.add_child(&ir_menu);
        root.add_child(&ble_menu);
        root.add_child(&hello);
        root.add_child(&dir);

        ir_menu.add_child(&ir_sniff);
        ir_menu.add_child(&ir_emit);

        ble_menu.add_child(&ble_scan);
        ble_menu.add_child(&ble_sink);
        ble_menu.add_child(&ble_hid);
        ble_menu.add_child(&ble_mitm);

        dir.add_child(&hello_two);
        dir.add_child(&subdir);
        subdir.add_child(&sub_sub_dir);
    }

}