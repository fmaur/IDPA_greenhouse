#include <DFRobot_DHT11.h>

DFRobot_DHT11 DHT;

#define MOISTURE_PIN A2
#define DHT11_PIN 9

void setup() {
  Serial.begin(9600);
}

void loop() {
  // Read DHT11 (no return value in this library)
  DHT.read(DHT11_PIN);

  int airHumidity = DHT.humidity;
  int airTemperature = DHT.temperature;
  int soilHumidity = analogRead(MOISTURE_PIN);

  Serial.print("airHumidity:");
  Serial.print(airHumidity);
  Serial.print(",\t");

  Serial.print("airTemperature:");
  Serial.print(airTemperature);
  Serial.print(",\t");

  Serial.print("soilHumidity:");
  Serial.println(soilHumidity);

  delay(1000);
}
