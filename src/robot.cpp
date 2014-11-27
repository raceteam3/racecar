#include "robot.h"

#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

int main(int argc, const char** argv)
{
  Robot robot;
  robot.initialize((argc > 1) ? argv[1] : "robot.json");
  robot.run();
  // left: 460
  // right :280
  return 0;
}

Robot::Robot() : m_Servo(0)
{
}

Robot::~Robot()
{
  delete m_Servo;
  m_Servo = 0;

  m_UltraSonicSensors.clear();
}

void Robot::initialize(const char* cfg)
{
  boost::property_tree::ptree pt;
  boost::property_tree::json_parser::read_json(cfg, pt);
  try {
    int addr = pt.get<int>("robot.servo.address");
    int channel = pt.get<int>("robot.servo.channel");
    int maxLeft = pt.get<int>("robot.servo.maxLeft");
    int maxRight = pt.get<int>("robot.servo.maxRight");
    m_Servo = new Servo(addr, channel, maxLeft, maxRight);
  } catch(boost::property_tree::ptree_error& e) {
    std::cout << "Failed to read servo configuration" << std::endl;
    throw;
  }
}

void Robot::run()
{
}
