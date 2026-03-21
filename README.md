# flopper-one

<img src="CAD/br1.png">

I built an open source Flipper Zero clone from scratch for a sixth of the cost.

[☞ How this got built](docs/journey.md)

Flopper One is a handheld security research tool built on an ESP32. It can send and receive IR signals, interface with NFC and RFID tags, scan for BLE devices, and run WiFi attacks — all from a d-pad and a 240x240 LCD. It fits in a pocket.

The Flipper Zero costs $200 + $60 for the WiFi add-on. Flopper One's full BOM comes to **$30.38**. The only missing features are sub-GHz radio (easily added) and iButton (pretty niche).

<table width="100%">
    <tr>
        <td>
            <img src="docs/pcb_3d_front.png" width="100%">
        </td>
        <td>
            <img src="docs/pcb_3d_back.png" width="100%">
        </td>
    </tr>
</table>

## Schematic, PCB

<img src="docs/schem/flopper_one.svg" width="100%">

<img src="docs/pcb.svg" width="100%">

## BOM

| Item                      | Link                                                               | Unit Cost | Qty |      Total |
| ------------------------- | ------------------------------------------------------------------ | --------: | --: | ---------: |
| ESP32 WROOM               | [AliExpress](https://www.aliexpress.us/item/3256805942117284.html) |     $4.71 |   1 |      $4.71 |
| TSOP38238 (IR receiver)   | [LCSC C141632](https://www.lcsc.com/product-detail/C141632.html)   |     $0.73 |   1 |      $0.73 |
| TSAL6200 (IR emitter)     | [LCSC C55528](https://www.lcsc.com/product-detail/C55528.html)     |     $0.83 |   1 |      $0.83 |
| PN532 (NFC/RFID)          | [AliExpress](https://www.aliexpress.us/item/3256806254348567.html) |     $0.99 |   1 |      $0.99 |
| ST7789 240×240 LCD        | [AliExpress](https://www.aliexpress.us/item/2255799995721426.html) |     $3.57 |   1 |      $3.57 |
| B3F tactile switches (x5) | [LCSC C271750](https://www.lcsc.com/product-detail/C271750.html)   |     $1.07 |   1 |      $1.07 |
| 2N2222A transistor        | [LCSC C118536](https://www.lcsc.com/product-detail/C118536.html)   |     $0.77 |   1 |      $0.77 |
| 1 kΩ resistors            | —                                                                  |      free |   3 |          — |
| 47–100 Ω resistors        | —                                                                  |      free |   1 |          — |
| 10 µF capacitors          | —                                                                  |      free |   2 |          — |
| Power switch              | —                                                                  |      free |   1 |          — |
| Header pins               | —                                                                  |      free |   — |          — |
| LCSC shipping             | —                                                                  |         — |   — |     $11.21 |
| PCBs (JLCPCB, min qty 5)  | —                                                                  |         — |   — |      $6.50 |
| **Total**                 |                                                                    |           |     | **$30.38** |

## How to use

Plug the esp into your computer and flash the firmware in /firmware using [platformIO](https://docs.platformio.org/en/latest/core/quickstart.html) in vscode. Use the d-pad to navigate.

See more in [firmware/README.md](firmware/README.md) !
