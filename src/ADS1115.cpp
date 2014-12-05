// I2Cdev library collection - ADS1115 I2C device class
// Based on Texas Instruments ADS1113/4/5 datasheet, May 2009 (SBAS444B, revised October 2009)
// Note that the ADS1115 uses 16-bit registers, not 8-bit registers.
// 8/2/2011 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//     2013-05-05 - Add debug information.  Rename methods to match datasheet.
//     2011-11-06 - added getVoltage, F. Farzanegan
//     2011-10-29 - added getDifferentialx() methods, F. Farzanegan
//     2011-08-02 - initial release
/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2011 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#include "ADS1115.h"
#include <wiringPiI2C.h>
#include <iostream>

/** Default constructor, uses default I2C address.
 * @see ADS1115_DEFAULT_ADDRESS
 */
ADS1115::ADS1115() {
    m_Fd = wiringPiI2CSetup(ADS1115_DEFAULT_ADDRESS / 2);
}

/** Specific address constructor.
 * @param address I2C address
 * @see ADS1115_DEFAULT_ADDRESS
 * @see ADS1115_ADDRESS_ADDR_GND
 * @see ADS1115_ADDRESS_ADDR_VDD
 * @see ADS1115_ADDRESS_ADDR_SDA
 * @see ADS1115_ADDRESS_ADDR_SDL
 */
ADS1115::ADS1115(uint8_t address) {
    m_Fd = wiringPiI2CSetup(address / 2);
}

/** Power on and prepare for general usage.
 * This device is ready to use automatically upon power-up. It defaults to
 * single-shot read mode, P0/N1 mux, 2.048v gain, 128 samples/sec, default
 * comparator with hysterysis, active-low polarity, non-latching comparator,
 * and comparater-disabled operation. 
 */
void ADS1115::initialize() {
  setMultiplexer(ADS1115_MUX_P0_N1);
  setGain(ADS1115_PGA_2P048);
  setMode(ADS1115_MODE_SINGLESHOT);
  setRate(ADS1115_RATE_128);
  setComparatorMode(ADS1115_COMP_MODE_HYSTERESIS);
  setComparatorPolarity(ADS1115_COMP_POL_ACTIVE_LOW);
  setComparatorLatchEnabled(ADS1115_COMP_LAT_NON_LATCHING);
  setComparatorQueueMode(ADS1115_COMP_QUE_DISABLE);
}

/** Verify the I2C connection.
 * Make sure the device is connected and responds as expected.
 * @return True if connection is valid, false otherwise
 */
bool ADS1115::testConnection() {
    return wiringPiI2CReadReg16(m_Fd, ADS1115_RA_CONVERSION) != - 1;
}

/** Wait until the single-shot conversion is finished
 * Retry at most 'max_retries' times
 * conversion is finished, then return;
 * @see ADS1115_OS_INACTIVE
 */
void ADS1115::waitBusy(uint16_t max_retries) {  
  for(uint16_t i = 0; i < max_retries; i++) {
    if (getOpStatus()==ADS1115_OS_INACTIVE) break;
  }
}


/** Read differential value based on current MUX configuration.
 * The default MUX setting sets the device to get the differential between the
 * AIN0 and AIN1 pins. There are 8 possible MUX settings, but if you are using
 * all four input pins as single-end voltage sensors, then the default option is
 * not what you want; instead you will need to set the MUX to compare the
 * desired AIN* pin with GND. There are shortcut methods (getConversion*) to do
 * this conveniently, but you can also do it manually with setMultiplexer()
 * followed by this method.
 *
 * In single-shot mode, this register may not have fresh data. You need to write
 * a 1 bit to the MSB of the CONFIG register to trigger a single read/conversion
 * before this will be populated with fresh data. This technique is not as
 * effortless, but it has enormous potential to save power by only running the
 * comparison circuitry when needed.
 *
 * @return 16-bit signed differential value
 * @see getConversionP0N1();
 * @see getConversionPON3();
 * @see getConversionP1N3();
 * @see getConversionP2N3();
 * @see getConversionP0GND();
 * @see getConversionP1GND();
 * @see getConversionP2GND();
 * @see getConversionP3GND);
 * @see setMultiplexer();
 * @see ADS1115_RA_CONVERSION
 * @see ADS1115_MUX_P0_N1
 * @see ADS1115_MUX_P0_N3
 * @see ADS1115_MUX_P1_N3
 * @see ADS1115_MUX_P2_N3
 * @see ADS1115_MUX_P0_NG
 * @see ADS1115_MUX_P1_NG
 * @see ADS1115_MUX_P2_NG
 * @see ADS1115_MUX_P3_NG
 */
