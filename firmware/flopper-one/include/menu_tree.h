#pragma once
#include "apps/hello_world_app.h"
#include "apps/hello_world_app_two.h"
#include "apps/ir_sniffer_app.h"
#include "apps/ir_emitter_app.h"
#include "apps/ir_led_test_app.h"
#include "apps/wifi_scan_app.h"
#include "apps/wifi_ap_sink_app.h"
#include "apps/ble_scan_app.h"
#include "apps/ble_sink_app.h"
#include "apps/ble_hid_app.h"
#include "apps/ble_mitm_app.h"
#include "apps/ble_sour_apple.h"
#include "apps/ble_test_app.h"

#include "apps/pn532/pn532_scan_app.h"
#include "apps/pn532/pn532_mifare_classic_app.h"
#include "apps/pn532/pn532_ntag_app.h"
#include "apps/pn532/pn532_target_app.h"
#include "apps/pn532/pn532_info_app.h"

#include "menu_node.h"
namespace flopper
{
    inline IrSnifferApp ir_sniff;
    inline IrEmitterApp ir_emit;
    inline IrLedTestApp ir_led_test;

    inline WifiScanApp wifi_scan;
    inline WifiApSinkApp wifi_ap_sink;

    inline BleScanApp ble_scan;
    inline BleSinkApp ble_sink;
    inline BleHidApp ble_hid;
    inline BleMitmApp ble_mitm;
    inline SourApple ble_sourapple;
    inline BleTestApp ble_test;

    inline Pn532ScanApp pn532_scan;
    inline Pn532MifareClassicApp pn532_mifare;
    inline Pn532NtagApp pn532_ntag;
    inline Pn532TargetApp pn532_target;
    inline Pn532InfoApp pn532_info;

    inline MenuNode ir_menu("IR");
    inline MenuNode ble_menu("BLE");
    inline MenuNode wifi_menu("WiFi");
    inline MenuNode nfc_menu("NFC");
    inline MenuNode root("Root");

    inline void build_menu_tree()
    {
        root.children.clear();
        ble_menu.children.clear();
        wifi_menu.children.clear();
        nfc_menu.children.clear();

        root.add_child(&ir_menu);
        root.add_child(&wifi_menu);
        root.add_child(&ble_menu);
        root.add_child(&nfc_menu);

        ir_menu.add_child(&ir_sniff);
        ir_menu.add_child(&ir_emit);
        ir_menu.add_child(&ir_led_test);

        wifi_menu.add_child(&wifi_scan);
        wifi_menu.add_child(&wifi_ap_sink);

        ble_menu.add_child(&ble_scan);
        ble_menu.add_child(&ble_sink);
        ble_menu.add_child(&ble_hid);
        ble_menu.add_child(&ble_mitm);
        ble_menu.add_child(&ble_sourapple);
        ble_menu.add_child(&ble_test);

        nfc_menu.add_child(&pn532_scan);
        nfc_menu.add_child(&pn532_mifare);
        nfc_menu.add_child(&pn532_ntag);
        nfc_menu.add_child(&pn532_target);
        nfc_menu.add_child(&pn532_info);
    }

}