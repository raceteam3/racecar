#include <iostream>
#include <sstream>
#include "Adafruit_PWMServoDriver.h"

int main(int argc, const char** argv)
{
  if(argc != 4) {
    std::cout << argv[0] << " addr channel pulse" << std::endl;
    return 1;
  }

  int addr = 0;
  int channel = 0;
  int pulse = 0;
  std::istringstream(argv[1]) >> std::hex >> addr;
  std::istringstream(argv[2]) >> channel;
  std::istringstream(argv[3]) >> pulse;

  std::cout << "Create servo driver" << std::endl;
  Adafruit_PWMServoDriver pwm(addr);
  pwm.reset();

  std::cout << "Set frequency to 60 Hz" << std::endl;
  pwm.setPWMFreq(60);

  std::cout << "Set pulse length to " << pulse << std::endl;
  pwm.setPin(channel, pulse, false);

  return 0;
}
