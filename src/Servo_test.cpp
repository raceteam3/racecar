#include <iostream>
#include <sstream>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include "Servo.h"

int main(int argc, const char** argv)
{
  if(argc != 6) {
    std::cout << argv[0] << " addr channel maxLeft maxRight direction" << std::endl;
    return 1;
  }

  int addr = 0;
  int channel;
  int direction = 0;
  int maxLeft = 0;
  int maxRight = 0;
  std::istringstream(argv[1]) >> std::hex >> addr;
  std::istringstream(argv[2]) >> channel;
  std::istringstream(argv[3]) >> maxLeft;
  std::istringstream(argv[4]) >> maxRight;
  std::istringstream(argv[5]) >> direction;

  std::cout << "Create servo driver" << std::endl;
  boost::shared_ptr<Adafruit_PWMServoDriver> pwm(new Adafruit_PWMServoDriver(addr));
  Servo servo(pwm, channel, maxLeft, maxRight);

  std::cout << "Set direction " << std::abs(direction) << "% " << ((direction < 0) ? "left" : "right") << std::endl;
  servo.setDirection(direction);

  return 0;
}