int16_t ADS1115::getConversion() {
#if 0
    if (devMode == ADS1115_MODE_SINGLESHOT) 
    {  
      setOpStatus(ADS1115_OS_ACTIVE);
      ADS1115::waitBusy(1000);
      
    }
#endif
      
    return readRegister(ADS1115_RA_CONVERSION);
}
/** Get AIN0/N1 differential.
 * This changes the MUX setting to AIN0/N1 if necessary, triggers a new
 * measurement (also only if necessary), then gets the differential value
 * currently in the CONVERSION register.
 * @return 16-bit signed differential value
 * @see getConversion()
 */
int16_t ADS1115::getConversionP0N1() {
    if (muxMode != ADS1115_MUX_P0_N1) setMultiplexer(ADS1115_MUX_P0_N1);
    return getConversion();
}

/** Get AIN0/N3 differential.
 * This changes the MUX setting to AIN0/N3 if necessary, triggers a new
 * measurement (also only if necessary), then gets the differential value
 * currently in the CONVERSION register.
 * @return 16-bit signed differential value
 * @see getConversion()
 */
int16_t ADS1115::getConversionP0N3() {
    if (muxMode != ADS1115_MUX_P0_N3) setMultiplexer(ADS1115_MUX_P0_N3);
    return getConversion();
}

/** Get AIN1/N3 differential.
 * This changes the MUX setting to AIN1/N3 if necessary, triggers a new
 * measurement (also only if necessary), then gets the differential value
 * currently in the CONVERSION register.
 * @return 16-bit signed differential value
 * @see getConversion()
 */
int16_t ADS1115::getConversionP1N3() {
    if (muxMode != ADS1115_MUX_P1_N3) setMultiplexer(ADS1115_MUX_P1_N3);
    return getConversion();
}

/** Get AIN2/N3 differential.
 * This changes the MUX setting to AIN2/N3 if necessary, triggers a new
 * measurement (also only if necessary), then gets the differential value
 * currently in the CONVERSION register.
 * @return 16-bit signed differential value
 * @see getConversion()
 */
int16_t ADS1115::getConversionP2N3() {
    if (muxMode != ADS1115_MUX_P2_N3) setMultiplexer(ADS1115_MUX_P2_N3);
    return getConversion();
}

/** Get AIN0/GND differential.
 * This changes the MUX setting to AIN0/GND if necessary, triggers a new
 * measurement (also only if necessary), then gets the differential value
 * currently in the CONVERSION register.
 * @return 16-bit signed differential value
 * @see getConversion()
 */
int16_t ADS1115::getConversionP0GND() {
    if (muxMode != ADS1115_MUX_P0_NG) setMultiplexer(ADS1115_MUX_P0_NG);
    return getConversion();
}
/** Get AIN1/GND differential.
 * This changes the MUX setting to AIN1/GND if necessary, triggers a new
 * measurement (also only if necessary), then gets the differential value
 * currently in the CONVERSION register.
 * @return 16-bit signed differential value
 * @see getConversion()
 */
int16_t ADS1115::getConversionP1GND() {
    if (muxMode != ADS1115_MUX_P1_NG) setMultiplexer(ADS1115_MUX_P1_NG);
    return getConversion();
}
/** Get AIN2/GND differential.
 * This changes the MUX setting to AIN2/GND if necessary, triggers a new
 * measurement (also only if necessary), then gets the differential value
 * currently in the CONVERSION register.
 * @return 16-bit signed differential value
 * @see getConversion()
 */
int16_t ADS1115::getConversionP2GND() {
    if (muxMode != ADS1115_MUX_P2_NG) setMultiplexer(ADS1115_MUX_P2_NG);
    return getConversion();
}
/** Get AIN3/GND differential.
 * This changes the MUX setting to AIN3/GND if necessary, triggers a new
 * measurement (also only if necessary), then gets the differential value
 * currently in the CONVERSION register.
 * @return 16-bit signed differential value
 * @see getConversion()
 */
int16_t ADS1115::getConversionP3GND() {
    if (muxMode != ADS1115_MUX_P3_NG) setMultiplexer(ADS1115_MUX_P3_NG);
    return getConversion();
}

