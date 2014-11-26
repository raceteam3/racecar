#include "srf08.h"

#include <unistd.h>
#include <wiringPiI2C.h>

#define CHECK_RETURN(x) if((x) == -1) { return false; }

srf08::srf08(uint8_t addr)
{
  m_Fd = wiringPiI2CSetup(addr);
}

bool srf08::initiateRanging()
{
  if(wiringPiI2CWriteReg8(m_Fd, 0, 0x51)) {
    return false;
  }
  return true;
}


bool srf08::rangingComplete()
{
  return (wiringPiI2CReadReg8(m_Fd, 0) != -1);
}

uint8_t srf08::getLightLevel()
{
  int val = wiringPiI2CReadReg8(m_Fd, 1);
  return (val == -1) ? 0 : val;
}

uint16_t srf08::getRange()
{
  uint8_t msb = wiringPiI2CReadReg8(m_Fd, 2);
  uint8_t lsb = wiringPiI2CReadReg8(m_Fd, 3);
  return (msb << 8) | lsb;
}

bool srf08::changeAddress(uint8_t addr)
{
  CHECK_RETURN(wiringPiI2CWriteReg8(m_Fd, 0, 0xA0));
  CHECK_RETURN(wiringPiI2CWriteReg8(m_Fd, 0, 0xAA));
  CHECK_RETURN(wiringPiI2CWriteReg8(m_Fd, 0, 0xA5));
  CHECK_RETURN(wiringPiI2CWriteReg8(m_Fd, 0, addr));

  close(m_Fd);
  m_Fd = wiringPiI2CSetup(addr/2);
  return true;
}
