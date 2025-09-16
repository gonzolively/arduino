#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>
#ifndef ARDUINO
#include <stdint.h>
#endif

#define BODS 7                         // BOD Sleep bit in MCUCR
#define BODSE 2                        // BOD Sleep enable bit in MCUCR

const int PIN = 1;                     // PB1 for speaker output (OC1A)
const int BUTTON_PIN = 3;              // PB3 for mode button (active LOW w/ pullup)

// Fast DDR gate helpers for crisp AM without restarting the timer
#define PIN_BIT (1 << PIN)
inline void gateOn()  { DDRB |=  PIN_BIT; }   // drive PB1 (output, OC1A active)
inline void gateOff() { DDRB &= ~PIN_BIT; }   // tri-state PB1 (input, silences carrier)

const int REGULAR_HI_MS = 200;         // Tone length for simple mode
const int VARIETY_TONE_MIN_MS = 50;    // Minimum tone length for variety mode
const int VARIETY_TONE_MAX_MS = 300;   // Maximum tone length for variety mode
const int WAKE_INDICATOR_HI_MS = 0;
const int INITIAL_BEEP_COUNT = 3;      // mode indicator burst count

// Sleep (WDT @ ~8 s) windows
const int SLEEP_MIN = 225;             // ~30 min (30*60/8)
const int SLEEP_MAX = 900;             // ~120 min
const int CRICKET_SLEEP_MIN = 15;      // ~2 min
const int CRICKET_SLEEP_MAX = 60;      // ~8 min

// Tone declarations
const int TONE_MIN = 1;                // Minimum OCR1C value (higher pitch)
const int TONE_MAX = 254;              // Maximum OCR1C value (lower pitch)
const int SIMPLE_TONE = 20;            // Fixed tone for simple mode

// Cricket tone (the “previous” settings you preferred)
const int CRICKET_TONE_MIN = 24;       // higher pitch = lower OCR
const int CRICKET_TONE_MAX = 45;

// Mode variables
volatile int currentMode = 0;          // 0=Simple, 1=Variety, 2=Cricket
volatile bool buttonPressed = false;
volatile unsigned long lastButtonTime = 0;
const unsigned long BUTTON_DEBOUNCE_MS = 300;

// Edge-detected debounce (trigger on release)
volatile uint8_t lastBtnState = HIGH;  // with INPUT_PULLUP, HIGH = not pressed

uint8_t mcucr1, mcucr2;
bool keepSleeping;                     // continue WDT sleeping?
long wdtCount;                         // number of ~8s WDT ticks slept

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);   // PB3 (active LOW)
  PCMSK |= _BV(PCINT3);                // pin change on PB3
  GIMSK |= _BV(PCIE);                  // enable pin change interrupt

  randomSeed(TCNT1);                   // seed PRNG from timer

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
      makeCricketSound();
      goToSleep(random(CRICKET_SLEEP_MIN, CRICKET_SLEEP_MAX + 1));
      break;
  }
}

void cycleToNextMode() {
  currentMode = (currentMode + 1) % 3;
  playModeIndicator();

  // wait for release to avoid mode skips on long press
  unsigned long start = millis();
  while (digitalRead(BUTTON_PIN) == LOW && (millis() - start) < 1000) {}
  delay(20); // guard against rebound
}

void playModeIndicator() {
  for (int i = 0; i < INITIAL_BEEP_COUNT; i++) {
    switch (currentMode) {
      case 0: makeTone(REGULAR_HI_MS, SIMPLE_TONE); break;
      case 1: {
        int len = random(VARIETY_TONE_MIN_MS, VARIETY_TONE_MAX_MS + 1);
        makeTone(len, random(TONE_MIN, TONE_MAX + 1));
      } break;
      case 2: makeCricketSound(); break;
    }
    if (i < INITIAL_BEEP_COUNT - 1) delay(100);
  }
}

