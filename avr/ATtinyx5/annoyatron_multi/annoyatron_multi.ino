#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>
#ifndef ARDUINO
#include <stdint.h>
#endif

#define BODS 7                   //BOD Sleep bit in MCUCR
#define BODSE 2                  //BOD Sleep enable bit in MCUCR

int PIN = 1;                     // PB1 for speaker output
int MODE1_PIN = 3;               // PB3 for simple beep mode detection
int MODE2_PIN = 4;               // PB4 for variety mode detection

int REGULAR_HI_MS = 200;
int WAKE_INDICATOR_HI_MS = 0;
int INITIAL_BEEP_COUNT = 3;      // number of "test" beeps before we go into the real loop

// Min/max number of 8-sec WDT periods to sleep for
int SIMPLE_SLEEP_MIN = 75;       // 10 mins (5 * 60 / 8)
int SIMPLE_SLEEP_MAX = 450;      // 60 mins (10 * 60 / 8)
int VARIETY_SLEEP_MIN = 75;      // 10 mins (10 * 60 / 8)
int VARIETY_SLEEP_MAX = 450;     // 60 mins (60 * 60 / 8

// Tone declaration
int TONE_MIN = 1;                // Minimum OCR1C value (higher pitch)
int TONE_MAX = 254;              // Maximum OlCR1C value (lower pitch)
int SIMPLE_TONE = 20;            // Fixed tone for simple mode (classic "annoyatron" beep)

uint8_t mcucr1, mcucr2;
bool keepSleeping;               //flag to keep sleeping or not
unsigned long msNow;             //the current time from millis()
unsigned long msWakeUp;          //the time we woke up
long wdtCount;                   //how many 8-sec WDT periods we've slept for

void setup() {
  // Set up mode detection pins with internal pull-ups
  pinMode(MODE1_PIN, INPUT_PULLUP);  // PB3
  pinMode(MODE2_PIN, INPUT_PULLUP);  // PB4

  // Seed the random number generator with some pseudo-random value
  randomSeed(TCNT1);
  
  // Check mode at startup and give appropriate initial beeps
  bool isSimpleMode = (digitalRead(MODE1_PIN) == LOW);

  for (int i = 0; i < INITIAL_BEEP_COUNT; i++) {
    if (isSimpleMode) {
      makeTone(REGULAR_HI_MS, SIMPLE_TONE);
    } else {
      makeTone(REGULAR_HI_MS, random(TONE_MIN, TONE_MAX + 1));
    }
    if (i < INITIAL_BEEP_COUNT - 1) {
      delay(100);
    }
  }
}

void loop() {
  // Check which mode we're in based on switch position
  bool isSimpleMode = (digitalRead(MODE1_PIN) == LOW);
  bool isVarietyMode = (digitalRead(MODE2_PIN) == LOW);

  if (isSimpleMode) {
    // Simple beep mode (left switch position)
    makeTone(REGULAR_HI_MS, SIMPLE_TONE);
    goToSleep(random(SIMPLE_SLEEP_MIN, SIMPLE_SLEEP_MAX + 1));
  }
  else if (isVarietyMode) {
    // Variety mode (right switch position)
    makeTone(REGULAR_HI_MS, random(TONE_MIN, TONE_MAX + 1));
    goToSleep(random(VARIETY_SLEEP_MIN, VARIETY_SLEEP_MAX + 1));
  }
  else {
    // Neither button pressed - just wait and check again
    delay(1000);
  }
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
    msNow = millis();
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
        sei();                         //enable interrupts again (but INT0 is disabled above)

        if (++wdtCount < wdtLimit) {
            keepSleeping = true;
            if (WAKE_INDICATOR_HI_MS > 0) {
              // Check mode for wake indicator beep
              bool isSimpleMode = (digitalRead(MODE1_PIN) == LOW);
              if (isSimpleMode) {
                makeTone(WAKE_INDICATOR_HI_MS, SIMPLE_TONE);
              } else {
                makeTone(WAKE_INDICATOR_HI_MS, random(TONE_MIN, TONE_MAX + 1));
              }
            }
        }
        else {
            keepSleeping = false;
        }
    } while (keepSleeping);
    
    msWakeUp = millis();
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
