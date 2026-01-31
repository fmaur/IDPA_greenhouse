#include <DFRobot_DHT11.h>

DFRobot_DHT11 DHT;

/* =======================
   PIN DEFINITIONS
   ======================= */
#define MOISTURE_PIN A2
#define DHT11_PIN    9

#define PUMP_PIN1    5
#define PUMP_PIN2    6

/* =======================
   SYSTEM CONSTANTS
   ======================= */
#define MEASUREMENT_COUNT      5
#define MEASUREMENT_INTERVAL   20000UL   // 20 seconds
#define SLEEP_INTERVAL         300000UL  // 5 minutes

#define PUMP_STAGE_DURATION    2000UL    // 2 seconds
#define INFILTRATION_DELAY     600000UL  // 10 minutes

/* =======================
   MOISTURE THRESHOLDS
   ======================= */
#define MOISTURE_LOW   400
#define MOISTURE_HIGH  500

/* =======================
   STATE VARIABLES
   ======================= */
bool previousCycleDry = false;

/* =======================
   SETUP
   ======================= */
void setup() {
  Serial.begin(9600);

  pinMode(PUMP_PIN1, OUTPUT);
  pinMode(PUMP_PIN2, OUTPUT);

  digitalWrite(PUMP_PIN1, LOW);
  digitalWrite(PUMP_PIN2, LOW);

  Serial.println("[SYSTEM] Smart irrigation controller started");
}

/* =======================
   MAIN LOOP
   ======================= */
void loop() {

  Serial.println("\n[SYSTEM] New measurement cycle started");

  int soilValues[MEASUREMENT_COUNT];
  int airHumidityValues[MEASUREMENT_COUNT];
  int airTemperatureValues[MEASUREMENT_COUNT];

  /* ----- Measurement Phase ----- */
  for (int i = 0; i < MEASUREMENT_COUNT; i++) {
    Serial.print("[MEASURE] Reading ");
    Serial.print(i + 1);
    Serial.println(" / 5");

    DHT.read(DHT11_PIN);

    airHumidityValues[i] = DHT.humidity;
    airTemperatureValues[i] = DHT.temperature;
    soilValues[i] = analogRead(MOISTURE_PIN);

    Serial.print("  Air humidity: ");
    Serial.print(airHumidityValues[i]);
    Serial.print(" %, Temp: ");
    Serial.print(airTemperatureValues[i]);
    Serial.print(" °C, Soil: ");
    Serial.println(soilValues[i]);

    delay(MEASUREMENT_INTERVAL);
  }

  /* ----- Filtering Phase ----- */
  Serial.println("[FILTER] Applying trimmed mean filter");

  int soilMoisture = trimmedMean(soilValues);
  int airHumidity = trimmedMean(airHumidityValues);
  int airTemperature = trimmedMean(airTemperatureValues);

  Serial.print("[FILTER] Soil moisture (filtered): ");
  Serial.println(soilMoisture);
  Serial.print("[FILTER] Air humidity (filtered): ");
  Serial.println(airHumidity);
  Serial.print("[FILTER] Temperature (filtered): ");
  Serial.println(airTemperature);

  /* ----- Decision Logic ----- */
  bool currentCycleDry = (soilMoisture < MOISTURE_LOW);

  Serial.print("[LOGIC] Moisture LOW threshold: ");
  Serial.println(MOISTURE_LOW);
  Serial.print("[LOGIC] Current cycle dry: ");
  Serial.println(currentCycleDry ? "YES" : "NO");
  Serial.print("[LOGIC] Previous cycle dry: ");
  Serial.println(previousCycleDry ? "YES" : "NO");

  if (currentCycleDry && previousCycleDry) {
    Serial.println("[LOGIC] Two consecutive dry cycles → irrigation allowed");
    irrigationProcess();
  } else {
    Serial.println("[LOGIC] Irrigation blocked (confirmation or moisture missing)");
  }

  previousCycleDry = currentCycleDry;

  /* ----- Sleep Phase ----- */
  Serial.println("[SYSTEM] Entering sleep mode (5 minutes)");
  delay(SLEEP_INTERVAL);
}

/* =======================
   IRRIGATION PROCESS
   ======================= */
void irrigationProcess() {

  Serial.println("[IRRIGATION] Stage 1 started");
  activatePump();

  Serial.println("[IRRIGATION] Infiltration delay started");
  delay(INFILTRATION_DELAY);

  Serial.println("[IRRIGATION] Re-measuring soil moisture");

  int soilValues[MEASUREMENT_COUNT];

  for (int i = 0; i < MEASUREMENT_COUNT; i++) {
    soilValues[i] = analogRead(MOISTURE_PIN);

    Serial.print("  Soil re-measure ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(soilValues[i]);

    delay(MEASUREMENT_INTERVAL);
  }

  int soilMoisture = trimmedMean(soilValues);

  Serial.print("[IRRIGATION] Filtered soil moisture after stage 1: ");
  Serial.println(soilMoisture);
  Serial.print("[IRRIGATION] Moisture HIGH threshold: ");
  Serial.println(MOISTURE_HIGH);

  if (soilMoisture < MOISTURE_HIGH) {
    Serial.println("[IRRIGATION] Moisture still low → Stage 2 activated");
    activatePump();
  } else {
    Serial.println("[IRRIGATION] Moisture sufficient → Stage 2 skipped");
  }
}

/* =======================
   PUMP CONTROL
   ======================= */
void activatePump() {
  Serial.println("[PUMP] Pump ON");
  digitalWrite(PUMP_PIN1, HIGH);
  digitalWrite(PUMP_PIN2, HIGH);

  delay(PUMP_STAGE_DURATION);

  digitalWrite(PUMP_PIN1, LOW);
  digitalWrite(PUMP_PIN2, LOW);
  Serial.println("[PUMP] Pump OFF");
}

/* =======================
   TRIMMED MEAN FUNCTION
   ======================= */
int trimmedMean(int values[]) {
  int minVal = values[0];
  int maxVal = values[0];
  long sum = 0;

  for (int i = 0; i < MEASUREMENT_COUNT; i++) {
    if (values[i] < minVal) minVal = values[i];
    if (values[i] > maxVal) maxVal = values[i];
    sum += values[i];
  }

  Serial.print("[FILTER] Raw sum: ");
  Serial.println(sum);
  Serial.print("[FILTER] Min removed: ");
  Serial.println(minVal);
  Serial.print("[FILTER] Max removed: ");
  Serial.println(maxVal);

  sum -= minVal;
  sum -= maxVal;

  int result = sum / (MEASUREMENT_COUNT - 2);

  Serial.print("[FILTER] Trimmed mean result: ");
  Serial.println(result);

  return result;
}
