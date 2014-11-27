#ifndef ROBOT_H
#define ROBOT_H

#include <stdint.h>
#include <vector>
#include "servo.h"
#include "srf08.h"

class Robot
{
 public:
  Robot();
  ~Robot();

  void initialize(const char* cfg);
  void run();

 private:
  Servo* m_Servo;
  std::vector<srf08> m_UltraSonicSensors;
};
#endif