/** Get the current voltage reading
 * Read the current differential and return it multiplied
 * by the constant for the current gain.  mV is returned to
 * increase the precision of the voltage
 *
 */
float ADS1115::getMilliVolts() {
  switch (pgaMode) { 
    case ADS1115_PGA_6P144:
      return (getConversion() * ADS1115_MV_6P144);
      break;    
    case ADS1115_PGA_4P096:
      return (getConversion() * ADS1115_MV_4P096);
      break;             
    case ADS1115_PGA_2P048:    
      return (getConversion() * ADS1115_MV_2P048);
      break;       
    case ADS1115_PGA_1P024:     
      return (getConversion() * ADS1115_MV_1P024);
      break;       
    case ADS1115_PGA_0P512:      
      return (getConversion() * ADS1115_MV_0P512);
      break;       
    case ADS1115_PGA_0P256:           
    case ADS1115_PGA_0P256B:          
    case ADS1115_PGA_0P256C:      
      return (getConversion() * ADS1115_MV_0P256);
      break;       
  }
}

/**
 * Return the current multiplier for the PGA setting.
 * 
 * This may be directly retreived by using getMilliVolts(),
 * but this causes an independent read.  This function could
 * be used to average a number of reads from the getConversion()
 * getConversionx() functions and cut downon the number of 
 * floating-point calculations needed.
 *
 */
 
float ADS1115::getMvPerCount() {
  switch (pgaMode) {
    case ADS1115_PGA_6P144:
      return ADS1115_MV_6P144;
      break;    
    case ADS1115_PGA_4P096:
      return  ADS1115_MV_4P096;
      break;             
    case ADS1115_PGA_2P048:    
      return ADS1115_MV_2P048;
      break;       
    case ADS1115_PGA_1P024:     
      return ADS1115_MV_1P024;
      break;       
    case ADS1115_PGA_0P512:      
      return ADS1115_MV_0P512;
      break;       
    case ADS1115_PGA_0P256:           
    case ADS1115_PGA_0P256B:          
    case ADS1115_PGA_0P256C:      
      return ADS1115_MV_0P256;
      break;       
  }
}

// CONFIG register

/** Get operational status.
 * @return Current operational status (0 for active conversion, 1 for inactive)
 * @see ADS1115_OS_ACTIVE
 * @see ADS1115_OS_INACTIVE
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_OS_BIT
 */
uint8_t ADS1115::getOpStatus() {
    return ((readRegister(ADS1115_RA_CONFIG) & (1 << ADS1115_CFG_OS_BIT)) ? ADS1115_OS_INACTIVE : ADS1115_OS_ACTIVE);
}
/** Set operational status.
 * This bit can only be written while in power-down mode (no conversions active).
 * @param status New operational status (0 does nothing, 1 to trigger conversion)
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_OS_BIT
 */
void ADS1115::setOpStatus(uint8_t status) {
    writeBitW(ADS1115_RA_CONFIG, ADS1115_CFG_OS_BIT, status);
}
/** Get multiplexer connection.
 * @return Current multiplexer connection setting
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_MUX_BIT
 * @see ADS1115_CFG_MUX_LENGTH
 */
uint8_t ADS1115::getMultiplexer() {
    muxMode = (uint8_t)readBitsW(ADS1115_RA_CONFIG, ADS1115_CFG_MUX_BIT, ADS1115_CFG_MUX_LENGTH);
    return muxMode;
}
/** Set multiplexer connection.  Continous mode may fill the conversion register
 * with data before the MUX setting has taken effect.  A stop/start of the conversion
 * is done to reset the values.
 * @param mux New multiplexer connection setting
 * @see ADS1115_MUX_P0_N1
 * @see ADS1115_MUX_P0_N3
 * @see ADS1115_MUX_P1_N3
 * @see ADS1115_MUX_P2_N3
 * @see ADS1115_MUX_P0_NG
 * @see ADS1115_MUX_P1_NG
 * @see ADS1115_MUX_P2_NG
 * @see ADS1115_MUX_P3_NG
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_MUX_BIT
 * @see ADS1115_CFG_MUX_LENGTH
 */
