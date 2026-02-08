# flopper-one

<table width="100%">
    <tr>
        <td>
            <img src="docs/3dfront.png" width="100%">
        </td>
        <td>
            <img src="docs/3dback.png" width="100%">
        </td>
    </tr>
</table>

Flipper zero-type firmware with an ESP32. Featuring IR, NFC, BLE, wifi, with a dpad and an lcd display. Not designed using any guide!

I've always thought that the flipper zero was pretty cool, but the $200 price point made it seem out of reach.

I joined up blueprint, made the [hackpad](https://github.com/JBlitzar/blitzypad), learned a lot and had a lot of fun.

Then I saw the [sorta flipper zero](https://blueprint.hackclub.com/projects/7192). This was truly inspring. I saw it, and I thought to myself, _I could totally do that_. And it turns out I have like half the parts! The ESP has wifi and bluetooth built in (you usually need to pay like $60 extra for wifi on a flipper), all that's left is to add... all the other stuff.

Since this is my second PCB project, I opted for a module build. In the end, this made the pcb quite crowded and taught me a lot about good routing (and I still had to use the smd TP4056 IC, you'll see later)

Basically, after the ESP, we need a UI, IR, NFC, and RFID.

The UI is some buttons in a dpad and a display.

IR is incredibly easy. It's literally an LED (with some passives sprinkled in), and then a TSOP38238 to recieve.

For NFC/RFID, I opted for the PN532 module. I breifly entertained the idea of using the bare PN532 chip and fully integrating it, but the [datasheet](https://www.nxp.com/docs/en/nxp/data-sheets/PN532_C1.pdf) is 222 pages long and the application circuit (page 212, figure 51) seems pretty involved if I do say so myself.

<img src="docs/nfc.png" width="30%">

> _incomplete nfc circuit... yeah its pretty complicated_
>
> At this point it's cheaper AND smaller to just use the module. So that's what I did

Then I thought it'd be fun to add power management. _"fun."_ If only it was simple like the xiao! I added the TP4056 IC because I was feeling fancy. I asked on slack, and it turns out you need a boost converter. To get your 3.7v battery up to 5v... so that it can go back down to 3v. I used a module for the boost converter. But I still got to add fun charging status LEDs! Also I got the "opportunity" to google translate Chinese datasheets.

<img src="docs/tp4056.png" width="30%">

I discovered some... interesting hacks like how the VIN pin on the esp [is actually 5v out](https://esp32.com/viewtopic.php?t=11904) when the usb is plugged in. Meaning we can charge from VIN and then discharge also to VIN.

## Schematic

<img src="docs/schematic.png" width="100%">

## Assembly

<img src="CAD/together.png" width="100%">

IMPORTANT: See [CAD/README.md](CAD/README.md) for notes.

## PCB

<img src="docs/pcb.png" width="100%">

## BOM in table format

see BOM.csv.

This BOM format has been [approved](https://hackclub.slack.com/archives/C09CMJV6V6K/p1770569396266989) by the blueprint team

| Item              | Link                                                  | Cost | Amount | total cost of | total | Gotten? |
| ----------------- | ----------------------------------------------------- | ---- | ------ | ------------- | ----- | ------- |
| esp32 wroom       | already own                                           | 0    | 1      | 0             | 30.87 | TRUE    |
| TSOP38238         | https://www.aliexpress.us/item/3256806628003365.html  | 1.55 | 1      | 1.55          |       | FALSE   |
| TSAL6200          | https://www.aliexpress.us/item/3256805924747789.html  | 2.42 | 1      | 2.42          |       | FALSE   |
| PN532             | https://www.aliexpress.us/item/3256806254348567.html  | 0.99 | 1      | 0.99          |       | FALSE   |
| ST7789            | https://www.aliexpress.us/item/2255799995721426.html  | 3.57 | 1      | 3.57          |       | FALSE   |
| 5x b3fs           | https://www.aliexpress.us/item/3256807254489666.html  | 3.69 | 1      | 3.69          |       | FALSE   |
| 10x TP4056 (moq)  | https://www.aliexpress.us/item/3256811339022742.html  | 1.19 | 1      | 1.19          |       | FALSE   |
| lipo battery      | already own                                           | 0    | 1      | 0             |       | TRUE    |
| MT3608 module     | https://www.aliexpress.us/item/3256808032184992.html? | 6.97 | 1      | 6.97          |       | FALSE   |
| shipping          | n/a                                                   | 5.19 | 1      | 5.19          |       | FALSE   |
| 5x PCB (moq)      | n/a                                                   | 5.3  | 1      | 5.3           |       | FALSE   |
| switch            | already own                                           | 0    | 1      | 0             |       | TRUE    |
| 1kΩ resistor      | can get for free?                                     | 0    | 3      | 0             |       | FALSE   |
| 47-100Ω resistor  | can get for free?                                     | 0    | 1      | 0             |       | FALSE   |
| 1.2kΩ resistor    | can get for free?                                     | 0    | 1      | 0             |       | FALSE   |
| 10uf capacitor    | can get for free?                                     | 0    | 1      | 0             |       | FALSE   |
| 2n2219            | can get for free?                                     | 0    | 1      | 0             |       | FALSE   |
| LEDS (green, red) | already own                                           | 0    | 2      | 0             |       | TRUE    |
| header pins       | can get for free                                      | 0    | 2      | 0             |       | FALSE   |
