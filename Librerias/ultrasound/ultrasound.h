#ifndef _ULTRASOUND_H
#define _ULTRASOUND_H
#include "Arduino.h"

class Ultrasound
{
private:
  int pin_trig;
  int pin_echo;

public:
  Ultrasound(int trig, int echo);
  float SensorRead();
};

#endif