void ADS1115::setMultiplexer(uint8_t mux) {
    if (writeBitsW(ADS1115_RA_CONFIG, ADS1115_CFG_MUX_BIT, ADS1115_CFG_MUX_LENGTH, mux)) {
        muxMode = mux;
        if (devMode == ADS1115_MODE_CONTINUOUS) {
          // Force a stop/start
          setMode(ADS1115_MODE_SINGLESHOT);
          getConversion();
          setMode(ADS1115_MODE_CONTINUOUS);
        }
    }
    
}
/** Get programmable gain amplifier level.
 * @return Current programmable gain amplifier level
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_PGA_BIT
 * @see ADS1115_CFG_PGA_LENGTH
 */
uint8_t ADS1115::getGain() {
    pgaMode = (uint8_t)readBitsW(ADS1115_RA_CONFIG, ADS1115_CFG_PGA_BIT, ADS1115_CFG_PGA_LENGTH);
    return pgaMode;
}
/** Set programmable gain amplifier level.  
 * Continous mode may fill the conversion register
 * with data before the gain setting has taken effect.  A stop/start of the conversion
 * is done to reset the values.
 * @param gain New programmable gain amplifier level
 * @see ADS1115_PGA_6P144
 * @see ADS1115_PGA_4P096
 * @see ADS1115_PGA_2P048
 * @see ADS1115_PGA_1P024
 * @see ADS1115_PGA_0P512
 * @see ADS1115_PGA_0P256
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_PGA_BIT
 * @see ADS1115_CFG_PGA_LENGTH
 */
void ADS1115::setGain(uint8_t gain) {
    if (writeBitsW(ADS1115_RA_CONFIG, ADS1115_CFG_PGA_BIT, ADS1115_CFG_PGA_LENGTH, gain)) {
      pgaMode = gain;
         if (devMode == ADS1115_MODE_CONTINUOUS) {
            // Force a stop/start
            setMode(ADS1115_MODE_SINGLESHOT);
            getConversion();
            setMode(ADS1115_MODE_CONTINUOUS);
         }
    }
}
/** Get device mode.
 * @return Current device mode
 * @see ADS1115_MODE_CONTINUOUS
 * @see ADS1115_MODE_SINGLESHOT
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_MODE_BIT
 */
uint8_t ADS1115::getMode() {
    return ((readRegister(ADS1115_RA_CONFIG) & (1 << ADS1115_CFG_MODE_BIT)) ? ADS1115_MODE_SINGLESHOT : ADS1115_MODE_CONTINUOUS);
}
/** Set device mode.
 * @param mode New device mode
 * @see ADS1115_MODE_CONTINUOUS
 * @see ADS1115_MODE_SINGLESHOT
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_MODE_BIT
 */
void ADS1115::setMode(uint8_t mode) {
    if (writeBitW(ADS1115_RA_CONFIG, ADS1115_CFG_MODE_BIT, mode)) {
        devMode = mode;
    }
}
/** Get data rate.
 * @return Current data rate
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_DR_BIT
 * @see ADS1115_CFG_DR_LENGTH
 */
uint8_t ADS1115::getRate() {
    return readBitsW(ADS1115_RA_CONFIG, ADS1115_CFG_DR_BIT, ADS1115_CFG_DR_LENGTH);
}
/** Set data rate.
 * @param rate New data rate
 * @see ADS1115_RATE_8
 * @see ADS1115_RATE_16
 * @see ADS1115_RATE_32
 * @see ADS1115_RATE_64
 * @see ADS1115_RATE_128
 * @see ADS1115_RATE_250
 * @see ADS1115_RATE_475
 * @see ADS1115_RATE_860
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_DR_BIT
 * @see ADS1115_CFG_DR_LENGTH
 */
void ADS1115::setRate(uint8_t rate) {
    writeBitsW(ADS1115_RA_CONFIG, ADS1115_CFG_DR_BIT, ADS1115_CFG_DR_LENGTH, rate);
}
/** Get comparator mode.
 * @return Current comparator mode
 * @see ADS1115_COMP_MODE_HYSTERESIS
 * @see ADS1115_COMP_MODE_WINDOW
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_COMP_MODE_BIT
 */
uint8_t ADS1115::getComparatorMode() {
  return ((readRegister(ADS1115_RA_CONFIG) & (1 << ADS1115_CFG_COMP_MODE_BIT)) ? ADS1115_COMP_MODE_WINDOW : ADS1115_COMP_MODE_HYSTERESIS);
}
/** Set comparator mode.
 * @param mode New comparator mode
 * @see ADS1115_COMP_MODE_HYSTERESIS
 * @see ADS1115_COMP_MODE_WINDOW
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_COMP_MODE_BIT
 */
