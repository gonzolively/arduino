# Annoyatron – Multi-Mode

A low-power "Annoyatron"-style device built around an ATtiny85 microcontroller, powered by a coin cell battery, with support for multiple modes that can be selected with a tactile push button.

- **Mode 1**: High-pitched, watch-like beep and the original annoyatron tone.
- **Mode 2**: Various and random tones, including the tone from mode 1, and many others.
- **Mode 3**: "Cricket" chirp. Note, this mode plays tones more frequently than the other two modes to emulate natural cricket sounds.

![Front View](images/front.jpg)

## Hardware

**Required**
- [ATtiny85](https://s.click.aliexpress.com/e/_oDQX1nd) (8-pin DIP) + [socket](https://s.click.aliexpress.com/e/_olj3N3z) (optional, but handy)
- [Piezo Buzzer (TMB12A05)](https://s.click.aliexpress.com/e/_onY2x6d) (Be sure to select the 5-volt variant as listed here.)
- [Coin cell holder](https://s.click.aliexpress.com/e/_okgPNi7) + [CR2032/2025/2016 battery](https://www.aliexpress.us/item/3256805763327909.html?spm=a2g0o.cart.0.0.4e8838daufVBVd&mp=1&pdp_npi=5%40dis%21USD%21USD%207.15%21USD%205.08%21%21USD%204.90%21%21%21%402101eac917569158391367885ee5ac%2112000034988159617%21ct%21US%216342416047%21%211%210&gatewayAdapt=glo2usa)
- [Tactile Push Button](https://s.click.aliexpress.com/e/_oEAKt9T)
- [Small protoboard](https://s.click.aliexpress.com/e/_oFym2z5) (1x1 inch or larger)

**Optional:**
- [Adhesive Magnets](https://s.click.aliexpress.com/e/_oFuSSpN) - Handy for placing the device in hard-to-reach areas

**Note:** If you choose a speaker other than the one listed here, you will likely need to tweak the code or add resistors to your setup, depending on which speaker you choose to get the desired tones.

## Usage
1. Flash the `annoyatron_multi.ino` sketch onto the ATtiny85 (see the repo [README](../../README.md) for programming bare AVR chips with the Arduino IDE).
2. Insert into the DIP socket on your board.
3. Wire battery, speaker, and DPDT switch as shown in [`docs/wiring.md`](docs/wiring.md).
4. Power on → device will give 3 quick startup beeps.
5. Use the tactile push button to select mode (**Mode 1** enabled by default at startup):
   - **1st push** Mode 1 (Simple tone) → Mode 2 (Multi-tone)
   - **2nd push** Mode 2 (Multi-tone) → Mode 3 (Cricket Sounds)
   - **3rd push, and so on** Mode 3 (Cricket Sounds) → Mode 1 (Simple tone)
