#ifndef MOUSE_SPEED_SENSOR_H
#define MOUSE_SPEED_SENSOR_H

#include <stdint.h>

class MouseSpeedSensor
{
 public:
  MouseSpeedSensor();
  ~MouseSpeedSensor();
  bool initialize(const char* device);

  struct MouseSpeed
  {
    int x;
    int y;
  };

  MouseSpeed getSpeed();

 private:
  int m_Fd;
};
#endif
