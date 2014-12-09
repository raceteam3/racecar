#include "MouseSpeedSensor.h"

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
  uint8_t data[3];
  if(read(m_Fd, data, sizeof(data)) == sizeof(data)) {
    speed.x = data[1];
    if(data[0] & (1<<6)) {
      speed.x = 1000;
    }
    if(data[0] & (1<<4)) {
      speed.x = -speed.x;
    }
    speed.y = data[2];
    if(data[0] & (1<<7)) {
      speed.y = 1000;
    }
    if(data[0] & (1<<5)) {
      speed.y = -speed.y;
    }
  }
  return speed;
}
