# Annoyatron
A low-power "Annoyatron"-style device built around an ATtiny85 microcontroller.  
It emits short periodic beeps at random intervals, powered by a coin cell battery.

## Hardware
See [`docs/wiring.md`](docs/wiring.md) for the wiring diagram.  
- ATtiny85 (8-pin DIP) + socket
- Piezo Speaker – NOT Buzzer (sometimes called "mini speaker")
- Coin cell holder + CR2032/2025/2016 battery
- Small protoboard (1x1 inch or larger)
- DPDT slide switch

**Note:** This code was developed specifically for [this](https://www.aliexpress.us/item/3256808491143220.html?spm=a2g0o.order_list.order_list_main.17.2f711802udN9xq&gatewayAdapt=glo2usa) Piezo speaker I bought on AliExpress which gave  
me the exact tones I was looking for. You can also use the TMB12A03 "active buzzer" and it will give you very similar tones, though "flatter" and less "musical". [This](https://www.sparkfun.com/mini-speaker-pc-mount-12mm-2-048khz.html) speaker from Adafruit MAY work, though I have not tested it.  
All in all, you will likely need to tweak the code, or add resistors to your setup depending on which speaker you choose to get the desired tones.

## Usage
1. Flash the `annoyatron.ino` sketch onto the ATtiny85 (see the repo [README](../../README.md) for programming bare AVR chips with the Arduino IDE).
2. Insert into DIP socket on your board.
3. Wire battery, speaker, and DPDT switch as shown in [`docs/wiring.md`](docs/wiring.md).
4. Power on → device will give 3 quick startup beeps.
