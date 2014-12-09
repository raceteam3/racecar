#include "Robot.h"

#include <time.h>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/circular_buffer.hpp>
#include <ncursesw/ncurses.h>
#include <wiringPi.h>
#include "GP2Y0A02.h"


int main(int argc, const char** argv)
{
  Robot robot;
  robot.initialize((argc > 1) ? argv[1] : "robot.json");
  if(argc > 2 && strcmp(argv[2], "manual") == 0) {
    robot.runManual();
  } else {
    robot.run();
  }
  // left: 460
  // right :280
  return 0;
}

Robot::Robot()
{
}

Robot::~Robot()
{
  m_PWMDrivers.clear();
  m_SRF08Sensors.clear();
}

void Robot::initialize(const char* cfg)
{
  boost::property_tree::ptree pt;
  boost::property_tree::json_parser::read_json(cfg, pt);
  try {
    BOOST_FOREACH(const boost::property_tree::ptree::value_type& child, pt.get_child("robot.pwm")) {
      std::string name = child.second.get<std::string>("name");
      int addr = child.second.get<int>("address");
      int frequency = child.second.get<int>("frequency");
      boost::shared_ptr<Adafruit_PWMServoDriver> pwm(new Adafruit_PWMServoDriver(addr));
      pwm->setPWMFreq(frequency);
      m_PWMDrivers.insert(std::pair<std::string, boost::shared_ptr<Adafruit_PWMServoDriver> >(name, pwm));
    }
  } catch(boost::property_tree::ptree_error& e) {
    std::cout << "Failed to read pwm configuration" << std::endl;
    throw;
  }

  try {
    int channel = pt.get<int>("robot.steering.channel");
    int maxLeft = pt.get<int>("robot.steering.maxLeft");
    int maxRight = pt.get<int>("robot.steering.maxRight");
    boost::shared_ptr<Adafruit_PWMServoDriver> pwm = m_PWMDrivers.at(pt.get<std::string>("robot.steering.pwm"));
    m_Steering = boost::shared_ptr<Servo>(new Servo(pwm, channel, maxLeft, maxRight));
  } catch(boost::property_tree::ptree_error& e) {
    std::cout << "Failed to read steering servo configuration" << std::endl;
    throw;
  } catch(std::out_of_range& e) {
    std::cout << "Non-existing pwm driver for steering servo" << std::endl;
    throw;
  }

  try {
    int channel = pt.get<int>("robot.motor.channel");
    int maxForward = pt.get<int>("robot.motor.maxForward");
    int maxReverse = pt.get<int>("robot.motor.maxReverse");
    m_MinSpeed = pt.get<int>("robot.motor.minSpeed");
    boost::shared_ptr<Adafruit_PWMServoDriver> pwm = m_PWMDrivers.at(pt.get<std::string>("robot.motor.pwm"));
    m_Motor = boost::shared_ptr<Motor>(new Motor(pwm, channel, maxReverse, maxForward));
  } catch(boost::property_tree::ptree_error& e) {
    std::cout << "Failed to read motor configuration" << std::endl;
    throw;
  } catch(std::out_of_range& e) {
    std::cout << "Non-existing pwm driver for motor" << std::endl;
    throw;
  }

  try {
    BOOST_FOREACH(const boost::property_tree::ptree::value_type& child, pt.get_child("robot.ADCs")) {
      std::string type = child.second.get<std::string>("type");
      if(type == "ads1115") {
	int addr = child.second.get<int>("address");
	std::string name = child.second.get<std::string>("name");
	boost::shared_ptr<ADS1115> adc(new ADS1115(addr));
	adc->initialize();
	m_ADS1115ADCs.insert(std::pair<std::string, boost::shared_ptr<ADS1115> >(name, adc));
      } else {
	std::cout << "ADC type " << type << " is unknown" << std::endl;
      }
    }
  } catch(boost::property_tree::ptree_error& e) {
    std::cout << "Failed to read sensor configuration" << std::endl;
    throw;
  }

  try {
    BOOST_FOREACH(const boost::property_tree::ptree::value_type& child, pt.get_child("robot.sensors")) {
      std::string type = child.second.get<std::string>("type");
      if(type == "srf08") {
        int addr = child.second.get<int>("address");
	int angle = child.second.get<int>("angle");
	boost::shared_ptr<srf08> sensor(new srf08(addr));
	sensor->initiateRanging();
	m_SRF08Sensors.insert(std::pair<int, boost::shared_ptr<srf08> >(angle, sensor));
      } else if(type =="analog") {
          std::string driver = child.second.get<std::string>("driver");
          int channel = child.second.get<int>("channel");
          int angle = child.second.get<int>("angle");
          boost::shared_ptr<ADS1115> adc;
          try {
              adc = m_ADS1115ADCs.at(child.second.get<std::string>("adc"));
          } catch(std::out_of_range& e) {
              std::cout << "Non-existing ADC for analog sensor" << std::endl;
              continue;
          }
          if(driver == "GP2Y0A02") {
              boost::shared_ptr<GP2Y0A02> sensor(new GP2Y0A02(adc, channel));
              m_AnalogDistanceSensors.insert(std::pair<int, boost::shared_ptr<AnalogDistanceSensor> >(angle, sensor));
          } else {
              std::cout << "Analog sensor driver " << driver << " is unknown" << std::endl;
          }
      } else if(type == "speed") {
        std::string driver = child.second.get<std::string>("driver");
        if(driver == "mouse") {
          if(m_MouseSpeedSensor) {
            std::cout << "Only one mouse speed sensor is currently supported!" << std::endl;
            continue;
          }
          std::string device = child.second.get<std::string>("device");
          m_MouseSpeedSensor.reset(new MouseSpeedSensor());
          if(!m_MouseSpeedSensor->initialize(device.c_str())) {
            std::cout << "Failed to initialize mouse speed sensor at " << device << std::endl;
            m_MouseSpeedSensor.reset();
          }
        } else {
          std::cout << "Speed sensor driver " << driver << " is unknown" << std::endl;
        }
      } else {
	std::cout << "Sensor type " << type << " is unknown" << std::endl;
      }
    }
  } catch(boost::property_tree::ptree_error& e) {
    std::cout << "Failed to read sensor configuration" << std::endl;
    throw;
  }

  m_LedPin = 14;
  m_ButtonPin = 15;
  wiringPiSetupGpio();
  /* LED is output and default off */
  pinMode(m_LedPin, OUTPUT);
  m_LedState = false;
  digitalWrite(m_LedPin, LOW);
  /* Button is input with pull-up */
  pinMode(m_ButtonPin, INPUT);
  pullUpDnControl(m_ButtonPin, PUD_UP);
}

