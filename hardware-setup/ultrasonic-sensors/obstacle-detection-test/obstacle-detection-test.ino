#include "obstacles.h"

const uint8_t HEADER = 0xAA;
const unsigned long BAUD_RATE = 115200;
const long MEASUREMENT_GAP = 15; 

UltrasonicSensor left_sensor(2, 3);
UltrasonicSensor center_sensor(4, 5);
UltrasonicSensor right_sensor(6, 7);

unsigned long previousMillis = 0;
int currentSensorInterval = 0; 

void setup() {
  Serial.begin(BAUD_RATE);
  left_sensor.begin();
  center_sensor.begin();
  right_sensor.begin();
}

void loop() {
  // Perform obstacle detection with given duration
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= MEASUREMENT_GAP) {
    previousMillis = currentMillis;

    if (currentSensorInterval == 0) {
      left_sensor.measure();
      currentSensorInterval = 1;
    } 
    else if (currentSensorInterval == 1) {
      center_sensor.measure();
      currentSensorInterval = 2;
    } 
    else if (currentSensorInterval == 2) {
      right_sensor.measure();
      
      // Send data to
      Serial.print("Left sensor readings: "); 
      Serial.println(left_sensor.getFilteredDistance());
      Serial.print("Center sensor readings: "); 
      Serial.println(center_sensor.getFilteredDistance());
      Serial.print("Right sensor readings: ");
      Serial.println(right_sensor.getFilteredDistance());

      currentSensorInterval = 0;
    }
  }
}