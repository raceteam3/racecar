#include <iostream>
#include <sstream>
#include <unistd.h>
#include "ADS1115.h"

int main(int argc, const char** argv)
{
  int addr = 0;
  int channel = 0;
  int continuous = (argc == 4);

  if(argc == 3 || argc == 4) {
    std::istringstream(argv[1]) >> std::hex >> addr;
    std::istringstream(argv[2]) >>  channel;
  } else {
    std::cout << argv[0] << " addr channel <cont>" << std::endl;
    return 1;
  }

  ADS1115 adc(addr);
  adc.initialize();

  adc.setGain(ADS1115_PGA_6P144);

  switch(channel) {
  case 0:
    adc.setMultiplexer(ADS1115_MUX_P0_NG);
    break;
  case 1:
    adc.setMultiplexer(ADS1115_MUX_P1_NG);
    break;
  case 2:
    adc.setMultiplexer(ADS1115_MUX_P2_NG);
    break;
  case 3:
    adc.setMultiplexer(ADS1115_MUX_P3_NG);
    break;
  }

  if(continuous) {
    adc.setMode(ADS1115_MODE_CONTINUOUS);
  } else {
      adc.setOpStatus(ADS1115_OS_ACTIVE);
      adc.waitBusy(1000);
  }
  do {
    std::cout << "ADC channel " << channel << ": " << adc.getMilliVolts() << " mV" << std::endl;
    usleep(100*1000);
  } while(continuous);

  return 0;
}
