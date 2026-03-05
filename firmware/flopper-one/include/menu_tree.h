#pragma once
#include "apps/hello_world_app.h"
#include "apps/hello_world_app_two.h"
#include "apps/ir_sniffer_app.h"
#include "apps/ir_emitter_app.h"
#include "apps/wifi_scan_app.h"
#include "apps/ble_scan_app.h"
#include "apps/ble_sink_app.h"
#include "apps/ble_hid_app.h"
#include "apps/ble_mitm_app.h"
#include "menu_node.h"
namespace flopper
{
    inline IrSnifferApp ir_sniff;
    inline IrEmitterApp ir_emit;

    inline WifiScanApp wifi_scan;

    inline BleScanApp ble_scan;
    inline BleSinkApp ble_sink;
    inline BleHidApp ble_hid;
    inline BleMitmApp ble_mitm;

    inline MenuNode ir_menu("IR");
    inline MenuNode ble_menu("BLE");
    inline MenuNode wifi_menu("WiFi");
    inline MenuNode root("Root");

    inline void build_menu_tree()
    {
        root.children.clear();
        ble_menu.children.clear();
        wifi_menu.children.clear();

        root.add_child(&ir_menu);
        root.add_child(&wifi_menu);
        root.add_child(&ble_menu);

        ir_menu.add_child(&ir_sniff);
        ir_menu.add_child(&ir_emit);

        wifi_menu.add_child(&wifi_scan);

        ble_menu.add_child(&ble_scan);
        ble_menu.add_child(&ble_sink);
        ble_menu.add_child(&ble_hid);
        ble_menu.add_child(&ble_mitm);
    }

}