#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <Servo.h>

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- Pin ----------------
const int PIR_PIN       = 10;
const int SOUND_PIN     = 11;
const int SERVO_PIN     = 12;
const int BUZZER_PIN    = 13;

const int LDR_PIN       = A0;
const int MAIN_LED_PIN  = A1;
const int STEP_LED1_PIN = A2;
const int STEP_LED2_PIN = A3;

// ---------------- Keypad 4x4 ----------------
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---------------- Servo ----------------
Servo doorServo;
const int SERVO_CLOSED_ANGLE = 0;
const int SERVO_OPEN_ANGLE   = 90;

// ---------------- Logica ----------------
String correctPIN = "1234"; # 
String enteredPIN = "";

bool unlocked = false;
bool mainLightOn = false;
bool scaleLightsOn = false;

unsigned long scaleLastMotionTime = 0;
const unsigned long SCALE_TIMEOUT = 5000;

int lightThreshold = 500;

bool lastSoundState = false;
unsigned long lastSoundTrigger = 0;
const unsigned long SOUND_DEBOUNCE = 1000;

unsigned long lastKeyPressTime = 0;
const unsigned long PIN_TIMEOUT = 30000;

unsigned long soundSensorTriggerTime = 0;
const unsigned long SOUND_DISPLAY_DURATION = 700;

unsigned long keypadConfirmTime = 0;
const unsigned long KEYPAD_CONFIRM_DURATION = 1800;

// ---------------- Display ----------------
void displayClear() {
  display.clearDisplay();
  display.setCursor(0, 0);
}

void displayMessage(const String &line1, const String &line2 = "", const String &line3 = "") {
  displayClear();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println(line1);
  if (line2.length() > 0) display.println(line2);
  if (line3.length() > 0) display.println(line3);
  display.display();
}

// ---------------- Buzzer ----------------
void beep(int freq, int duration) {
  tone(BUZZER_PIN, freq, duration);
  delay(duration);
  noTone(BUZZER_PIN);
  delay(20);
}

void melodySuccess() {
  beep(1000, 120);
  beep(1300, 120);
  beep(1600, 180);
}

void melodyError() {
  beep(350, 250);
  beep(250, 300);
}

void melodySoundEvent() {
  beep(900, 100);
  beep(1100, 120);
}

// ---------------- Porta ----------------
void openDoor() {
  doorServo.write(SERVO_OPEN_ANGLE);
}

void closeDoor() {
  doorServo.write(SERVO_CLOSED_ANGLE);
}

// ---------------- Luce principale ----------------
void setMainLight(bool on) {
  mainLightOn = on;
  digitalWrite(MAIN_LED_PIN, on ? HIGH : LOW);
}

// ---------------- Luci scala ----------------
void setScaleLights(bool on) {
  scaleLightsOn = on;
  digitalWrite(STEP_LED1_PIN, on ? HIGH : LOW);
  digitalWrite(STEP_LED2_PIN, on ? HIGH : LOW);
}

void progressiveScaleOn() {
  digitalWrite(STEP_LED1_PIN, HIGH);
  delay(150);
  digitalWrite(STEP_LED2_PIN, HIGH);
  scaleLightsOn = true;
}

// ---------------- LDR ----------------
bool isDark() {
  int value = analogRead(LDR_PIN);
  return value < lightThreshold;
}

// ---------------- PIN ----------------
void resetPINEntry() {
  enteredPIN = "";
  lastKeyPressTime = millis();
  displayMessage("Inserire PIN", "per aprire la porta");
}

