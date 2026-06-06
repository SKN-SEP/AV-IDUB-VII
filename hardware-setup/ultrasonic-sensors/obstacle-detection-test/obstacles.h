#pragma once

const int MAX_DISTANCE = 100;

class UltrasonicSensor {
private: 
  int trig_pin;
  int echo_pin;
  long readings[3];
  long lastValidDist;

  long getMedianOfThreeVariables(long a, long b, long c);

public:
  UltrasonicSensor(int trig_pin, int echo_pin);
  void begin();
  void measure();
  long getFilteredDistance();
};