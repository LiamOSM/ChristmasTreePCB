// This version consists of 2 light patterns,
// a music placeholder (beeping), and all the
// framework for changing states and controlling
// individual LEDs from a setter function.

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
bool music = false;
bool mode2state = 0;
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
}

void loop() {
  if (music) {
    music = false;
    for (int i = 0; i < 100; i ++) {
      tone(spkrPin, 500);
      fullLED();
      delay(50);
      noTone();
      clrLED();
      delay(50);
    }
  }

  if (mode == 0 && !complete) { // idle
    clrLED();
    complete = 1;
  }

  if (mode == 1 && !complete) { // all on
    fullLED();
    complete = 1;
  }

  if (mode == 2 && !complete) { // flashing mode
    if (abs(millis() - lastTime) > 100) {
      lastTime = millis();
      mode2state = !mode2state; // toggle
      if (mode2state)
        fullLED();
      else
        clrLED();
    }
  }

  //  for (int i = 1; i <= 9; i ++) {
  //    setLED(i, 1);
  //    delay(90);
  //  }
  //  for (int i = 1; i <= 9; i ++) {
  //    setLED(i, 0);
  //    delay(90);
  //  }
  //  for (int i = 9; i >= 1; i --) {
  //    setLED(i, 1);
  //    delay(90);
  //  }
  //  for (int i = 9; i >= 1; i --) {
  //    setLED(i, 0);
  //    delay(90);
  //  }

  int reading = analogRead(A0);
  if (abs(reading - btnLightThreshold) < tolerance) { // light button pressed
    complete = 0;
    mode++;
    if (mode >= 3)
      mode = 0;
    delay(300);
  }
  if (abs(reading - btnMusicThreshold) < tolerance) { // music button pressed
    music = true;
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