void handleKeypad() {
  char key = keypad.getKey();
  if (key == NO_KEY) {
    // Controllo timeout PIN
    if (enteredPIN.length() > 0 && (millis() - lastKeyPressTime > PIN_TIMEOUT)) {
      enteredPIN = "";
      displayMessage("PIN timeout", "Reinserire PIN");
      lastKeyPressTime = millis();
    }
    return;
  }

  lastKeyPressTime = millis();

  if (key >= '0' && key <= '9') {
    if (enteredPIN.length() < 4) {
      enteredPIN += key;

      String masked = "";
      for (unsigned int i = 0; i < enteredPIN.length(); i++) {
        masked += "*";
      }

      displayMessage("Inserire PIN:", masked, "# conferma  * cancella");
    }
  }
  else if (key == '#') {
    if (enteredPIN == correctPIN) {
      unlocked = true;
      displayMessage("Codice corretto", "Accesso consentito");
      melodySuccess();
      openDoor();
      if (isDark()) {
        setMainLight(true);
      }
    } else {
      unlocked = false;
      displayMessage("Codice errato", "Riprova");
      melodyError();
      closeDoor();
      setMainLight(false);
      setScaleLights(false);
    }

    keypadConfirmTime = millis();
  }
  else if (key == '*') {
    enteredPIN = "";
    displayMessage("PIN cancellato", "Reinserire PIN");
    lastKeyPressTime = millis();
  }
}

void handleKeypadDisplay() {
  // Gestione non-bloccante del timeout di conferma PIN
  if (keypadConfirmTime > 0 && (millis() - keypadConfirmTime > KEYPAD_CONFIRM_DURATION)) {
    resetPINEntry();
    keypadConfirmTime = 0;
  }
}

// ---------------- PIR ----------------
void handlePIR() {
  if (!unlocked) {
    setScaleLights(false);
    return;
  }

  if (!isDark()) {
    setScaleLights(false);
    return;
  }

  int pirValue = digitalRead(PIR_PIN);

  if (pirValue == HIGH) {
    if (!scaleLightsOn) {
      progressiveScaleOn();
    }
    scaleLastMotionTime = millis();
  } else {
    if (scaleLightsOn && (millis() - scaleLastMotionTime > SCALE_TIMEOUT)) {
      setScaleLights(false);
    }
  }
}

// ---------------- Sensore suono ----------------
void handleSoundSensor() {
  if (!unlocked) return;

  bool soundDetected = (digitalRead(SOUND_PIN) == HIGH);
  unsigned long now = millis();

  if (soundDetected && !lastSoundState && (now - lastSoundTrigger > SOUND_DEBOUNCE)) {
    lastSoundTrigger = now;
    melodySoundEvent();

    if (isDark()) {
      setMainLight(!mainLightOn);
      if (mainLightOn) {
        displayMessage("Suono rilevato", "Luce principale ON");
      } else {
        displayMessage("Suono rilevato", "Luce principale OFF");
      }
      soundSensorTriggerTime = millis();
    }
  }

  lastSoundState = soundDetected;
}

void handleSoundSensorDisplay() {
  // Gestione non-bloccante della visualizzazione del sensore suono
  if (soundSensorTriggerTime > 0 && (millis() - soundSensorTriggerTime > SOUND_DISPLAY_DURATION)) {
    resetPINEntry();
    soundSensorTriggerTime = 0;
  }
}

// ---------------- Stato luce principale ----------------
void handleEnergySaving() {
  if (!unlocked) return;

  if (!isDark()) {
    setMainLight(false);
    setScaleLights(false);
  }
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(9600);
  Serial.println("Smart Home System - Initialization");

  pinMode(PIR_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(MAIN_LED_PIN, OUTPUT);
  pinMode(STEP_LED1_PIN, OUTPUT);
  pinMode(STEP_LED2_PIN, OUTPUT);

  setMainLight(false);
  setScaleLights(false);

  doorServo.attach(SERVO_PIN);
  closeDoor();

  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    display.clearDisplay();
    display.display();
    Serial.println("OLED initialized successfully");
  } else {
    Serial.println("OLED initialization failed");
  }

  displayMessage("Sistema Smart Home", "Avvio...");
  delay(1500);
  resetPINEntry();
  Serial.println("System ready - waiting for PIN input");
}

// ---------------- Loop ----------------
void loop() {
  handleKeypad();
  handleKeypadDisplay();
  handlePIR();
  handleSoundSensor();
  handleSoundSensorDisplay();
  handleEnergySaving();
}