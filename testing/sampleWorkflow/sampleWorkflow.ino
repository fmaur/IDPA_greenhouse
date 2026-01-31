#include <DFRobot_DHT11.h>

DFRobot_DHT11 DHT;

// Pins (EcoDuino)
#define MOISTURE_PIN A2
#define DHT11_PIN    9
#define PUMP_PIN1    5
#define PUMP_PIN2    6

// --- Einstellungen (anpassbar) ---
const int   SOIL_DRY_THRESHOLD = 750;   // Rohwert 0..1023: ab hier gilt "zu trocken" (kalibrieren!)
const int   SOIL_WET_THRESHOLD = 850;   // oberhalb davon wird sicher NICHT gepumpt (Hysterese)
const int   SOIL_SAMPLES       = 10;    // Anzahl Messungen für Mittelwert
const int   SOIL_SAMPLE_DELAY  = 50;    // ms zwischen Messungen

const unsigned long PUMP_ON_MS       = 2000;   // "geringe Wassermenge": 2 Sekunden
const unsigned long COOLDOWN_MS      = 60000;  // 60s Sperrzeit nach Pumpen (Schutz vor Dauerpumpen)
const unsigned long LOOP_DELAY_MS    = 2000;   // Hauptloop-Geschwindigkeit

// Klima-Logik (optional, aber sinnvoll)
const int HOT_TEMP_C          = 28;     // ab dieser Temp mehr Bedarf
const int DRY_AIR_HUMIDITY    = 35;     // unter dieser Luftfeuchte mehr Bedarf

// --- interne Variablen ---
unsigned long lastPumpTime = 0;
bool lastStateWasDry = false; // für Hysterese

// Mittelwert Soil
int readSoilAverage() {
  long sum = 0;
  for (int i = 0; i < SOIL_SAMPLES; i++) {
    sum += analogRead(MOISTURE_PIN);
    delay(SOIL_SAMPLE_DELAY);
  }
  return (int)(sum / SOIL_SAMPLES);
}

// Pumpe steuern
void pumpOn() {
  digitalWrite(PUMP_PIN1, HIGH);
  digitalWrite(PUMP_PIN2, HIGH);
}
void pumpOff() {
  digitalWrite(PUMP_PIN1, LOW);
  digitalWrite(PUMP_PIN2, LOW);
}

// Entscheidung: soll gepumpt werden?
bool shouldPump(int soilAvg, int tempC, int airHum) {
  // Cooldown: nach Pumpen erstmal warten
  if (millis() - lastPumpTime < COOLDOWN_MS) return false;

  // Hysterese: verhindert "Flattern" um einen einzigen Schwellwert
  // - wenn vorher nicht trocken: erst pumpen, wenn unter DRY_THRESHOLD
  // - wenn vorher trocken: erst wieder "okay", wenn über WET_THRESHOLD
  if (!lastStateWasDry) {
    if (soilAvg >= SOIL_DRY_THRESHOLD) return false;
  } else {
    if (soilAvg >= SOIL_WET_THRESHOLD) {
      // wieder feucht genug -> Zustand zurücksetzen
      return false;
    }
  }

  // Optional: Klima als Verstärker (bei Hitze oder trockener Luft eher pumpen)
  bool climateSuggestsMoreWater = (tempC >= HOT_TEMP_C) || (airHum > 0 && airHum <= DRY_AIR_HUMIDITY);

  // Boden ist trocken -> grundsätzlich pumpen.
  // Wenn Klima extrem ist, pumpen wir trotzdem nur 2s (kleine Menge), aber wir erlauben es.
  // (Du kannst hier später z.B. PUMP_ON_MS leicht erhöhen.)
  (void)climateSuggestsMoreWater; // aktuell nur informativ
  return true;
}

void setup() {
  Serial.begin(9600);

  pinMode(PUMP_PIN1, OUTPUT);
  pinMode(PUMP_PIN2, OUTPUT);

  // Sicherheit: Start AUS
  pumpOff();
}

void loop() {
  // --- Sensoren lesen ---
  int soilAvg = readSoilAverage();

  DHT.read(DHT11_PIN);
  int airHum  = DHT.humidity;     // 0..100 (bei Fehlern manchmal 0)
  int tempC   = DHT.temperature;  // °C

  // --- Diagnoseausgabe ---
  Serial.print("soilAvg:");
  Serial.print(soilAvg);
  Serial.print("\tairHumidity:");
  Serial.print(airHum);
  Serial.print("\tairTemp:");
  Serial.print(tempC);

  // --- Entscheidung ---
  bool pump = shouldPump(soilAvg, tempC, airHum);

  if (pump) {
    Serial.print("\t-> PUMP 2s");
    pumpOn();
    delay(PUMP_ON_MS);
    pumpOff();

    lastPumpTime = millis();
    lastStateWasDry = true; // wir waren trocken und haben reagiert
  } else {
    Serial.print("\t-> no pump");
    // Zustand trocken/nicht trocken anhand Hysterese updaten:
    // Wenn deutlich feucht, setzen wir zurück
    if (soilAvg >= SOIL_WET_THRESHOLD) lastStateWasDry = false;
  }

  Serial.println();
  delay(LOOP_DELAY_MS);
}
