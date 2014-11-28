#include "robot.h"

#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <ncursesw/ncurses.h>


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

Robot::Robot() : m_Steering(0), m_Motor(0)
{
}

Robot::~Robot()
{
  delete m_Steering;
  m_Steering = 0;

  delete m_Motor;
  m_Motor = 0;

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
    m_Steering = new Servo(pwm, channel, maxLeft, maxRight);
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
    boost::shared_ptr<Adafruit_PWMServoDriver> pwm = m_PWMDrivers.at(pt.get<std::string>("robot.motor.pwm"));
    m_Motor = new Servo(pwm, channel, maxReverse, maxForward);
  } catch(boost::property_tree::ptree_error& e) {
    std::cout << "Failed to read motor configuration" << std::endl;
    throw;
  } catch(std::out_of_range& e) {
    std::cout << "Non-existing pwm driver for motor" << std::endl;
    throw;
  }

  try {
    BOOST_FOREACH(const boost::property_tree::ptree::value_type& child, pt.get_child("robot.sensors")) {
      std::string type = child.second.get<std::string>("type");
      if(type == "srf08") {
	int addr = child.second.get<int>("address");
	int angle = child.second.get<int>("angle");
	boost::shared_ptr<srf08> sensor(new srf08(addr));
	m_SRF08Sensors.insert(std::pair<int, boost::shared_ptr<srf08> >(angle, sensor));	
      } else {
	std::cout << "Sensor type " << type << " is unkown" << std::endl;
      }
    }
  } catch(boost::property_tree::ptree_error& e) {
    std::cout << "Failed to read sensor configuration" << std::endl;
    throw;
  }
}

void Robot::run()
{
#if 0
  /* Go forward at 10% of top speed for five seconds */
  std::cout << "Forward 5 seconds" << std::endl;
  m_Motor->setDirection(10);
  usleep(5000 * 1000);

  std::cout << "Break 0.1 second" << std::endl;
  m_Motor->setDirection(-10);
  usleep(100 * 1000);

  /* Go to reverse for 1 second*/
  std::cout << "Reverse 1 second" << std::endl;
  m_Motor->setDirection(0);
  usleep(100 * 1000);
  m_Motor->setDirection(-10);
  usleep(1000 * 1000);

  std::cout << "Halt" << std::endl;
  /* Come to a halt */
  m_Motor->setDirection(0);
#endif
}

void Robot::runManual()
{
  bool run = true;

  int startx = 0;
  int starty = 0;

  int8_t speed = 0;
  int8_t turn = 0;

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

    mvwprintw(win, 6, 2, "Arrows: Change speed/turn");
    mvwprintw(win, 7, 2, "s: Stop robot");
    mvwprintw(win, 8, 2, "q: Stop robot and quit");
    wrefresh(win);

    c = wgetch(win);
    switch(c)
    {
      // Increase speed
      case KEY_UP:
        if(speed < 100)
        {
          speed++;
          m_Motor->setDirection(speed);
        }
        break;
        // Decrease speed
      case KEY_DOWN:
        if(speed > -100)
        {
          speed--;
          m_Motor->setDirection(speed);
        }
        break;
        // Increase left turn
      case KEY_LEFT:
        if(turn > -100)
        {
          turn--;
          m_Steering->setDirection(turn);
        }
        break;
        // Increase right turn
      case KEY_RIGHT:
        if(turn < 100)
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
        m_Motor->setDirection(speed);
        m_Steering->setDirection(turn);
        break;
        // Stop robot and exit
      case 'q':
      case 'Q':
        speed = 0;
        turn = 0;
        m_Motor->setDirection(speed);
        m_Steering->setDirection(turn);
        run = false;
        break;
    }
  }
  clrtoeol();
  refresh();
  endwin();
}
