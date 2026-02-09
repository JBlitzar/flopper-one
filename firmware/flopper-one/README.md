# Flopper-one firmware

Hello, this is the [platformio](https://platformio.org/) project directory for the flopper one firmware. To not be presumptuous, it's currently set to run a relatively conservative self-test. I've found that things often go wrong, and it's better to debug that early. I have great plans for this in the future. For now it just prints debug statements to the serial console.

All of the hardware pins should be corroborated by the schematic (see [`include/flopper_pins.h`](include/flopper_pins.h))

## A note on AI usage

I used AI for programming portions of this firmware, since I'm not too experienced with platformio. I still feel as if I understand all the code. Don't think of this as a no-effort vibe-coded software stack.

This aligns with Blueprint's AI policy. From the FAQ canvas:

> _Can I use AI?_
>
> _**Yes as guidance/help for code**, no for journals and the rest of your project. This is your project, not AI’s. AI projects will lead to a rejection._

(emphasis mine)

## Roadmap

- Make sure everything works!
- proper menu system
- integrate peripherals, classic flipper functionality (sniff, emit, etc)
  - tv remote emu
  - ble hid
  - easy save / replay on all
- wifi pranks feat. marauder
- polish / boot screen etc
- openhaystack integration? https://github.com/seemoo-lab/openhaystack/tree/main/Firmware/ESP32
- funny usb stuff
- logic analyzer / i2c debugger
