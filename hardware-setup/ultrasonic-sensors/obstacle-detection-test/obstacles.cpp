#include "obstacles.h"
#include <Arduino.h>

long UltrasonicSensor::getMedianOfThreeVariables(long a, long b, long c) {
  if ((a <= b && b <= c) || (c <= b && b <= a)) return b;
  if ((b <= a && a <= c) || (c <= a && a <= b)) return a;
  return c;
}

UltrasonicSensor::UltrasonicSensor(int trig_pin, int echo_pin) {
  this->trig_pin = trig_pin;
  this->echo_pin = echo_pin;

  this->readings[0] = this->readings[1] = this->readings[2] = MAX_DISTANCE;
  this->lastValidDist = MAX_DISTANCE;
}

void UltrasonicSensor::begin() {
  pinMode(this->trig_pin, OUTPUT);
  pinMode(this->echo_pin, INPUT);
}

void UltrasonicSensor::measure() {
  digitalWrite(this->trig_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(this->trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(this->trig_pin, LOW);

  // Timeout 10000 µs 
  long duration = pulseIn(this->echo_pin, HIGH, 10000); 
  long distance = duration * 0.034 / 2; 

  if (distance <= 0 || distance > 100)
    distance = MAX_DISTANCE;
  else
    this->lastValidDist = distance;

  // Save measured distance
  this->readings[2] = this->readings[1];
  this->readings[1] = this->readings[0];
  this->readings[0] = distance;
}

long UltrasonicSensor::getFilteredDistance() {
  return this->getMedianOfThreeVariables(this->readings[0], this->readings[1], this->readings[2]);
}