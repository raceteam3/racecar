#ifndef ROBOT_H
#define ROBOT_H

#include "Adafruit_PWMServoDriver.h"
#include "servo.h"
#include "motor.h"
#include "srf08.h"
#include "ADS1115.h"

#include <stdint.h>
#include <boost/shared_ptr.hpp>
#include <map>
#include <string>

class Robot
{
 public:
  Robot();
  ~Robot();

  void initialize(const char* cfg);
  void run();
  void runManual();

 private:
  boost::shared_ptr<Servo> m_Steering;
  boost::shared_ptr<Motor> m_Motor;
  std::map<std::string, boost::shared_ptr<Adafruit_PWMServoDriver> > m_PWMDrivers;
  std::map<int, boost::shared_ptr<srf08> > m_SRF08Sensors;
  std::map<std::string, boost::shared_ptr<ADS1115> > m_ADS1115ADCs;
};
#endif