void Robot::run()
{
  while(digitalRead(m_ButtonPin) == 1) {
    usleep(100);
  }
  digitalWrite(m_LedPin, HIGH);

  std::map<int, boost::shared_ptr<AnalogDistanceSensor> >::const_iterator analogIter=m_AnalogDistanceSensors.end();
  bool running = true;

  std::map<int, boost::circular_buffer<int> > distances;
  bool lastForward = false;
  int lastDirection = 0;

  int forwardSpeed = m_MinSpeed;
  int reverseSpeed = -m_MinSpeed;
  int moving = 0;
  int readSpeedCounter = 0;

  struct timespec lastSpeedChange = {0,0};

  while(running) {

    /* Sense */
    for(std::map<int, boost::shared_ptr<srf08> >::const_iterator iter=m_SRF08Sensors.begin(); iter!=m_SRF08Sensors.end(); ++iter) {
      if(iter->second->rangingComplete()) {
        std::map<int, boost::circular_buffer<int> >::iterator dIter = distances.find(iter->first);
        if(dIter == distances.end()) {
          distances.insert(std::pair<int, boost::circular_buffer<int> >(iter->first, boost::circular_buffer<int>(10)));
          dIter = distances.find(iter->first);
        }
        int range = iter->second->getRange();
        dIter->second.push_back(range);
        iter->second->initiateRanging();
      }
    }

    if(analogIter==m_AnalogDistanceSensors.end()) {
      analogIter=m_AnalogDistanceSensors.begin();
      analogIter->second->initiateRanging();
    }
    else if(analogIter->second->rangingComplete()) {
        std::map<int, boost::circular_buffer<int> >::iterator dIter = distances.find(analogIter->first);
        if(dIter == distances.end()) {
          distances.insert(std::pair<int, boost::circular_buffer<int> >(analogIter->first, boost::circular_buffer<int>(10)));
          dIter = distances.find(analogIter->first);
        }
        int range = analogIter->second->getRange();
        dIter->second.push_back(range);

      ++analogIter;
      if(analogIter==m_AnalogDistanceSensors.end()) {
        analogIter=m_AnalogDistanceSensors.begin();
      }
      analogIter->second->initiateRanging();
    }

    /* Decide */
    bool forward = true;
    int turnMultiplier = 1;
    std::map<int, boost::circular_buffer<int> >::iterator dIter = distances.find(0);
    if(!dIter->second.empty()) {
      if(dIter->second.back() < 80) {
	turnMultiplier = 2;
      }
      if(dIter->second.back() < 50) {
	turnMultiplier = 4;
      }
      if(dIter->second.back() < 30) {
        turnMultiplier = -2;
	forward = false;
      }
    }

    int direction = 0;
    std::map<int, boost::circular_buffer<int> >::iterator leftDistance = distances.find(135);
    std::map<int, boost::circular_buffer<int> >::iterator rightDistance = distances.find(45);

    std::map<int, boost::circular_buffer<int> >::iterator leftSoundDistance = distances.find(270);
    std::map<int, boost::circular_buffer<int> >::iterator rightSoundDistance = distances.find(90);

    if(!leftDistance->second.empty() && !rightDistance->second.empty() && !leftSoundDistance->second.empty() && !rightSoundDistance->second.empty()) {
      int right = rightDistance->second.back();
      int left = leftDistance->second.back();
      if(rightSoundDistance->second.back() < 25) {
	right = rightSoundDistance->second.back();
      }
      if(leftSoundDistance->second.back() < 20) {
	left = leftSoundDistance->second.back();
      }

      if(abs(left-right) > 20 || turnMultiplier != 1) {
	if(left > right + 20) {
	  if(right < 50 || turnMultiplier != 1) {
	    direction = -60;
	  }
	} else {
	  if(left < 70 || turnMultiplier != 1) {
	    direction = 60;
	  }
	}
      }
    }

    direction *= turnMultiplier;

    if(readSpeedCounter++ == 5) {
      if(m_MouseSpeedSensor) {
	MouseSpeedSensor::MouseSpeed speed = m_MouseSpeedSensor->getSpeed();
	if((forward && speed.y < -50) || (!forward && speed.y > 50)) {
	  moving = 10;
	} else if(moving) {
	  moving--;
	}
      } else {
	moving = 10;
      }
      readSpeedCounter = 0;
    }

    bool updateSpeed = false;
    if(!moving && forward == lastForward) {
      struct timespec spec;
       clock_gettime(CLOCK_MONOTONIC, &spec);
       if(spec.tv_sec == lastSpeedChange.tv_sec) {
         if(spec.tv_nsec - lastSpeedChange.tv_nsec > 1000000*500) {
           updateSpeed = true;
         }
       } else if(spec.tv_sec == lastSpeedChange.tv_sec + 1) {
         if(1000000000 - lastSpeedChange.tv_nsec + spec.tv_nsec > 1000000*500) {
           updateSpeed = true;
         }
       } else {
         updateSpeed = true;
       }
    }

    if(updateSpeed) {
      if(forward) {
        forwardSpeed+=2;
      } else {
        reverseSpeed-=2;
      }
    }

    /* Actuate */
    if(lastForward != forward || updateSpeed) {
      if(forward) {
        m_Motor->setSpeed(forwardSpeed);
      } else {
	m_Motor->setSpeed(reverseSpeed);
      }
      clock_gettime(CLOCK_MONOTONIC, &lastSpeedChange);
      lastForward = forward;
    }
    if(lastDirection != direction) {
      m_Steering->setDirection(direction);
      lastDirection = direction;
    }
    usleep(1000 * 10);

  }

#if 0
  /* Go forward at 10% of top speed for five seconds */
  std::cout << "Forward 5 seconds" << std::endl;
  m_Motor->setSpeed(10);
  usleep(5000 * 1000);

  std::cout << "Break 0.1 second" << std::endl;
  m_Motor->setSpeed(-10);
  usleep(100 * 1000);

  /* Go to reverse for 1 second*/
  std::cout << "Reverse 1 second" << std::endl;
  m_Motor->setSpeed(0);
  usleep(100 * 1000);
  m_Motor->setSpeed(-10);
  usleep(1000 * 1000);

  std::cout << "Halt" << std::endl;
  /* Come to a halt */
  m_Motor->setSpeed(0);
#endif
}

