#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include "servo.h"
#include "Adafruit_PWMServoDriver.h"

#include <boost/shared_ptr.hpp>

class Motor
{
 public:
  Motor(boost::shared_ptr<Adafruit_PWMServoDriver> pwm, uint8_t channel, uint16_t maxReverse, uint16_t maxForward);

  void setSpeed(int speed);
  void breakMotor();

 private:
  Servo m_Servo;
  int m_CurrentSpeed;
};
#endif
