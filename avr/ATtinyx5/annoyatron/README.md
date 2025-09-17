# Annoyatron

A low-power "Annoyatron"-style device built around an ATtiny85 microcontroller. It emits short periodic beeps at random intervals, powered by a coin cell battery, with a DPDT switch to change modes.

![Front View]()

## Hardware

*Required**
- [ATtiny85](https://s.click.aliexpress.com/e/_oDQX1nd) (8-pin DIP) + [socket](https://s.click.aliexpress.com/e/_olj3N3z) (optional, but handy)
- [Piezo Buzzer (TMB12A05)](https://s.click.aliexpress.com/e/_onY2x6d) (Be sure to select the 5-volt variant as listed here.)
- [Coin cell holder](https://s.click.aliexpress.com/e/_okgPNi7) + [CR2032/2025/2016 battery](https://www.aliexpress.us/item/3256805763327909.html?spm=a2g0o.cart.0.0.4e8838daufVBVd&mp=1&pdp_npi=5%40dis%21USD%21USD%207.15%21USD%205.08%21%21USD%204.90%21%21%21%402101eac917569158391367885ee5ac%2112000034988159617%21ct%21US%216342416047%21%211%210&gatewayAdapt=glo2usa)
- [Small protoboard](https://s.click.aliexpress.com/e/_oFym2z5) (1x1 inch or larger)

**Optional:**
- [Adhesive Magnets](https://s.click.aliexpress.com/e/_oFuSSpN) - Handy for placing the device in hard-to-reach areas

**Note:** If you choose a speaker other than the one listed here, you will likely need to tweak the code or add resistors to your setup, depending on which speaker you choose to get the desired tones.

## Usage
1. Flash the `annoyatron.ino` sketch onto the ATtiny85 (see the repo [README](../../README.md) for programming bare AVR chips with the Arduino IDE).
2. Insert into DIP socket on your board.
3. Wire battery, speaker, and DPDT switch as shown in [`docs/wiring.md`](docs/wiring.md).
