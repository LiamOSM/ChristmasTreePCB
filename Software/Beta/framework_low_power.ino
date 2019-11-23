// This code is basically the same as framework.ino, except
// I've added the low_power() function, which is executed
// when mode = 0. This function disables the ADC and some
// other things, which reduces the power consumption.
// Previously, the "off" current consumption was 1mA, which
// would drain the 200mAh battery in 8 days. Now, the "off"
// current consumption is a mere 0.2uA which will last a
// whopping 114 years! Hopefully the CR2032 battery form-
// factor is still being used in the year 2133. If not, any
// 3V to 5V battery will work, and my apologies for not
// making my design "future proof".

#include <avr/sleep.h>
#include <EEPROM.h>

const int spkrPin = 4; // Speaker
const int led5Pin = 3; // LED5 (tip)
const int clockPin = 2; // SH_CP
const int latchPin = 1; // ST_CP
const int dataPin = 0; // DS
const int btnPin = A0; // Buttons

const int btnMusicThreshold = 902;
const int btnLightThreshold = 798;
const int btnBothThreshold  = 722;
const int tolerance = 30;

uint8_t sr = 0b00000000;
uint8_t mode = 0;
uint8_t complete = 0;
uint8_t music = 0;
unsigned long lastTime = 0;

// modes:
// 0 = off
// 1 = all on
// 2 = single chase
// 3 = inverted chase
// 4 = side to side
// 5 = ???

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(led5Pin, OUTPUT);
  pinMode(spkrPin, OUTPUT);

  // clear LEDs ASAP (SR sometimes sets them high)
  clrLED();

  // on startup, get new state from EEPROM
  mode = EEPROM.read(0);
  if (mode > 2)
    mode = 0;
  delay(300);
  // write next state into EEPROM
  EEPROM.write(0, mode + 1);
}

void loop() {
  static bool mode2state = 0;
  if (music) {
    music = 0;
    for (int i = 0; i < 100; i ++) {
      tone(spkrPin, 500);
      fullLED();
      delay(50);
      noTone();
      clrLED();
      delay(50);
    }
    complete = 0; //
  }

  if (mode == 0 && !complete) { // idle
    clrLED();
    complete = 1;
    low_power();
    // once low_power() is called, the microcontroller
    // is basically "bricked" until reset is pressed
  }

  if (mode == 1 && !complete) { // all on
    fullLED();
    complete = 1;
  }

  if (mode == 2 && !complete) { // flashing mode
    if (abs(millis() - lastTime) > 200) {
      lastTime = millis();
      mode2state = !mode2state; // toggle
      if (mode2state)
        fullLED();
      else
        clrLED();
    }
  }

  int reading = analogRead(A0);
  // check if music button pressed
  if (abs(reading - btnMusicThreshold) < tolerance) {
    music = 1;
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
