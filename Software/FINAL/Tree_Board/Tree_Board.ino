#include <avr/sleep.h>
#include <EEPROM.h>
#include "pitches.h"

const uint8_t spkrPin = 4; // Speaker
const uint8_t led5Pin = 3; // LED5 (top of tree)

// shift register pins
const uint8_t clockPin = 2; // SH_CP
const uint8_t latchPin = 1; // ST_CP
const uint8_t dataPin = 0; // DS

// both buttons are on A0
// note that the "lights" button resets the chip
const int btnPin = A0; // Buttons
const uint8_t debounce = 50;
const int btnMusicThreshold = 902;
//const int btnLightThreshold = 798;
//const int btnBothThreshold  = 722;
const int tolerance = 30;

uint8_t sr = 0b00000000;
uint8_t mode = 0;

// Some light modes just need to be set once.
// This flag is used to check if that's happened.
uint8_t complete = 0;

uint8_t music = 0;
uint8_t max_state = 7; // last LED mode, rollover to zero next

// modes:
// 0 = off
// 1 = full-blast
// 2 = flashing
// 3 = see-saw
// 4 = twinkle
// 5 = chase
// 6 = inverted chase
// 7 = superlike

int notes[] = {
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_G5, NOTE_C5, NOTE_D5,
  NOTE_E5,
  NOTE_F5, NOTE_F5, NOTE_F5, NOTE_F5,
  NOTE_F5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_D5, NOTE_D5, NOTE_E5,
  NOTE_D5, NOTE_G5
};

uint8_t toneLengths[] = {
  8, 8, 4,
  8, 8, 4,
  8, 8, 8, 8,
  2,
  8, 8, 8, 8,
  8, 8, 8, 16, 16,
  8, 8, 8, 8,
  4, 4
};

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(led5Pin, OUTPUT);
  pinMode(spkrPin, OUTPUT);

  // clear LEDs ASAP (SR sets them high on reset)
  clrLED();

  // on startup, get new state from EEPROM
  mode = EEPROM.read(0);
  if (mode > max_state)
    mode = 0;
  delay(debounce);
  // write next state into EEPROM
  EEPROM.write(0, mode + 1);
}

