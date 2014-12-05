#ifndef GP2Y0A02_H
#define GP2Y0A02_H

#include <stdint.h>
#include "analog_distance_sensor.h"

class GP2Y0A02: public AnalogDistanceSensor
{
 public:

  GP2Y0A02(boost::shared_ptr<ADS1115> adc, uint8_t channel);

  virtual void setupRanging();
  virtual uint16_t voltageToRange(int millivolts);
};
#endif
