#ifndef ANALOG_DISTANCE_SENSOR_H
#define ANALOG_DISTANCE_SENSOR_H

#include <stdint.h>
#include "ADS1115.h"

#include <boost/shared_ptr.hpp>

class AnalogDistanceSensor
{
 public:
  AnalogDistanceSensor(boost::shared_ptr<ADS1115> adc, uint8_t channel);
  virtual ~AnalogDistanceSensor();

  bool initiateRanging();
  bool rangingComplete();

  uint16_t getRange();

private:
  virtual void setupRanging() = 0;
  virtual uint16_t voltageToRange(float millivolts) = 0;

protected:
  boost::shared_ptr<ADS1115> m_Adc;

 private:
  int m_Channel;
};
#endif
