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

Then I thought it'd be fun to add power management. _"fun."_ If only it was simple [like the xiao](https://wiki.seeedstudio.com/xiao_nrf54l15_sense_getting_started/#battery-powered-board)! I added the TP4056 IC because I was feeling fancy. I asked on slack, and it turns out you need a boost converter. To get your 3.7v battery up to 5v... so that it can go back down to 3.3v. I used a module for the boost converter. But I still got to add fun charging status LEDs! Also I got the "opportunity" to google translate Chinese datasheets.

<img src="docs/tp4056.png" width="30%">

I also discovered some... interesting hacks like how the VIN pin on the esp [is actually 5v out](https://esp32.com/viewtopic.php?t=11904) when the usb is plugged in. Meaning we can charge from VIN... and then later discharge also to VIN.

I added a physical switch to prevent backflow. So congratulations, users are now trusted with power supply logic!
