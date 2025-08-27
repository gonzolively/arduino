# Wiring Diagram

## Component Connections
| Component | Terminal/Pin | Connection To |
|-----------|--------------|---------------|
| **ATtiny85** | Pin 1 (RESET) | Not connected |
| | Pin 2 (PB3) | DPDT Terminal 1 |
| | Pin 3 (PB4) | DPDT Terminal 3 |
| | Pin 4 (GND) | DPDT Terminal 2, Speaker negative, Battery negative |
| | Pin 5 (PB0) | Not used |
| | Pin 6 (PB1) | Speaker positive |
| | Pin 7 (PB2) | Not used |
| | Pin 8 (VCC) | DPDT Terminal 5 |
| **DPDT Switch** | Terminal 1 | ATtiny85 PB3 (pin 2) |
| | Terminal 2 | ATtiny85 GND (pin 4) |
| | Terminal 3 | ATtiny85 PB4 (pin 3) |
| | Terminal 4 | Battery positive |
| | Terminal 5 | ATtiny85 VCC (pin 8) |
| | Terminal 6 | Battery positive |
| **Speaker** | Positive (+) | ATtiny85 PB1 (pin 6) |
| | Negative (-) | ATtiny85 GND (pin 4) |
| **Battery** | Positive (+) | DPDT Terminals 4 & 6 |
| | Negative (-) | ATtiny85 GND (pin 4) |

## Switch Position Functions
| Position | Power State | Mode Pin States | Function |
|----------|-------------|-----------------|----------|
| LEFT | ON | PB3 = LOW, PB4 = HIGH | Simple beep mode |
| MIDDLE | OFF | N/A | Device completely off |
| RIGHT | ON | PB3 = HIGH, PB4 = LOW | Variety mode |
