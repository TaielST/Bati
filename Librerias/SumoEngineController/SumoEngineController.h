#ifndef _SUMO_ENGINE_CONTROLLER_H
#define _SUMO_ENGINE_CONTROLLER_H
#include "Arduino.h"

class Engine
{
private:
  int speed;
  int pinDirection;
  int pinPwm;
  int disabled=true;

public:
  Engine(int dir, int pwm);
  void Forward(int speed);
  void Backward(int speed);
  void Stop();
  void Disabled();
};

class EngineController
{
private:
  int pinDirectionRight;
  int pinPwmRihgt;
  int pinDirectionLeft;
  int pinPwmLeft;
  int speedRight;
  int speedLeft;
  Engine *engineRight;
  Engine *engineLeft;
  

public:
  EngineController(int dirRight, int pwmRight, int dirLeft, int pwmLeft);
  void Forward(int pwmRight, int pwmLeft);
  void Backward(int pwmRight, int pwmLeft);
  void Right(int pwmRight, int pwmLeft);
  void Left(int pwmRight, int pwmLeft);
  void Stop();
  void DisabledMotorRight();
  void DisabledMotorLeft();
};

#endif