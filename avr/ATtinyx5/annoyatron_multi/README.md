# Annoyatron – Multi-Mode

A low-power "Annoyatron"-style device built around an ATtiny85 microcontroller with support for multiple modes.  
It emits short periodic beeps at random intervals, powered by a coin cell battery, with a DPDT switch to change modes.

![Front View](images/front.jpg)

## Hardware

**Required**
- [ATtiny85](https://s.click.aliexpress.com/e/_oDQX1nd) (8-pin DIP) + [socket](https://s.click.aliexpress.com/e/_olj3N3z) (optional, but handy)
- [Piezo Speaker](https://s.click.aliexpress.com/e/_oogEnCr) - NOT Buzzer (sometimes called "mini speaker")
- [Coin cell holder](https://s.click.aliexpress.com/e/_okgPNi7) + [CR2032/2025/2016 battery](https://www.aliexpress.us/item/3256805763327909.html?spm=a2g0o.cart.0.0.4e8838daufVBVd&mp=1&pdp_npi=5%40dis%21USD%21USD%207.15%21USD%205.08%21%21USD%204.90%21%21%21%402101eac917569158391367885ee5ac%2112000034988159617%21ct%21US%216342416047%21%211%210&gatewayAdapt=glo2usa)
- [Small protoboard](https://s.click.aliexpress.com/e/_oFym2z5) (1x1 inch or larger)
- [DPDT slide switch](https://s.click.aliexpress.com/e/_oE2SVij)

**Optional:**
- [Adhesive Magnets](https://s.click.aliexpress.com/e/_oFuSSpN) - Handy for placing the device in hard to reach areas

**Note:** This code was developed specifically for [this](https://s.click.aliexpress.com/e/_oogEnCr) speaker I bought on AliExpress which gave me the exact tones I was looking for. You can also use the TMB12A03 "active buzzer" and it will give you very similar tones, though "flatter" and less "musical". [This](https://www.sparkfun.com/mini-speaker-pc-mount-12mm-2-048khz.html) speaker from Adafruit MAY work, though I have not tested it.

The specific speaker I used is a tiny PC motherboard speaker with two wires terminating in a female header connector. You can likely find an easier-to-use speaker for this project, but if you choose the same motherboard-style speaker I used, be prepared to do some tweaking to get the connections right.

All in all, you will likely need to tweak the code, or add resistors to your setup depending on which speaker you choose to get the desired tones.

## Usage
1. Flash the `annoyatron_multi.ino` sketch onto the ATtiny85 (see the repo [README](../../README.md) for programming bare AVR chips with the Arduino IDE).
2. Insert into DIP socket on your board.
3. Wire battery, speaker, and DPDT switch as shown in [`docs/wiring.md`](docs/wiring.md).
4. Power on → device will give 3 quick startup beeps.
5. Use the DPDT switch to select mode:
   - **Left position** → Simple beep mode
   - **Middle position** → Device completely off
   - **Right position** → Variety mode