void Robot::runManual()
{
  bool run = true;

  int startx = 0;
  int starty = 0;

  int speed = 0;
  int turn = 0;

  std::map<int, boost::shared_ptr<AnalogDistanceSensor> >::const_iterator analogIter=m_AnalogDistanceSensors.end();

  WINDOW *win;
  int c;

  initscr();
  clear();
  noecho();
  curs_set(0);
  cbreak();/* Line buffering disabled. pass on everything */

  win = newwin(20, 50, starty, startx);
  keypad(win, TRUE);
  refresh();

  while(run) {
    wclear(win);
    box(win, 0, 0);
    mvwprintw(win, 1, 2, "SPEED: %i", speed);
    mvwprintw(win, 2, 2, "TURN: %i", turn);

    int i =0;
    for(std::map<int, boost::shared_ptr<srf08> >::const_iterator iter=m_SRF08Sensors.begin(); iter!=m_SRF08Sensors.end(); ++iter) {
      if(iter->second->rangingComplete()) {
	mvwprintw(win, 3+i, 2, "Sensor at %u degrees: %u cm", iter->first, iter->second->getRange());
      } else {
	mvwprintw(win, 3+i, 2, "Sensor at %u degrees: ranging", iter->first);
      }
      ++i;
    }
    for(std::map<int, boost::shared_ptr<AnalogDistanceSensor> >::const_iterator iter=m_AnalogDistanceSensors.begin(); iter!=m_AnalogDistanceSensors.end(); ++iter) {
      if(iter->second->rangingComplete()) {
	mvwprintw(win, 3+i, 2, "Analog sensor at %u degrees: %u cm", iter->first, iter->second->getRange());
      } else {
	mvwprintw(win, 3+i, 2, "Analog sensor at %u degrees: no value", iter->first);
      }
      ++i;
    }
    mvwprintw(win, 3+i, 2, "Button state: %s", (digitalRead(m_ButtonPin) ? "not pressed" : "pressed"));
    ++i;

    mvwprintw(win, 6+i, 2, "Arrows: Change speed/turn");
    mvwprintw(win, 7+i, 2, "s: Stop robot");
    mvwprintw(win, 8+i, 2, "q: Stop robot and quit");
    mvwprintw(win, 9+i, 2, "a: Start sensor acquisition");
    mvwprintw(win, 10+i, 2, "l: Toggle led");
    mvwprintw(win, 11+i, 2, "p: Print state");
    wrefresh(win);

    c = wgetch(win);
    switch(c)
    {
      // Increase speed
      case KEY_UP:
        if(speed < 1000)
        {
          speed++;
          m_Motor->setSpeed(speed);
        }
        break;
        // Decrease speed
      case KEY_DOWN:
        if(speed > -1000)
        {
          speed--;
          m_Motor->setSpeed(speed);
        }
        break;
        // Increase left turn
      case KEY_LEFT:
        if(turn > -1000)
        {
          turn--;
          m_Steering->setDirection(turn);
        }
        break;
        // Increase right turn
      case KEY_RIGHT:
        if(turn < 1000)
        {
          turn++;
          m_Steering->setDirection(turn);
        }
        break;
        // Stop robot
      case 's':
      case 'S':
        speed = 0;
        turn = 0;
        m_Motor->setSpeed(speed);
        m_Steering->setDirection(turn);
        break;
        // Stop robot and exit
      case 'q':
      case 'Q':
        speed = 0;
        turn = 0;
        m_Motor->setSpeed(speed);
        m_Steering->setDirection(turn);
        run = false;
        break;
      case 'a':
      case 'A':
	for(std::map<int, boost::shared_ptr<srf08> >::const_iterator iter=m_SRF08Sensors.begin(); iter!=m_SRF08Sensors.end(); ++iter) {
	  if(iter->second->rangingComplete()) {
	    std::cout << "Initiate ranging at " << iter->first << " degrees" << std::endl;
	    iter->second->initiateRanging();
	  }
	}

	if(analogIter==m_AnalogDistanceSensors.end()) {
	  analogIter=m_AnalogDistanceSensors.begin();
	  analogIter->second->initiateRanging();
	}
	else if(analogIter->second->rangingComplete()) {
	  ++analogIter;
	  if(analogIter==m_AnalogDistanceSensors.end()) {
	    analogIter=m_AnalogDistanceSensors.begin();
	  }
	  analogIter->second->initiateRanging();
	}
	break;
      case 'l':
      case 'L':
	digitalWrite(m_LedPin, m_LedState ? LOW : HIGH);
	m_LedState = !m_LedState;
	break;
      case 'p':
      case 'P':
	break;
    }
  }
  clrtoeol();
  refresh();
  endwin();
}
