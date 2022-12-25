#define NH3_MUX_PIN 0
#define ANALOG_PIN A0
#define SERVO_PIN D10

#define BLYNK_TEMPLATE_ID "TMPLYydOvcAY"
#define BLYNK_DEVICE_NAME "Lobster IoT"
#define BLYNK_AUTH_TOKEN "Z-9uxvyoA_hwKkX7txIpwpCTmyOV_p8X"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>

const int MUX_SELECT_PINS[3] = {D5, D6, D7};

const char ssid[] = "Canary99";
const char password[] = "otokotok";
const char auth[] = "Z-9uxvyoA_hwKkX7txIpwpCTmyOV_p8X";

BlynkTimer timer;
Servo servo;

void setup() {
  pinMode(ANALOG_PIN, INPUT);
  for(int i = 0; i < 3; i++){
    pinMode(MUX_SELECT_PINS[i], OUTPUT);
  }
  servo.attach(SERVO_PIN);
  servo.write(0);
  
  Serial.begin(9600);

  Blynk.begin(auth, ssid, password);
  timer.setInterval(1000L, blynkTimerEvent);

  preheatNH3(5);
}

void loop() {
  Blynk.run();
  timer.run();
}

void blynkTimerEvent() {
  readAllMux();
  float nh3_val = readNH3();
  Blynk.virtualWrite(V0, nh3_val);
}

// Feeder Virtual Pin 01
BLYNK_WRITE(V1) {
  if (param.asInt() == 1) {
    Serial.println("[Feeder] Received feeder activation signal!");
    Serial.println("[Feeder] Started feeding!");
    startFeeder();
    delay(1000);
    Serial.println("[Feeder] Stopped feeding!");
  } else {
    Serial.println("[Feeder] Received feeder deactivation signal!");
  }
}

// Multiplexer
void selectMuxPin(byte pin)
{
  if (pin > 7) return; // Exit if pin is out of scope
  for (int i=0; i<3; i++)
  {
    if (pin & (1<<i))
      digitalWrite(MUX_SELECT_PINS[i], HIGH);
    else
      digitalWrite(MUX_SELECT_PINS[i], LOW);
  }
}

void startFeeder(){
  for(int i = 0; i < 5; i++){
    servo.write(120);
    delay(500);
    servo.write(0);
    delay(500);
  }
}

void readAllMux(){
  Serial.print("[MUX] ");
  for (byte pin=0; pin<=7; pin++)
  {
    selectMuxPin(pin); // Select one at a time
    int inputValue = analogRead(ANALOG_PIN); // and read Z
    Serial.print(String(inputValue) + "\t");
  }
  Serial.println();
}

float readNH3() {
  selectMuxPin(NH3_MUX_PIN);

  const float RO = 37;
  const float RL = 10;
  const float m = -0.417;
  const float b = 0.858;

  float VRL = analogRead(ANALOG_PIN) * (5 / 1023.0);
  float RS = (5.0 / VRL - 1) * 10;
  float ratio = RS / RO;
  float ppm = pow(10, (log10(ratio) - b) / m);

  Serial.print("[NH3] Reading NH3 value of ");
  Serial.print(ppm);
  Serial.println(" PPM");

  return ppm;
}

void preheatNH3(int seconds) {
  for (int i = 1; i <= seconds; i++) {
    Serial.print("[NH3] Preheating for ");
    Serial.print(i);
    Serial.println(" seconds");
    delay(1000);
  }
}
