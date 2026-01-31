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
