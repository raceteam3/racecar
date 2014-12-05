#include "analog_distance_sensor.h"


AnalogDistanceSensor::AnalogDistanceSensor(boost::shared_ptr<ADS1115> adc, uint8_t channel) : m_Adc(adc), m_Channel(channel)
{
}

AnalogDistanceSensor::~AnalogDistanceSensor()
{
}


bool AnalogDistanceSensor::initiateRanging()
{
  switch(m_Channel) {
  case 0:
    m_Adc->setMultiplexer(ADS1115_MUX_P0_NG);
    break;
  case 1:
    m_Adc->setMultiplexer(ADS1115_MUX_P1_NG);
    break;
  case 2:
    m_Adc->setMultiplexer(ADS1115_MUX_P2_NG);
    break;
  case 3:
    m_Adc->setMultiplexer(ADS1115_MUX_P3_NG);
    break;
  }
  m_Adc->setMode(ADS1115_MODE_SINGLESHOT);
  setupRanging();
  m_Adc->setOpStatus(ADS1115_OS_ACTIVE);
  return true;
}


bool AnalogDistanceSensor::rangingComplete()
{
  return (m_Adc->getOpStatus()==ADS1115_OS_INACTIVE);
}

uint16_t AnalogDistanceSensor::getRange()
{
  return voltageToRange(m_Adc->getMilliVolts());
}
