#ifndef SRF08_H
#define SRF08_H

#include <stdint.h>

class srf08
{
 public:
  srf08(uint8_t addr);

  bool initiateRanging();
  bool rangingComplete();

  uint8_t getLightLevel();
  uint16_t getRange();

  bool changeAddress(uint8_t addr);

 private:
  int m_Fd;
};
#endif
