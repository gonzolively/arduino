#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <math.h>
#ifndef ARDUINO
#include <stdint.h>
#endif

#define BODS 7
#define BODSE 2

const int PIN = 1;                     // PB1 for speaker output (OC1A)
const int BUTTON_PIN = 3;              // PB3 for mode button (active LOW w/ pullup)

// Fast DDR gate helpers for crisp AM without restarting the timer
#define PIN_BIT (1 << PIN)
inline void gateOn()  { DDRB |=  PIN_BIT; }
inline void gateOff() { DDRB &= ~PIN_BIT; }

// ----- Tuning -----
#define MEAN_WDT_TICKS 3               // average WDT ticks between cricket phrases (1 tick ≈ 8 s)
#define EXTRA_PHRASE_PCT 10            // % chance to immediately play a second phrase

const int REGULAR_HI_MS = 200;
const int VARIETY_TONE_MIN_MS = 50;
const int VARIETY_TONE_MAX_MS = 300;
const int WAKE_INDICATOR_HI_MS = 0;
const int INITIAL_BEEP_COUNT = 3;

const int SLEEP_MIN = 225;             // ~30 min
const int SLEEP_MAX = 900;             // ~120 min

const int TONE_MIN = 1;
const int TONE_MAX = 254;
const int SIMPLE_TONE = 20;

// Cricket tone
const int CRICKET_TONE_MIN = 24;
const int CRICKET_TONE_MAX = 45;

volatile int currentMode = 0;          // 0=Simple, 1=Variety, 2=Cricket
volatile bool buttonPressed = false;
volatile unsigned long lastButtonTime = 0;
const unsigned long BUTTON_DEBOUNCE_MS = 300;

volatile uint8_t lastBtnState = HIGH;

uint8_t mcucr1, mcucr2;
bool keepSleeping;
long wdtCount;

// After switching to cricket mode, skip the immediate phrase once:
volatile bool skipNextCricket = false;

// ---------- Forward decls ----------
static void cricketPhrase();
static void singleCricketChirp(uint8_t ocrBase = 0);
static int  expWdtTicks(int meanTicks);
static void cricketIndicator();

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  PCMSK |= _BV(PCINT3);
  GIMSK |= _BV(PCIE);

  randomSeed(TCNT1);

  currentMode = 0;
  playModeIndicator();
}

void loop() {
  if (buttonPressed) {
    cycleToNextMode();
    buttonPressed = false;
  }

  switch (currentMode) {
    case 0: // Simple beep
      makeTone(REGULAR_HI_MS, SIMPLE_TONE);
      goToSleep(random(SLEEP_MIN, SLEEP_MAX + 1));
      break;

    case 1: // Variety
    {
      int len = random(VARIETY_TONE_MIN_MS, VARIETY_TONE_MAX_MS + 1);
      makeTone(len, random(TONE_MIN, TONE_MAX + 1));
      goToSleep(random(SLEEP_MIN, SLEEP_MAX + 1));
    }
      break;

    case 2: // Cricket
    {
      // If we just switched into cricket mode, we already played the 3-chirp indicator.
      // Skip the immediate phrase once, then sleep.
      if (skipNextCricket) {
        skipNextCricket = false;
        int ticks = expWdtTicks(MEAN_WDT_TICKS);
        if (ticks < 1)  ticks = 1;
        if (ticks > 25) ticks = 25;
        goToSleep(ticks);
        break;
      }

      cricketPhrase();
      if (random(0, 100) < EXTRA_PHRASE_PCT) {
        delay(600 + random(0, 900));   // 0.6–1.5 s inter-phrase gap
        cricketPhrase();
      }
      int ticks = expWdtTicks(MEAN_WDT_TICKS);
      if (ticks < 1)  ticks = 1;
      if (ticks > 25) ticks = 25;
      goToSleep(ticks);
    }
      break;
  }
}

void cycleToNextMode() {
  currentMode = (currentMode + 1) % 3;

  // If switching into cricket mode, we will show a 3-chirp indicator
  // and then purposely skip the first phrase once.
  if (currentMode == 2) {
    skipNextCricket = true;
  }

  playModeIndicator();

  unsigned long start = millis();
  while (digitalRead(BUTTON_PIN) == LOW && (millis() - start) < 1000) {}
  delay(20);
}

// ---------- Indicators ----------
void playModeIndicator() {
  switch (currentMode) {
    case 0: // three simple beeps
      for (int i = 0; i < INITIAL_BEEP_COUNT; i++) {
        makeTone(REGULAR_HI_MS, SIMPLE_TONE);
        if (i < INITIAL_BEEP_COUNT - 1) delay(100);
      }
      break;

    case 1: // three random tones
      for (int i = 0; i < INITIAL_BEEP_COUNT; i++) {
        int len = random(VARIETY_TONE_MIN_MS, VARIETY_TONE_MAX_MS + 1);
        makeTone(len, random(TONE_MIN, TONE_MAX + 1));
        if (i < INITIAL_BEEP_COUNT - 1) delay(100);
      }
      break;

    case 2: // exactly three cricket chirps (true chirps, not plain tones)
      cricketIndicator();
      break;
  }
}

// Play exactly 3 short chirps (not a whole phrase)
static void cricketIndicator() {
  for (int i = 0; i < 3; i++) {
    // Use a mid-range OCR so the indicator is consistent and “crickety”
    uint8_t mid = (CRICKET_TONE_MIN + CRICKET_TONE_MAX) / 2;
    singleCricketChirp(mid);
    if (i < 2) delay(120);
  }
}

