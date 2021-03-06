/***************************************************
  This is a library for our Adafruit 16-channel PWM & Servo driver

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/815

  These displays use I2C to communicate, 2 pins are required to
  interface. For Arduino UNOs, thats SCL -> Analog 5, SDA -> Analog 4

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_PWMServoDriver.h"
#include <wiringPiI2C.h>
#include <iostream>
#include <unistd.h>
#include <cmath>

// Set to true to print some debug messages, or false to disable them.
#define ENABLE_DEBUG_OUTPUT true

Adafruit_PWMServoDriver::Adafruit_PWMServoDriver(uint8_t addr)
{
  m_Fd = wiringPiI2CSetup(addr/2);

  write8(PCA9685_MODE2, PCA9685_BIT_OUTDRV);
  write8(PCA9685_MODE1, PCA9685_BIT_ALLCALL);
  usleep(5);

  uint8_t mode1 = read8(PCA9685_MODE1);
  write8(PCA9685_MODE1, mode1);
  usleep(5);
}

void Adafruit_PWMServoDriver::reset(void)
{
 write8(PCA9685_MODE1, 0x0);
}

void Adafruit_PWMServoDriver::setPWMFreq(float freq)
{
  freq *= 0.9;  // Correct for overshoot in the frequency setting (see issue #11).
  float prescaleval = 25000000;
  prescaleval /= 4096;
  prescaleval /= freq;
  prescaleval -= 1;
  if (ENABLE_DEBUG_OUTPUT) {
    std::cout << "Estimated pre-scale: " <<  prescaleval << std::endl;
  }
  uint8_t prescale = std::floor(prescaleval + 0.5);
  if (ENABLE_DEBUG_OUTPUT) {
    std::cout << "Final pre-scale: " << prescale << std::endl;
  }

  uint8_t oldmode = read8(PCA9685_MODE1);
  uint8_t newmode = (oldmode&0x7F) | PCA9685_BIT_SLEEP;
  write8(PCA9685_MODE1, newmode); // go to sleep
  write8(PCA9685_PRESCALE, prescale); // set the prescaler
  write8(PCA9685_MODE1, oldmode);
  usleep(5);
  write8(PCA9685_MODE1, oldmode | PCA9685_BIT_RESTART);  //  This sets the MODE1 register to turn on auto increment.
                                          // This is why the beginTransmission below was not working.
  //  Serial.print("Mode now 0x"); Serial.println(read8(PCA9685_MODE1), HEX);
}

void Adafruit_PWMServoDriver::setPWM(uint8_t num, uint16_t on, uint16_t off)
{
  //Serial.print("Setting PWM "); Serial.print(num); Serial.print(": "); Serial.print(on); Serial.print("->"); Serial.println(off);

  write8(LED0_ON_L+4*num, on & 0xFF);
  write8(LED0_ON_H+4*num, on >> 8);
  write8(LED0_OFF_L+4*num, off & 0xFF);
  write8(LED0_OFF_H+4*num, off >> 8);
}

// Sets pin without having to deal with on/off tick placement and properly handles
// a zero value as completely off.  Optional invert parameter supports inverting
// the pulse for sinking to ground.  Val should be a value from 0 to 4095 inclusive.
void Adafruit_PWMServoDriver::setPin(uint8_t num, uint16_t val, bool invert)
{
  // Clamp value between 0 and 4095 inclusive.
  val = std::min<uint16_t>(val, 4095);
  if (invert) {
    if (val == 0) {
      // Special value for signal fully on.
      setPWM(num, 4096, 0);
    }
    else if (val == 4095) {
      // Special value for signal fully off.
      setPWM(num, 0, 4096);
    }
    else {
      setPWM(num, 0, 4095-val);
    }
  }
  else {
    if (val == 4095) {
      // Special value for signal fully on.
      setPWM(num, 4096, 0);
    }
    else if (val == 0) {
      // Special value for signal fully off.
      setPWM(num, 0, 4096);
    }
    else {
      setPWM(num, 0, val);
    }
  }
}

uint8_t Adafruit_PWMServoDriver::read8(uint8_t addr)
{
  return wiringPiI2CReadReg8(m_Fd, addr);
}

void Adafruit_PWMServoDriver::write8(uint8_t addr, uint8_t d)
{
  wiringPiI2CWriteReg8(m_Fd, addr, d);
}
