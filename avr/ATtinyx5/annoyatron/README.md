# Annoyatron

A simple low-power "Annoyatron"-style device built around an ATtiny85 microcontroller.  
It emits short periodic beeps at random intervals, powered by a coin cell battery.

## Hardware
See [`docs/bom.md`](docs/bom.md) and [`docs/wiring.md`](docs/wiring.md) for component list and wiring diagram.  
- ATtiny85 (8-pin DIP) + socket  
- Piezo speaker (not buzzer)  
- Coin cell holder + CR2032/2025/2016 battery  
- Small protoboard  

## Usage
1. Flash the `annoyatron.ino` sketch onto the ATtiny85.  
2. Insert into DIP socket on your board.  
3. Wire battery and piezo speaker as shown in [`docs/wiring.md`](docs/wiring.md).  
4. Power on → device will give 3 quick startup beeps.  
5. After that, it will enter its cycle of periodic beeps at random intervals.  
