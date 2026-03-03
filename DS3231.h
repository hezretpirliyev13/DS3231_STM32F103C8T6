#ifndef _DS3231_H
#define _DS3231_H

#include <cstdint>
#include <Wire.h>
#if defined(MCU_STM32F103C8)
#pragma message("STM32F103C8 found")
extern TwoWire wire2;



#elif defined(ARDUINO_ARCH_AVR)
#pragma message("Arduino arch AVR family")
#else
#error "Sorry you didn't choose exact microcontroller. Please choose \"STM32F103C8\""
#endif

#define DS3231_ADDR_R 0xD1
#define DS3231_ADDR_W 0xD0

#define DS3231_ADDR 0x68
#define DS3231_ADDR_2 0x57

#define REG_SEC 0x00
#define REG_MIN 0x01
#define REG_HOUR 0x02
#define REG_DOW 0x03
#define REG_DATE 0x04
#define REG_MON 0x05
#define REG_YEAR 0x06
#define REG_CON 0x0e
#define REG_STATUS 0x0f
#define REG_AGING 0x10
#define REG_TEMPM 0x11
#define REG_TEMPL 0x12

#define SEC_1970_TO_2000 946684800

#define MONDAY 1
#define TUESDAY 2
#define WEDNESDAY 3
#define THURSDAY 4
#define FRIDAY 5
#define SATURDAY 6
#define SUNDAY 7

class Time {
public:
  uint8_t hour;   //Hour
  uint8_t min;    //Minute
  uint8_t sec;    //Second
  uint8_t date;   //Day
  uint8_t mon;    //Month
  uint16_t year;  //Year
  uint8_t dow;    //Day of week
  Time();
};
class DS3231 {
public:

  DS3231();

  Time getTime();
  void begin();
  uint8_t Address(uint8_t *Address);
  void setTime(uint8_t hour, uint8_t min, uint8_t sec);
  void setDate(uint8_t date, uint8_t mon, uint16_t year);
  void setDOW();
  void setOutput(byte enable);
  void enable32kHz(bool enable);
  float getTemp();
  void setSQWRate(int rate);
  void EnableOscillator(void);

private:
  uint8_t _readRegister(uint8_t reg);
  uint8_t _decode(uint8_t value);
  uint8_t _decodeH(uint8_t value);
  uint8_t _encode(uint8_t value);
  void _writeRegister(uint8_t reg, uint8_t value);
  void setDOW(uint8_t dow);
  void _sendStart(byte addr);
  void _sendStop();
  void _sendNack();
  void _sendAck();
  void _waitForAck();
  uint8_t _readByte();
  void _writeByte(uint8_t value);
  void _burstRead();

  uint8_t _sda_pin = PB11;
  uint8_t _scl_pin = PB10;
  uint8_t _burstArray[7];
};

#endif