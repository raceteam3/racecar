#include "MouseSpeedSensor.h"

#include <iostream>
#include <fcntl.h>
#include <unistd.h>

MouseSpeedSensor::MouseSpeedSensor(): m_Fd(-1)
{
}

MouseSpeedSensor::~MouseSpeedSensor()
{
  if(m_Fd != -1) {
    close(m_Fd);
  }
}

bool MouseSpeedSensor::initialize(const char* device)
{
  m_Fd = open(device, O_RDWR | O_NONBLOCK);
  return (m_Fd != -1);
}

MouseSpeedSensor::MouseSpeed MouseSpeedSensor::getSpeed()
{
  MouseSpeed speed = {0,0};
  int8_t data[3];
  if(read(m_Fd, data, sizeof(data)) == sizeof(data)) {
    speed.x = data[1];
    speed.y = data[2];
  }
  return speed;
}