void ADS1115::setComparatorMode(uint8_t mode) {
    writeBitW(ADS1115_RA_CONFIG, ADS1115_CFG_COMP_MODE_BIT, mode);
}
/** Get comparator polarity setting.
 * @return Current comparator polarity setting
 * @see ADS1115_COMP_POL_ACTIVE_LOW
 * @see ADS1115_COMP_POL_ACTIVE_HIGH
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_COMP_POL_BIT
 */
uint8_t ADS1115::getComparatorPolarity() {
  return ((readRegister(ADS1115_RA_CONFIG) & (1 << ADS1115_CFG_COMP_POL_BIT)) ? ADS1115_COMP_POL_ACTIVE_HIGH : ADS1115_COMP_POL_ACTIVE_LOW);
}
/** Set comparator polarity setting.
 * @param polarity New comparator polarity setting
 * @see ADS1115_COMP_POL_ACTIVE_LOW
 * @see ADS1115_COMP_POL_ACTIVE_HIGH
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_COMP_POL_BIT
 */
void ADS1115::setComparatorPolarity(uint8_t polarity) {
    writeBitW(ADS1115_RA_CONFIG, ADS1115_CFG_COMP_POL_BIT, polarity);
}
/** Get comparator latch enabled value.
 * @return Current comparator latch enabled value
 * @see ADS1115_COMP_LAT_NON_LATCHING
 * @see ADS1115_COMP_LAT_LATCHING
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_COMP_LAT_BIT
 */
bool ADS1115::getComparatorLatchEnabled() {
  return ((readRegister(ADS1115_RA_CONFIG) & (1 << ADS1115_CFG_COMP_LAT_BIT)) ? ADS1115_COMP_LAT_LATCHING : ADS1115_COMP_LAT_NON_LATCHING);
}
/** Set comparator latch enabled value.
 * @param enabled New comparator latch enabled value
 * @see ADS1115_COMP_LAT_NON_LATCHING
 * @see ADS1115_COMP_LAT_LATCHING
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_COMP_LAT_BIT
 */
void ADS1115::setComparatorLatchEnabled(bool enabled) {
    writeBitW(ADS1115_RA_CONFIG, ADS1115_CFG_COMP_LAT_BIT, enabled);
}
/** Get comparator queue mode.
 * @return Current comparator queue mode
 * @see ADS1115_COMP_QUE_ASSERT1
 * @see ADS1115_COMP_QUE_ASSERT2
 * @see ADS1115_COMP_QUE_ASSERT4
 * @see ADS1115_COMP_QUE_DISABLE
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_COMP_QUE_BIT
 * @see ADS1115_CFG_COMP_QUE_LENGTH
 */
uint8_t ADS1115::getComparatorQueueMode() {
    return readBitsW(ADS1115_RA_CONFIG, ADS1115_CFG_COMP_QUE_BIT, ADS1115_CFG_COMP_QUE_LENGTH);
}
/** Set comparator queue mode.
 * @param mode New comparator queue mode
 * @see ADS1115_COMP_QUE_ASSERT1
 * @see ADS1115_COMP_QUE_ASSERT2
 * @see ADS1115_COMP_QUE_ASSERT4
 * @see ADS1115_COMP_QUE_DISABLE
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_COMP_QUE_BIT
 * @see ADS1115_CFG_COMP_QUE_LENGTH
 */
void ADS1115::setComparatorQueueMode(uint8_t mode) {
    writeBitsW(ADS1115_RA_CONFIG, ADS1115_CFG_COMP_QUE_BIT, ADS1115_CFG_COMP_QUE_LENGTH, mode);
}

// *_THRESH registers

/** Get low threshold value.
 * @return Current low threshold value
 * @see ADS1115_RA_LO_THRESH
 */
int16_t ADS1115::getLowThreshold() {
  return readRegister(ADS1115_RA_LO_THRESH);
}
/** Set low threshold value.
 * @param threshold New low threshold value
 * @see ADS1115_RA_LO_THRESH
 */
void ADS1115::setLowThreshold(int16_t threshold) {
  writeRegister(ADS1115_RA_LO_THRESH, threshold);
}
/** Get high threshold value.
 * @return Current high threshold value
 * @see ADS1115_RA_HI_THRESH
 */
int16_t ADS1115::getHighThreshold() {
  return readRegister(ADS1115_RA_HI_THRESH);
}
/** Set high threshold value.
 * @param threshold New high threshold value
 * @see ADS1115_RA_HI_THRESH
 */
