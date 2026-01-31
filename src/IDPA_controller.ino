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
}

/* =======================
   MAIN LOOP
   ======================= */
void loop() {

  int soilValues[MEASUREMENT_COUNT];
  int airHumidityValues[MEASUREMENT_COUNT];
  int airTemperatureValues[MEASUREMENT_COUNT];

  /* ----- Measurement Phase ----- */
  for (int i = 0; i < MEASUREMENT_COUNT; i++) {
    DHT.read(DHT11_PIN);

    airHumidityValues[i] = DHT.humidity;
    airTemperatureValues[i] = DHT.temperature;
    soilValues[i] = analogRead(MOISTURE_PIN);

    delay(MEASUREMENT_INTERVAL);
  }

  /* ----- Filtering Phase ----- */
  int soilMoisture = trimmedMean(soilValues);
  int airHumidity = trimmedMean(airHumidityValues);
  int airTemperature = trimmedMean(airTemperatureValues);

  Serial.print("Soil:");
  Serial.print(soilMoisture);
  Serial.print(" | AirHum:");
  Serial.print(airHumidity);
  Serial.print(" | Temp:");
  Serial.println(airTemperature);

  /* ----- Decision Logic ----- */
  bool currentCycleDry = (soilMoisture < MOISTURE_LOW);

  if (currentCycleDry && previousCycleDry) {
    irrigationProcess();
  }

  previousCycleDry = currentCycleDry;

  /* ----- Sleep Phase ----- */
  delay(SLEEP_INTERVAL);
}

/* =======================
   IRRIGATION PROCESS
   ======================= */
void irrigationProcess() {

  // Stage 1
  activatePump();

  // Infiltration delay
  delay(INFILTRATION_DELAY);

  // Re-measure soil moisture
  int soilValues[MEASUREMENT_COUNT];

  for (int i = 0; i < MEASUREMENT_COUNT; i++) {
    soilValues[i] = analogRead(MOISTURE_PIN);
    delay(MEASUREMENT_INTERVAL);
  }

  int soilMoisture = trimmedMean(soilValues);

  if (soilMoisture < MOISTURE_HIGH) {
    activatePump();  // Stage 2
  }
}

/* =======================
   PUMP CONTROL
   ======================= */
void activatePump() {
  digitalWrite(PUMP_PIN1, HIGH);
  digitalWrite(PUMP_PIN2, HIGH);
  delay(PUMP_STAGE_DURATION);
  digitalWrite(PUMP_PIN1, LOW);
  digitalWrite(PUMP_PIN2, LOW);
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

  sum -= minVal;
  sum -= maxVal;

  return sum / (MEASUREMENT_COUNT - 2);
}