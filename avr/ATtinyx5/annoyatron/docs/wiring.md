# Wiring Diagram

## Component Connections
| Component | Terminal/Pin | Connection To |
|-----------|--------------|---------------|
| **ATtiny85** | Pin 1 (RESET) | NC |
| | Pin 2 (PB3) | NC |
| | Pin 3 (PB4) | NC |
| | Pin 4 (GND) | DPDT Terminal 2, Speaker negative, Battery negative |
| | Pin 5 (PB0) | Not used |
| | Pin 6 (PB1) | Speaker positive |
| | Pin 7 (PB2) | Not used |
| | Pin 8 (VCC) | Battery Positive |
| **Speaker** | Positive (+) | ATtiny85 PB1 (pin 6) |
| | Negative (-) | ATtiny85 GND (pin 4) |
| **Battery** | Positive (+) | ATtiny85 (pin 8) |
| | Negative (-) | ATtiny85 GND (pin 4) |
