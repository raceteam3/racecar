#ifndef ROBOT_H
#define ROBOT_H

#include "servo.h"
#include "srf08.h"

#include <stdint.h>
#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>
#include <string>

class Robot
{
 public:
  Robot();
  ~Robot();

  void initialize(const char* cfg);
  void run();

 private:
  Servo* m_Steering;
  Servo* m_Motor;
  std::map<std::string, boost::shared_ptr<Adafruit_PWMServoDriver> > m_PWMDrivers;
  std::vector<srf08> m_UltraSonicSensors;
};
#endif
