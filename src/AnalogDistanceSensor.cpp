#include "AnalogDistanceSensor.h"


AnalogDistanceSensor::AnalogDistanceSensor(boost::shared_ptr<ADS1115> adc, uint8_t channel) : m_Adc(adc), m_Channel(channel)
{
  switch(channel) {
  case 0:
    m_Channel = ADS1115_MUX_P0_NG;
    break;
  case 1:
    m_Channel = ADS1115_MUX_P1_NG;
    break;
  case 2:
    m_Channel = ADS1115_MUX_P2_NG;
    break;
  case 3:
    m_Channel = ADS1115_MUX_P3_NG;
    break;
  }
}

AnalogDistanceSensor::~AnalogDistanceSensor()
{
}


bool AnalogDistanceSensor::initiateRanging()
{
  m_Adc->setMultiplexer(m_Channel);
  m_Adc->setMode(ADS1115_MODE_SINGLESHOT);
  setupRanging();
  m_Adc->setOpStatus(ADS1115_OS_ACTIVE);
  return true;
}


bool AnalogDistanceSensor::rangingComplete()
{
  if(m_Adc->getMultiplexer() != m_Channel) {
    return false;
  }
  return (m_Adc->getOpStatus()==ADS1115_OS_INACTIVE);
}

uint16_t AnalogDistanceSensor::getRange()
{
  if(m_Adc->getMultiplexer() != m_Channel) {
    return 0;
  }
  return voltageToRange(m_Adc->getMilliVolts());
}
