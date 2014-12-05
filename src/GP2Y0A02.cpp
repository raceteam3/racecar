#include "GP2Y0A02.h"

GP2Y0A02::GP2Y0A02(boost::shared_ptr<ADS1115> adc, uint8_t channel) : AnalogDistanceSensor(adc, channel)
{
}

void GP2Y0A02::setupRanging()
{
    m_Adc->setGain(ADS1115_PGA_4P096);
}

uint16_t GP2Y0A02::voltageToRange(int millivolts)
{
    /* TODO: Calculate distance */
    return 0;
}