// ---------- Cricket logic ----------

// One whole phrase: N chirps with per-chirp variation (no long rest here)
static void cricketPhrase() {
  int chirps = random(2, 11); // 2–10 chirps

  for (int c = 0; c < chirps; c++) {
    singleCricketChirp();     // random pitch per chirp
    // Jittery inter-chirp gap to avoid mechanical rhythm
    delay(60 + random(0, 140));   // 60–200 ms
  }
}

// One chirp (stable carrier + AM gate + tiny FM sweep)
static void singleCricketChirp(uint8_t ocrBase) {
  // If ocrBase==0, choose random in the preferred cricket range
  int baseOCR = ocrBase ? ocrBase : random(CRICKET_TONE_MIN, CRICKET_TONE_MAX + 1);

  int fmSpan  = max(1, (baseOCR * 3) / 100);      // ~+3% upward sweep
  int ocrLow  = baseOCR - fmSpan / 2;
  int ocrHigh = baseOCR + fmSpan / 2;

  int chirpMs   = random(45, 85);                 // 45–85 ms per chirp
  int gateHz    = random(230, 320);               // 230–320 Hz AM
  unsigned gatePeriodUs = 1000000UL / gateHz;

  int dutyStart = 48, dutyPeak = 62, dutyEnd = 52; // A/D envelope

  startTone(baseOCR);
  unsigned long tStart = micros();
  unsigned long tEnd   = tStart + (unsigned long)chirpMs * 1000UL;

  while ((long)(micros() - tEnd) < 0) {
    unsigned long now = micros();
    float prog = (float)(now - tStart) / (float)(tEnd - tStart);
    if (prog < 0) prog = 0; if (prog > 1) prog = 1;

    int ocr = ocrLow + (int)((ocrHigh - ocrLow) * prog);
    if (ocr < 10) ocr = 10; if (ocr > 200) ocr = 200;
    OCR1C = ocr;

    int duty;
    if (prog < 0.5f)
      duty = dutyStart + (int)((dutyPeak - dutyStart) * (prog / 0.5f));
    else
      duty = dutyPeak + (int)((dutyEnd  - dutyPeak) * ((prog - 0.5f) / 0.5f));
    if (duty < 40) duty = 40; if (duty > 75) duty = 75;

    unsigned onUs  = (gatePeriodUs * (unsigned)duty) / 100U;
    unsigned offUs = (gatePeriodUs - onUs);

    gateOn();
    delayMicroseconds(onUs);
    gateOff();
    delayMicroseconds(offUs);
  }

  stopTone();
}

// Exponential random sleep ticks
static int expWdtTicks(int meanTicks) {
  long r = random(1, 10001);   // 1..10000
  float U = r / 10000.0f;      // (0,1]
  float x = -log(U) * (float)meanTicks;
  int ticks = (int)(x + 0.5f);
  if (ticks < 1) ticks = 1;
  return ticks;
}

// ---------- Button + tone + sleep ----------
ISR(PCINT0_vect) {
  uint8_t now = digitalRead(BUTTON_PIN);
  unsigned long t = millis();

  if (lastBtnState == LOW && now == HIGH && (t - lastButtonTime) > BUTTON_DEBOUNCE_MS) {
    buttonPressed = true;
    lastButtonTime = t;
  }
  lastBtnState = now;
}

void makeTone(int msOfTone, int toneValue) {
  startTone(toneValue);
  delay(msOfTone);
  stopTone();
}

void startTone(int toneValue) {
  if (toneValue < 10)  toneValue = 10;
  if (toneValue > 200) toneValue = 200;

  pinMode(PIN, OUTPUT);
  TCCR1 = 0x92;
  OCR1C = toneValue;
}

void stopTone() {
  TCCR1 = 0x90;
  pinMode(PIN, INPUT);
}

void goToSleep(long wdtLimit)
{
  wdtCount = 0;

  do {
    if (buttonPressed) return;

    ACSR  |= _BV(ACD);
    ADCSRA &= ~_BV(ADEN);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    wdtEnable();

    cli();
    mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);
    mcucr2 = mcucr1 & ~_BV(BODSE);
    MCUCR = mcucr1;
    MCUCR = mcucr2;
    sei();
    sleep_cpu();

    cli();
    sleep_disable();
    wdtDisable();
    sei();

    if (++wdtCount < wdtLimit) {
      keepSleeping = true;
      if (WAKE_INDICATOR_HI_MS > 0) {
        switch (currentMode) {
          case 0: makeTone(WAKE_INDICATOR_HI_MS, SIMPLE_TONE); break;
          case 1: {
            int len = random(VARIETY_TONE_MIN_MS, VARIETY_TONE_MAX_MS + 1);
            makeTone(len, random(TONE_MIN, TONE_MAX + 1));
          } break;
          case 2: cricketIndicator(); break;
        }
      }
    } else {
      keepSleeping = false;
    }
  } while (keepSleeping && !buttonPressed);
}

ISR(WDT_vect) {}

void wdtEnable(void)
{
  wdt_reset();
  cli();
  MCUSR = 0x00;
  WDTCR |= _BV(WDCE) | _BV(WDE);
  WDTCR = _BV(WDIE) | _BV(WDP3) | _BV(WDP0);    // ~8192 ms
  sei();
}

void wdtDisable(void)
{
  wdt_reset();
  cli();
  MCUSR = 0x00;
  WDTCR |= _BV(WDCE) | _BV(WDE);
  WDTCR = 0x00;
  sei();
}