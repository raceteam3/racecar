#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>
#include "Adafruit_PWMServoDriver.h"

#include <boost/shared_ptr.hpp>

class Servo
{
 public:
  Servo(boost::shared_ptr<Adafruit_PWMServoDriver> pwm, uint8_t channel, uint16_t maxLeft, uint16_t maxRight);

  void setDirection(int direction);

 private:
  boost::shared_ptr<Adafruit_PWMServoDriver> m_PWM;
  uint8_t m_Channel;
  uint16_t m_Min;
  uint16_t m_Max;
  bool m_InvertDirection;
};
#endif
