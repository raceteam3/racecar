#include "servo.h"
#include <algorithm>
#include <iostream>

Servo::Servo(boost::shared_ptr<Adafruit_PWMServoDriver> pwm, uint8_t channel, uint16_t maxLeft, uint16_t maxRight) :
  m_PWM(pwm),
  m_Channel(channel)
{
  m_PWM->setPWMFreq(60);
  if(maxLeft < maxRight) {
    m_Min = maxLeft;
    m_Max = maxRight;
    m_InvertDirection = false;
  } else {
    m_Min = maxRight;
    m_Max = maxLeft;
    m_InvertDirection = true;
  }
}

void Servo::setDirection(int direction)
{
  if(m_InvertDirection) {
    direction = -direction;
  }
  uint16_t value = std::max<uint16_t>(m_Min, std::min<uint16_t>(m_Max, (((double)(m_Min + m_Max))/200.0)*(direction+100)));
  m_PWM->setPin(m_Channel, value, false);
}
