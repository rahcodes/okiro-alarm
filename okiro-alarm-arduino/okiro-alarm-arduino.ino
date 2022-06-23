#include <TimeLib.h>

// IO pins
const int BUZZER_PIN = 2;
const int MAT_BUTTON_PIN = 3;
const int LDR_PIN = A0;
const int CURTAIN_MOTOR_PIN_1 = 4;
const int CURTAIN_MOTOR_PIN_2 = 5;

// other constants
const int MAT_BUTTON_DEBOUNCE_DELAY = 100;
const int LDR_THRESHOLD = 700;
const int CURTAIN_MOTOR_RUNNING_DURATION = 6000;

// runtime variables
bool alarmOn = false;
bool runAlarm = false;
bool curtainOpen = false;
unsigned long alarmTime;
unsigned long syncedTime;
unsigned int curtainMode;

// constants for bluetooth inputs
const char END_CHAR_SYNC = '?';
const char END_CHAR_ALARM = '*';
const char END_CHAR_SET = '#';
const char END_CHAR_CUR = '$';
const char CLEAR_CHAR = '!';

// variables for mat button
int buttonLastSteadyState = HIGH;
int buttonlastFlickerableState  = HIGH;
int buttonCurrentState;
unsigned long buttonLastDebounceTime = 0;

// buffer for bluetooth input
String bluetoothBufferMsg;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(MAT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CURTAIN_MOTOR_PIN_1, OUTPUT);
  pinMode(CURTAIN_MOTOR_PIN_2, OUTPUT);
}

void loop() {
  receiveData();

  if (alarmOn) {
    if (!runAlarm && now() >= alarmTime) {
      runAlarm = true;
    }

    if (curtainMode == 1) {
      if (sunrised()) {
        openCurtain();
      }
    }

    if (runAlarm) {
      if (curtainMode == 2) {
        openCurtain();
      }

      runBuzzer();

      if (matButtonPressed()) {
        runAlarm = false;
        alarmOn = false;
      }
    }
  }

}

void receiveData() {
  char data;

  while (Serial1.available()) {
    data = Serial1.read();

    if (data == END_CHAR_SYNC) {
      syncedTime = strtoul(bluetoothBufferMsg.c_str(), NULL, 10);
      setTime(syncedTime);

      bluetoothBufferMsg = "";
    } else if (data == END_CHAR_ALARM) {
      alarmTime = strtoul(bluetoothBufferMsg.c_str(), NULL, 10);

      bluetoothBufferMsg = "";
      curtainOpen = false;
      runAlarm = false;
    } else if (data == END_CHAR_SET) {
      alarmOn = (bool) bluetoothBufferMsg[0];

      bluetoothBufferMsg = "";
    }  else if (data == END_CHAR_CUR) {
      curtainMode = bluetoothBufferMsg.toInt();

      bluetoothBufferMsg = "";
    } else if (data == CLEAR_CHAR) {
      bluetoothBufferMsg = "";
    } else {
      bluetoothBufferMsg.concat(data);
    }
  }
}

void runBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
  delay(500);
}

bool matButtonPressed() {
  buttonCurrentState = digitalRead(MAT_BUTTON_PIN);

  // set change in state as flickerable
  if (buttonCurrentState != buttonlastFlickerableState) {
    buttonLastDebounceTime = millis();

    buttonlastFlickerableState = buttonCurrentState;
  }

  // after delay if flickerable state doesn't change
  if ((millis() - buttonLastDebounceTime) > MAT_BUTTON_DEBOUNCE_DELAY) {
    if (buttonLastSteadyState == HIGH && buttonCurrentState == LOW) {
      return true;
    } else if (buttonLastSteadyState == LOW && buttonCurrentState == HIGH) {
      return false;
    }

    buttonLastSteadyState = buttonCurrentState;
  } else {
    return false;
  }
}

bool sunrised() {
  if (analogRead(LDR_PIN) >= LDR_THRESHOLD) {
    return true;
  }
  return false;
}

void openCurtain() {
  if (!curtainOpen) {
    digitalWrite(CURTAIN_MOTOR_PIN_1, HIGH);
    digitalWrite(CURTAIN_MOTOR_PIN_2, LOW);
    delay(CURTAIN_MOTOR_RUNNING_DURATION);
    digitalWrite(CURTAIN_MOTOR_PIN_1, LOW);

    curtainOpen = true;
  }
}