void ADS1115::setHighThreshold(int16_t threshold) {
  writeRegister(ADS1115_RA_HI_THRESH, threshold);
}

// Create a mask between two bits
unsigned createMask(unsigned a, unsigned b)
{
   unsigned mask = 0;
   for (unsigned i=a; i<=b; i++)
       mask |= 1 << i;
   return mask;
}

uint16_t shiftDown(uint16_t extractFrom, int places)
{
  return (extractFrom >> places);
}


uint16_t getValueFromBits(uint16_t extractFrom, int high, int length) 
{
   int low= high-length +1;
   uint16_t mask = createMask(low ,high);
   return shiftDown(extractFrom & mask, low); 
}

/** Show all the config register settings
 */
void ADS1115::showConfigRegister()
{
  uint16_t configRegister = readRegister(ADS1115_RA_CONFIG);
    
    
#ifdef ADS1115_SERIAL_DEBUG
    std::cout << std::hex;
    std::cout << "Register is:" << configRegister << std::endl;
  
    std::cout << "OS:\t" << getValueFromBits(configRegister, ADS1115_CFG_OS_BIT,1) << std::endl;
      std::cout << "MUX:\t" << getValueFromBits(configRegister,  
        ADS1115_CFG_MUX_BIT,ADS1115_CFG_MUX_LENGTH) << std::endl;
        
      std::cout << "PGA:\t" << getValueFromBits(configRegister, 
        ADS1115_CFG_PGA_BIT,ADS1115_CFG_PGA_LENGTH) << std::endl;
        
      std::cout << "MODE:\t" << getValueFromBits(configRegister,
        ADS1115_CFG_MODE_BIT,1) << std::endl;
        
      std::cout << "DR:\t" << getValueFromBits(configRegister, 
        ADS1115_CFG_DR_BIT,ADS1115_CFG_DR_LENGTH) << std::endl;
        
      std::cout << "CMP_MODE:\t" << getValueFromBits(configRegister, 
        ADS1115_CFG_COMP_MODE_BIT,1) << std::endl;
        
      std::cout << "CMP_POL:\t" << getValueFromBits(configRegister, 
        ADS1115_CFG_COMP_POL_BIT,1) << std::endl;
        
      std::cout << "CMP_LAT:\t" << getValueFromBits(configRegister, 
        ADS1115_CFG_COMP_LAT_BIT,1) << std::endl;
        
      std::cout << "CMP_QUE:\t" << getValueFromBits(configRegister, 
        ADS1115_CFG_COMP_QUE_BIT,ADS1115_CFG_COMP_QUE_LENGTH) << std::endl;
      std::cout << std::dec;
#endif
    
};


bool ADS1115::writeBitW(uint8_t regAddr, uint8_t bitNum, uint16_t data)
{
  uint16_t val = readRegister(regAddr);
    if(data) {
        val |= (1 << bitNum);
    } else {
        val &= ~(1 << bitNum);
    }
    return (writeRegister(regAddr, val) >= 0);
}

bool ADS1115::writeBitsW(uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t data)
{
  int ret = readRegister(regAddr);
    if(ret < 0) {
        return false;
    }
    uint16_t val = ret & 0xffff;
    uint16_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    data <<= (bitStart - length + 1); // shift data into correct position
    data &= mask; // zero all non-important bits in data
    val &= ~(mask); // zero all important bits in existing word
    val |= data; // combine data with existing word
    return (writeRegister(regAddr, val) >= 0);
}

uint16_t ADS1115::readBitsW(uint8_t regAddr, uint8_t bitStart, uint8_t length)
{
  uint16_t val = readRegister(ADS1115_RA_CONFIG);
    uint16_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    val &= mask;
    val >>= (bitStart - length + 1);
    return val;
}

bool ADS1115::writeRegister(uint8_t regAddr, uint16_t data)
{
  uint8_t buf[2];
  buf[1] = (data & 0xFF);
  buf[0] = ((data >> 8) & 0xFF);
  return (wiringPiI2CWriteBlockData(m_Fd, regAddr, 2, buf) > 0);
}

uint16_t ADS1115::readRegister(uint8_t regAddr)
{
  uint8_t buf[2];
  wiringPiI2CReadBlockData(m_Fd, regAddr, 2, buf);
  uint16_t data = ((buf[0] << 8) | buf[1]);
  return data;
}
