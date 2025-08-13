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

int PIN = 1;
int REGULAR_HI_MS = 200;
int WAKE_INDICATOR_HI_MS = 0; //200;
int INITIAL_BEEP_COUNT = 3;   // number of "test" beeps before we go into the real loop

// min/max number of 8-sec WDT periods to sleep for
//int RANDOM_SLEEP_MIN = 7;     // 1 min (1 * 60 / 8)
//int RANDOM_SLEEP_MAX = 30;    // 4 mins (4 * 60 / 8)

// debug
int RANDOM_SLEEP_MIN = 99;     // 1 min (1 * 60 / 8)
int RANDOM_SLEEP_MAX = 99;    // 4 mins (4 * 60 / 8)

uint8_t mcucr1, mcucr2;
bool keepSleeping;                   //flag to keep sleeping or not
unsigned long msNow;                 //the current time from millis()
unsigned long msWakeUp;              //the time we woke up
long wdtCount;                       //how many 8-sec WDT periods we've slept for

void setup() {
  for (int i=0; i < INITIAL_BEEP_COUNT - 1; i++) {
    makeTone(REGULAR_HI_MS);
    goToSleep(1);
    Serial.write("hello");
  }
}

void loop() {
  makeTone(REGULAR_HI_MS);
  goToSleep(random(RANDOM_SLEEP_MIN, RANDOM_SLEEP_MAX + 1));
}

void makeTone(int msOfTone) {
  pinMode(PIN, OUTPUT);
  startTone();
  delay(msOfTone);  // let the tone sound for a bit
  stopTone();
  pinMode(PIN, INPUT);
}

void startTone() {
  TCCR1 = 0x99;  // clock speed (highest to lowest values: 0x94 - 0x9F)
  OCR1C = 100;     // Pitch (highest to lowest values: 255-4)
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
              makeTone(WAKE_INDICATOR_HI_MS);            //briefly blink an LED so we can see the wdt wake-ups
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
