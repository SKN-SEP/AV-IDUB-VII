#include "obstacles.h"

const uint8_t HEADER = 0xAA;
const unsigned long BAUD_RATE = 115200;
const long MEASUREMENT_GAP = 15; 

UltrasonicSensor left_sensor(2, 3);
UltrasonicSensor center_sensor(4, 5);
UltrasonicSensor right_sensor(6, 7);

unsigned long previousMillis = 0;
int currentSensorInterval = 0; 

void sendPacket() {
  uint8_t left_reading = (uint8_t) left_sensor.getFilteredDistance();
  uint8_t center_reading = (uint8_t) center_sensor.getFilteredDistance();
  uint8_t right_reading = (uint8_t) right_sensor.getFilteredDistance();
  uint8_t crc = HEADER ^ left_reading ^ center_reading ^ right_reading;

  Serial.write(HEADER);
  Serial.write(left_reading);
  Serial.write(center_reading);
  Serial.write(right_reading);
  Serial.write(crc);
}

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

      sendPacket();
      currentSensorInterval = 0;
    }
  }
}