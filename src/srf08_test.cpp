#include <iostream>
#include <sstream>
#include "srf08.h"

int main(int argc, const char** argv)
{
  int addr = 0xE0;
  int newAddr = 0xE0;

  if(argc == 2 || argc == 3) {
    std::istringstream(argv[1]) >> std::hex >> addr;
    if(argc == 3) {
      std::istringstream(argv[2]) >> std::hex >> newAddr;
    }
  } else {
    std::cout << argv[1] << " addr [newAddr]" << std::endl;
    return 1;
  }

  srf08 sensor(addr/2);
  if(argc == 3) {
    std::cout << "Change address to 0x" << std::hex << newAddr << std::dec << std::endl;
    if(!sensor.changeAddress(newAddr)) {
      std::cout << "Failed" << std::endl;
      return 1;
    }
  }
  if(!sensor.initiateRanging()) {
    std::cout << "Ranging failed" << std::endl;
    return 1;
  }

  std::cout << "Waiting for ranging" << std::endl;

  while(!sensor.rangingComplete()) {
  }

  std::cout << "light=" << (int)sensor.getLightLevel() << ", range=" << sensor.getRange() << " cm" << std::endl;
}
