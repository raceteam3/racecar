#include "motor.h"
#include <algorithm>
#include <iostream>

Motor::Motor(boost::shared_ptr<Adafruit_PWMServoDriver> pwm, uint8_t channel, uint16_t maxReverse, uint16_t maxForward) :
  m_Servo(pwm, channel, maxReverse, maxForward),
  m_CurrentSpeed(0)
{
  m_Servo.setDirection(0);
}

void Motor::setSpeed(int speed)
{
  if(speed >= 0 || m_CurrentSpeed < 0) {
    m_Servo.setDirection(speed);
  } else {
    /* Break */
    m_Servo.setDirection(-100);
    usleep(100 * 1000);
    /* Stop motor */
    m_Servo.setDirection(0);
    usleep(100 * 1000);
    /* And finally set requested reverse speed */
    m_Servo.setDirection(speed);
  }
  m_CurrentSpeed = speed;
}

void Motor::breakMotor()
{
  if(m_CurrentSpeed > 5) {
    m_Servo.setDirection(-100);
    m_CurrentSpeed = -100;
  } else {
    setSpeed(0);
  }
}
