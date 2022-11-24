#include "Ultrasound.h"

Ultrasound::Ultrasound(int trig, int echo)
{
    pin_echo = echo;
    pin_trig = trig;

    pinMode(pin_echo, INPUT);
    pinMode(pin_trig, OUTPUT);
    digitalWrite(pin_trig, LOW);
}
// metodos
float Ultrasound::SensorRead()
{
    float distance;
    float ultrasoundPulse;
    // SENSOR
    digitalWrite(pin_trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(pin_trig, LOW);
    ultrasoundPulse = pulseIn(pin_echo, HIGH);
    distance = ultrasoundPulse / 58.2;
    return distance;
}