void loop() {
  static bool mode2state = 0;
  static uint8_t mode3counter = 1;
  static bool mode4state = 0;
  static uint8_t mode56counterA = 1;
  static uint8_t mode56counterB = 2;
  static uint8_t mode7counter = 1;
  static unsigned long lastTime = 0;

  while (music) {
    uint8_t size = sizeof(notes) / sizeof(int);
    // add note counter to determine which note to play
    // reset to zero if counter == size
    // check if abs(millis() - lastTime) > noteDuration * 1.30
    for (uint8_t i = 0; i < size; i++) {
      int noteDuration = 2000 / toneLengths[i];
      tone(spkrPin, notes[i], noteDuration);
      if (i % 2) // toggle LEDs on each note
        fullLED();
      else
        clrLED();
      delay(noteDuration * 1.30);
      noTone();
    }
  }


  if (!complete) { // lights need updating
    switch (mode) {
      case 0: // idle, 0.2uA current draw
        clrLED();
        complete = 1;
        low_power();
        // once low_power() is called, the microcontroller
        // is basically "bricked" until reset is pressed
        break;
      case 1: // full-blast
        fullLED();
        complete = 1;
        break;
      case 2: // flashing
        if (abs(millis() - lastTime) > 500) { // 0.5 sec passed
          lastTime = millis();
          mode2state = !mode2state; // toggle
          if (mode2state)
            fullLED();
          else
            clrLED();
        }
        break;
      case 3: // see-saw
        if (abs(millis() - lastTime) > 80) { // 100ms passed
          lastTime = millis();
          if (mode3counter >= 1 && mode3counter <= 9)
            setLED(mode3counter, 1);
          else if (mode3counter >= 10 && mode3counter <= 18)
            setLED(mode3counter - 9, 0);
          else if (mode3counter >= 19 && mode3counter <= 27)
            setLED(9 - (mode3counter - 19), 1);
          else if (mode3counter >= 28 && mode3counter <= 36)
            setLED(9 - (mode3counter - 28), 0);
          mode3counter = (mode3counter >= 36) ? 1 : mode3counter + 1;
        }
        break;
      case 4: // twinkle
        if (abs(millis() - lastTime) > 100) { // 100ms passed
          lastTime = millis();
          mode4state = !mode4state;
          for (uint8_t i = 1; i <= 9; i ++) {
            setLED(i, (i % 2) ^ mode4state );
          }
        }
        break;
      case 5: // chase
        if (abs(millis() - lastTime) > 80) { // 50ms passed
          lastTime = millis();
          setLED(mode56counterA, 0);
          mode56counterA = (mode56counterA >= 11) ? 1 : mode56counterA + 1;
          mode56counterB = (mode56counterB >= 11) ? 1 : mode56counterB + 1;
          setLED(mode56counterB, 1);
        }
        break;
      case 6: // inverted chase
        if (abs(millis() - lastTime) > 80) { // 50ms passed
          lastTime = millis();
          setLED(mode56counterA, 1);
          mode56counterA = (mode56counterA >= 11) ? 1 : mode56counterA + 1;
          mode56counterB = (mode56counterB >= 11) ? 1 : mode56counterB + 1;
          setLED(mode56counterB, 0);
        }
        break;
      case 7: // superlike
        if (abs(millis() - lastTime) > 100) { // 50ms passed
          lastTime = millis();
          setLED(mode7counter, 0);
          setLED(10 - mode7counter, 0);
          mode7counter = (mode7counter >= 5) ? 1 : mode7counter + 1;
          setLED(mode7counter, 1);
          setLED(10 - mode7counter, 1);
        }
        break;
    }
  }

  int reading = analogRead(A0);
  // check if music button pressed
  if (abs(reading - btnMusicThreshold) < tolerance) {
    music = 1;
    EEPROM.write(0, mode);
  }
}

void setLED(int index, bool state) {
  // the states are all inverted because I'm too lazy to invert
  // each ternary statement, and I accidentalyl made them all
  // backwards originally
  switch (index) {
    case 1:
      sr = (!state) ? (sr & 0b11111110) : (sr | 0b00000001);
      break;
    case 2:
      sr = (!state) ? (sr & 0b11111101) : (sr | 0b00000010);
      break;
    case 3:
      sr = (!state) ? (sr & 0b11111011) : (sr | 0b00000100);
      break;
    case 4:
      sr = (!state) ? (sr & 0b11110111) : (sr | 0b00001000);
      break;
    case 5:
      digitalWrite(led5Pin, state ? HIGH : LOW);
      break;
    case 6:
      sr = (!state) ? (sr & 0b11101111) : (sr | 0b00010000);
      break;
    case 7:
      sr = (!state) ? (sr & 0b11011111) : (sr | 0b00100000);
      break;
    case 8:
      sr = (!state) ? (sr & 0b10111111) : (sr | 0b01000000);
      break;
    case 9:
      sr = (!state) ? (sr & 0b01111111) : (sr | 0b10000000);
      break;
    default:
      break;
  }
  if (index != 5) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, sr);
    digitalWrite(latchPin, HIGH);
  }
}

void clrLED() {
  sr = 0b00000000;
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, sr);
  digitalWrite(latchPin, HIGH);
  digitalWrite(led5Pin, LOW);
}

void fullLED() {
  sr = 0b11111111;
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, sr);
  digitalWrite(latchPin, HIGH);
  digitalWrite(led5Pin, HIGH);
}

void low_power() {
  // the chosen sleep mode (SLEEP_MODE_PWR_DOWN)
  // disables pretty much everything except the
  // watchdog timer and interrupts, neither of
  // which are being used in this program
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  ADCSRA &= ~(1 << ADEN); // disable ADC
  sleep_enable(); // set Sleep Mode Control Register enable bit to 1
  sleep_cpu(); // enter sleep mode now
}
