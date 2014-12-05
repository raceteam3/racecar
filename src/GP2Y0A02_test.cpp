#include <iostream>
#include <sstream>
#include <unistd.h>
#include <boost/shared_ptr.hpp>
#include "GP2Y0A02.h"

int main(int argc, const char** argv)
{
  int addr = 0;
  int channel = 0;

  if(argc == 3) {
    std::istringstream(argv[1]) >> std::hex >> addr;
    std::istringstream(argv[2]) >>  channel;
  } else {
    std::cout << argv[0] << " addr channel" << std::endl;
    return 1;
  }

  boost::shared_ptr<ADS1115> adc(new ADS1115(addr));
  adc->initialize();

  GP2Y0A02 sensor(adc, channel);

  if(!sensor.initiateRanging()) {
    std::cout << "Ranging failed" << std::endl;
    return 1;
  }

  std::cout << "Waiting for ranging" << std::endl;

  while(!sensor.rangingComplete()) {
  }

  std::cout << "range=" << sensor.getRange() << " cm" << std::endl;

  return 0;
}
