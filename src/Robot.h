#ifndef ROBOT_H
#define ROBOT_H

#include "Adafruit_PWMServoDriver.h"
#include "Servo.h"
#include "Motor.h"
#include "SRF08.h"
#include "AnalogDistanceSensor.h"
#include "ADS1115.h"
#include "MouseSpeedSensor.h"

#include <stdint.h>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/asio/impl/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include <map>
#include <string>

class Robot
{
 public:
  Robot();
  ~Robot();

  void initialize(const char* cfg);
  void run();
  void runManual();

 private:
  void signalHandler(const boost::system::error_code& ec, int signalNumber);

 private:
  boost::shared_ptr<Servo> m_Steering;
  boost::shared_ptr<Motor> m_Motor;
  std::map<std::string, boost::shared_ptr<Adafruit_PWMServoDriver> > m_PWMDrivers;
  std::map<int, boost::shared_ptr<srf08> > m_SRF08Sensors;
  std::map<int, boost::shared_ptr<AnalogDistanceSensor> > m_AnalogDistanceSensors;
  std::map<std::string, boost::shared_ptr<ADS1115> > m_ADS1115ADCs;
  boost::shared_ptr<MouseSpeedSensor> m_MouseSpeedSensor;
  int m_ButtonPin;
  int m_LedPin;
  int m_InitialForwardSpeed;
  int m_InitialReverseSpeed;
  bool m_LedState;
  bool m_Running;

  boost::asio::io_service m_IoService;
  boost::asio::signal_set m_Signals;
};
#endif