// --- Cricket: the earlier “just-right” feel (pitch & speed reverted) ---
void makeCricketSound() {
  int chirps = random(2, 5);  // 2–4 chirps per phrase

  for (int c = 0; c < chirps; c++) {
    // Base pitch with a small upward sweep (~3%)
    int baseOCR = random(CRICKET_TONE_MIN, CRICKET_TONE_MAX + 1);
    int fmSpan  = max(1, (baseOCR * 3) / 100);
    int ocrLow  = baseOCR - fmSpan/2;
    int ocrHigh = baseOCR + fmSpan/2;

    // Chirp timing & AM gate (slower than “sparkle”, like before)
    int chirpMs   = random(45, 85);               // 45–85 ms per chirp
    int gateHz    = random(230, 320);             // 230–320 Hz AM rate
    unsigned gatePeriodUs = 1000000UL / gateHz;   // period in µs

    // Duty envelope: 48% -> 62% -> 52% (keeps it natural but not muddy)
    int dutyStart = 48, dutyPeak = 62, dutyEnd = 52;

    // Start carrier once; keep timer running during the whole chirp
    startTone(baseOCR);

    unsigned long tStart = micros();
    unsigned long tEnd   = tStart + (unsigned long)chirpMs * 1000UL;

    while ((long)(micros() - tEnd) < 0) {
      unsigned long now = micros();
      float prog = (float)(now - tStart) / (float)(tEnd - tStart);
      if (prog < 0) prog = 0; if (prog > 1) prog = 1;

      // Small FM sweep (ocrLow -> ocrHigh)
      int ocr = ocrLow + (int)((ocrHigh - ocrLow) * prog);
      if (ocr < 10) ocr = 10; if (ocr > 200) ocr = 200;
      OCR1C = ocr;

      // Triangular duty envelope
      int duty;
      if (prog < 0.5f)
        duty = dutyStart + (int)((dutyPeak - dutyStart) * (prog / 0.5f));
      else
        duty = dutyPeak + (int)((dutyEnd  - dutyPeak) * ((prog - 0.5f) / 0.5f));
      if (duty < 40) duty = 40; if (duty > 75) duty = 75;

      unsigned onUs  = (gatePeriodUs * (unsigned)duty) / 100U;
      unsigned offUs = (gatePeriodUs - onUs);

      // Gate PB1 without touching the timer (stable carrier)
      gateOn();
      delayMicroseconds(onUs);
      gateOff();
      delayMicroseconds(offUs);
    }

    stopTone(); // stop carrier after the chirp

    // Gap between chirps in a phrase (snappy but not rushed)
    delay(90 + random(40, 110));
  }

  // Phrase pause (slightly shorter than original, longer than sparkle)
  delay(300 + random(0, 180));
}

ISR(PCINT0_vect) {
  uint8_t now = digitalRead(BUTTON_PIN);  // HIGH when not pressed, LOW when pressed
  unsigned long t = millis();

  // Only act on release (LOW -> HIGH)
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
  // Clamp to safe audible bounds
  if (toneValue < 10)  toneValue = 10;
  if (toneValue > 200) toneValue = 200;

  pinMode(PIN, OUTPUT);   // ensure OC1A is driving
  TCCR1 = 0x92;           // fixed prescaler/config
  OCR1C = toneValue;      // smaller OCR = higher pitch
}

void stopTone() {
  TCCR1 = 0x90;           // stop Timer1
  pinMode(PIN, INPUT);    // tri-state output
}

// --- Sleep infrastructure (WDT ~8 s) ---
void goToSleep(long wdtLimit)
{
  wdtCount = 0;

  do {
    if (buttonPressed) return;

    ACSR  |= _BV(ACD);            // disable analog comparator
    ADCSRA &= ~_BV(ADEN);         // disable ADC
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    wdtEnable();

    // Disable BOD during sleep (rev C or later)
    cli();
    mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);
    mcucr2 = mcucr1 & ~_BV(BODSE);
    MCUCR = mcucr1;
    MCUCR = mcucr2;
    sei();
    sleep_cpu();                  // ---- zzzz ----

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
          case 2: makeCricketSound(); break;
        }
      }
    } else {
      keepSleeping = false;
    }
  } while (keepSleeping && !buttonPressed);
}

ISR(WDT_vect) {}  // nothing needed—just wake

// enable the WDT for ~8 s interrupt
void wdtEnable(void)
{
  wdt_reset();
  cli();
  MCUSR = 0x00;
  WDTCR |= _BV(WDCE) | _BV(WDE);
  WDTCR = _BV(WDIE) | _BV(WDP3) | _BV(WDP0);    // ~8192 ms
  sei();
}

// disable the WDT
void wdtDisable(void)
{
  wdt_reset();
  cli();
  MCUSR = 0x00;
  WDTCR |= _BV(WDCE) | _BV(WDE);
  WDTCR = 0x00;
  sei();
}