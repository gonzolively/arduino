#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>
#ifndef ARDUINO
#include <stdint.h>
#include "millionUtil.h"         //not needed if compiled with Arduino & Arduino-Tiny
#endif

#define BODS 7                   //BOD Sleep bit in MCUCR
#define BODSE 2                  //BOD Sleep enable bit in MCUCR

// Function declarations (forward declarations)
void goToSleep(long wdtLimit);
void makeTone(int msOfTone);
void startTone();
void stopTone();
void wdtEnable(void);
void wdtDisable(void);

// Configuration
int PIN = 0;                     // Output pin (PB0 on ATtiny85)
int REGULAR_HI_MS = 500;         // Shorter beep - more annoying than long ones
int WAKE_INDICATOR_HI_MS = 0;    // Debug indicator (0 = disabled)
int INITIAL_BEEP_COUNT = 3;      // Beep 3 times upon power on

// Sleep timing (in 8-second watchdog periods)
int RANDOM_SLEEP_MIN = 38;       // ~5 mins (38 * 8 seconds = 304s)
int RANDOM_SLEEP_MAX = 225;      // ~30 mins (225 * 8 seconds = 1800s)
//int RANDOM_SLEEP_MIN = 225;    // 30 mins for maximum stealth
//int RANDOM_SLEEP_MAX = 900;    // 2 hours for maximum stealth

// Global variables
uint8_t mcucr1, mcucr2;
bool keepSleeping;
unsigned long msNow;
unsigned long msWakeUp;
long wdtCount;                   // Counts watchdog wake-ups

void setup() {
  randomSeed(analogRead(A2));    // Seed random with analog noise
  delay(3000);                   // Wait 3 seconds before first beep

  // Initial test beeps to confirm it's working
  for (int i=0; i < INITIAL_BEEP_COUNT; i++) {
    makeTone(REGULAR_HI_MS);
    if (i < INITIAL_BEEP_COUNT - 1) {
      goToSleep(1);              // Sleep 8 seconds between test beeps
    }
  }
}

void loop() {
  makeTone(REGULAR_HI_MS);       // Make annoying sound/blink
  // Sleep for random period (5-30 minutes with current settings)
  goToSleep(random(RANDOM_SLEEP_MIN, RANDOM_SLEEP_MAX + 1));
}

void makeTone(int msOfTone) {
  pinMode(PIN, OUTPUT);
  startTone();                   // Start PWM tone generation
  delay(msOfTone);               // Let it run for specified time
  stopTone();                    // Stop the tone
  pinMode(PIN, INPUT);           // Save power by setting pin to input
}

void startTone() {
  // Generate high-pitch tone using Timer1 PWM
  // Clock values: Range from 9F-94 (with 9F being the slowest clock)
  TCCR1 = 0x94;                  // Set timer prescaler and mode
  // Pitch: Range from 255-4 (255 being the lowest pitch)
  OCR1C = 7;                     // Set frequency (lower = higher pitch)
}

void stopTone() {
  TCCR1 = 0x90;                  // Stop the timer
}

// Ultra-low power sleep using watchdog timer
// wdtLimit = number of 8-second periods to sleep
void goToSleep(long wdtLimit)
{
    msNow = millis();
    wdtCount = 0;
    
    do {
        // Disable power-hungry peripherals
        ACSR |= _BV(ACD);          // Disable analog comparator
        ADCSRA &= ~_BV(ADEN);      // Disable ADC
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        
        // Start 8-second watchdog timer
        wdtEnable();
        
        // Disable brown-out detector for ultra-low power (~0.5µA)
        cli();
        mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);
        mcucr2 = mcucr1 & ~_BV(BODSE);
        MCUCR = mcucr1;
        MCUCR = mcucr2;
        sei();

        // Go to sleep until watchdog wakes us
        sleep_cpu();

        // Wake up here after ~8 seconds
        cli();
        sleep_disable();
        wdtDisable();
        sei();

        // Check if we need to keep sleeping
        if (++wdtCount < wdtLimit) {
            keepSleeping = true;
            if (WAKE_INDICATOR_HI_MS > 0) {
              makeTone(WAKE_INDICATOR_HI_MS);  // Debug: show wake-ups
            }
        }
        else {
            keepSleeping = false;  // Done sleeping
        }
    } while (keepSleeping);
    
    msWakeUp = millis();
}

ISR(WDT_vect) {}                 // Watchdog interrupt - just wake up

// Configure watchdog for 8-second interrupt
void wdtEnable(void)
{
    wdt_reset();
    cli();
    MCUSR = 0x00;
    WDTCR |= _BV(WDCE) | _BV(WDE);
    WDTCR = _BV(WDIE) | _BV(WDP3) | _BV(WDP0);  // 8192ms timeout
    sei();
}

// Turn off watchdog
void wdtDisable(void)
{
    wdt_reset();
    cli();
    MCUSR = 0x00;
    WDTCR |= _BV(WDCE) | _BV(WDE);
    WDTCR = 0x00;
    sei();
}
