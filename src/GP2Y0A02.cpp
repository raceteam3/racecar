#include "GP2Y0A02.h"
#include <math.h>

GP2Y0A02::GP2Y0A02(boost::shared_ptr<ADS1115> adc, uint8_t channel) : AnalogDistanceSensor(adc, channel)
{
}

void GP2Y0A02::setupRanging()
{
    m_Adc->setGain(ADS1115_PGA_4P096);
}

uint16_t GP2Y0A02::voltageToRange(float millivolts)
{
  return 65*pow(((double)millivolts)/1000.0, -1.10);
}
