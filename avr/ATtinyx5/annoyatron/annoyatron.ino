#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>
#ifndef ARDUINO
#include <stdint.h>
#endif

#define BODS 7                         //BOD Sleep bit in MCUCR
#define BODSE 2                        //BOD Sleep enable bit in MCUCR

const int PIN = 1;                     // PB1 for speaker output
const int REGULAR_HI_MS = 200;         // Tone length
const int WAKE_INDICATOR_HI_MS = 0;
const int INITIAL_BEEP_COUNT = 3;      // number of "test" beeps before we go into the real loop

// Min/max number of 8-sec WDT periods to sleep for
const int SLEEP_MIN = 75;              // 10 mins (10 * 60 / 8)
const int SLEEP_MAX = 450;             // 60 mins (60 * 60 / 8)

// Simple tone setting
const int SIMPLE_TONE = 20;            // Fixed tone (classic "annoyatron" beep)

uint8_t mcucr1, mcucr2;
bool keepSleeping;                     //flag to keep sleeping or not
long wdtCount;                         //how many 8-sec WDT periods we've slept for

void setup() {
  // Seed the random number generator with some pseudo-random value
  randomSeed(TCNT1);

  // Initial beeps to indicate startup
  for (int i = 0; i < INITIAL_BEEP_COUNT; i++) {
    makeTone(REGULAR_HI_MS, SIMPLE_TONE);
    if (i < INITIAL_BEEP_COUNT - 1) {
      delay(100);
    }
  }
}

void loop() {
  // Simple beep mode - consistent watch beep
  makeTone(REGULAR_HI_MS, SIMPLE_TONE);
  goToSleep(random(SLEEP_MIN, SLEEP_MAX + 1));
}

void makeTone(int msOfTone, int toneValue) {
  pinMode(PIN, OUTPUT);
  startTone(toneValue);
  delay(msOfTone);  // let the tone sound for a bit
  stopTone();
  pinMode(PIN, INPUT);
}

void startTone(int toneValue) {
  // Set up timer with specified tone value
  TCCR1 = 0x92;  // clock speed (highest to lowest values: 0x92 - 0x9F)
  OCR1C = toneValue;
}

void stopTone() {
  TCCR1 = 0x90; // stop the counter
}

// wdtLimit = number of WDT periods to wake after
void goToSleep(long wdtLimit)
{
    wdtCount = 0;

    do {
        ACSR |= _BV(ACD);                         //disable the analog comparator
        ADCSRA &= ~_BV(ADEN);                     //disable ADC
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();

        wdtEnable();              //start the WDT

        //turn off the brown-out detector.
        //must have an ATtiny45 or ATtiny85 rev C or later for software to be able to disable the BOD.
        //current while sleeping will be <0.5uA if BOD is disabled, <25uA if not.
        cli();
        mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);  //turn off the brown-out detector
        mcucr2 = mcucr1 & ~_BV(BODSE);
        MCUCR = mcucr1;
        MCUCR = mcucr2;
        sei();                         //ensure interrupts enabled so we can wake up again
        sleep_cpu();                   //go to sleep
                                       //----zzzz----zzzz----zzzz----zzzz
        cli();                         //wake up here, disable interrupts
        sleep_disable();
        wdtDisable();                  //don't need the watchdog while we're awake
        sei();                         //enable interrupts again

        if (++wdtCount < wdtLimit) {
            keepSleeping = true;
            if (WAKE_INDICATOR_HI_MS > 0) {
              makeTone(WAKE_INDICATOR_HI_MS, SIMPLE_TONE);
            }
        }
        else {
            keepSleeping = false;
        }
    } while (keepSleeping);
}

ISR(WDT_vect) {}                  //don't need to do anything here when the WDT wakes the MCU

//enable the WDT for 8sec interrupt
void wdtEnable(void)
{
    wdt_reset();
    cli();
    MCUSR = 0x00;
    WDTCR |= _BV(WDCE) | _BV(WDE);
    WDTCR = _BV(WDIE) | _BV(WDP3) | _BV(WDP0);    //8192ms
    sei();
}

//disable the WDT
void wdtDisable(void)
{
    wdt_reset();
    cli();
    MCUSR = 0x00;
    WDTCR |= _BV(WDCE) | _BV(WDE);
    WDTCR = 0x00;
    sei();
}
