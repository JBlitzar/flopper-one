# flopper-one

<img src="CAD/br1.png">

I built an open source Flipper Zero clone from scratch for a sixth of the cost.

[☞ How this got built](docs/journey.md)

Flopper One is a handheld security research tool built on an ESP32. It can send and receive IR signals, interface with NFC and RFID tags, scan for BLE devices, and run WiFi attacks — all from a d-pad and a 240x240 LCD. It fits in a pocket.

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

| Item                        | Link                                                               | Cost  | Amount | total cost of | total |
| --------------------------- | ------------------------------------------------------------------ | ----- | ------ | ------------- | ----- |
|                             |                                                                    |       |        |               | 32.15 |
| esp32 wroom                 | already own                                                        | 0     | 1      | 0             |       |
| TSOP38238                   | [LCSC C141632](https://www.lcsc.com/product-detail/C141632.html)   | 0.73  | 1      | 0.73          |       |
| TSAL6200                    | [LCSC C55528](https://www.lcsc.com/product-detail/C55528.html)     | 0.83  | 1      | 0.83          |       |
| PN532                       | [AliExpress](https://www.aliexpress.us/item/3256806254348567.html) | 0.99  | 1      | 0.99          |       |
| ST7789                      | [AliExpress](https://www.aliexpress.us/item/2255799995721426.html) | 3.57  | 1      | 3.57          |       |
| 5x b3fs                     | [LCSC C271750](https://www.lcsc.com/product-detail/C271750.html)   | 1.07  | 1      | 1.07          |       |
| 5x TP4056 (moq)             | [LCSC C16581](https://www.lcsc.com/product-detail/C16581.html)     | 0.91  | 1      | 0.91          |       |
| lipo battery                | already own                                                        | 0     | 1      | 0             |       |
| MT3608 module               | [AliExpress](https://www.aliexpress.us/item/3256808032184992.html) | 6.77  | 1      | 6.77          |       |
| 5x PCB (moq)                | N/A                                                                | 5.3   | 1      | 5.3           |       |
| switch                      | already own                                                        | 0     | 1      | 0             |       |
| 1kΩ resistor                | can get for free                                                   | 0     | 3      | 0             |       |
| 47-100Ω resistor            | can get for free                                                   | 0     | 1      | 0             |       |
| 1.2kΩ resistor              | can get substitute for free                                        | 0     | 1      | 0             |       |
| 10uf capacitor              | can get for free                                                   | 0     | 2      | 0             |       |
| 2n2222a (2n2219 substitute) | [LCSC C118536](https://www.lcsc.com/product-detail/C118536.html)   | 0.77  | 1      | 0.77          |       |
| LEDs (green, red)           | already own                                                        | 0     | 2      | 0             |       |
| header pins                 | can get for free                                                   | 0     | 2      | 0             |       |
| aliexpress shipping         | N/A                                                                | 0     | 1      | 0             |       |
| lcsc shipping               | N/A                                                                | 11.21 | 1      | 11.21         |       |

## How to use

Plug the esp into your computer and flash the firmware in /firmware using [platformIO](https://docs.platformio.org/en/latest/core/quickstart.html) in vscode. Use the d-pad to navigate.

See more in [firmware/README.md](firmware/README.md) !
