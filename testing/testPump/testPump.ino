#define PUMP_PIN1 5
#define PUMP_PIN2 6

void setup() {
  pinMode(PUMP_PIN1, OUTPUT);
  pinMode(PUMP_PIN2, OUTPUT);

  // Sicherheit: Startzustand AUS
  digitalWrite(PUMP_PIN1, LOW);
  digitalWrite(PUMP_PIN2, LOW);
}

void loop() {
  // Pumpe EIN (2 Sekunden)
  digitalWrite(PUMP_PIN1, HIGH);
  digitalWrite(PUMP_PIN2, HIGH);
  delay(2000);

  // Pumpe AUS
  digitalWrite(PUMP_PIN1, LOW);
  digitalWrite(PUMP_PIN2, LOW);

  // 5 Sekunden warten
  delay(5000);
}
