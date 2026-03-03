#include "DS3231.h"

TwoWire wire2(2, I2C_FAST_MODE);


Time::Time() {
  this->year = 2014;
  this->mon = 1;
  this->date = 1;
  this->hour = 0;
  this->min = 0;
  this->sec = 0;
  this->dow = 3;
}
DS3231::DS3231() {
}

void DS3231::begin() {
  wire2.begin();
  wire2.beginTransmission(DS3231_ADDR);
  wire2.write(0x0E);  //0x0E address
  wire2.write(0x00);  //0x00 byte
  wire2.write(0x00);  //0x0F address 0x00 byte
  wire2.endTransmission();
}
uint8_t DS3231::Address(uint8_t* Address) {
  uint8_t nDevices = 0, error = 0, addr = 1;
  for (addr; addr < 127; addr++) {
    wire2.beginTransmission(addr);
    error = wire2.endTransmission();
    // Serial.print(error);
    if (error == 0) {
      Address[nDevices++] = addr;
    } else if (error == 4) {
      return 0xFE;  //Bus error
    }
  }
  return nDevices;
}
void DS3231::setTime(uint8_t hour, uint8_t min, uint8_t sec) {
  if (((hour >= 0) && (hour < 24)) && ((min >= 0) && (min < 60)) && ((sec >= 0) && (sec < 60))) {
    _writeRegister(REG_SEC, _encode(sec));
    _writeRegister(REG_MIN, _encode(min));
    _writeRegister(REG_HOUR, _encode(hour));
  }
}
void DS3231::setDate(uint8_t date, uint8_t mon, uint16_t year) {
  if (((date > 0) && (date <= 31)) && ((mon > 0) && (mon <= 12)) && ((year > 2000) && (year <= 3000))) {
    year -= 2000;
    _writeRegister(REG_DATE, _encode(date));
    _writeRegister(REG_MON, _encode(mon));
    _writeRegister(REG_YEAR, _encode(year));
  }
}
uint8_t DS3231::_decode(uint8_t value) {
  uint8_t decoded = value & 0b01111111;
  decoded = (decoded & 0b00001111) + 10 * ((decoded & (0b00001111 << 4)) >> 4);
  return decoded;
}
uint8_t DS3231::_decodeH(uint8_t value) {
  if (value & 0b10000000)
    value = (value & 0b00001111) + (12 * ((value & 0b00100000) >> 5));
  else
    value = (value & 0b00001111) + (10 * ((value & 0b00110000) >> 4));
  return value;
}
uint8_t DS3231::_encode(uint8_t value) {
  uint8_t encoded = ((value / 10) << 4) + (value % 10);
  return encoded;
}
void DS3231::_writeRegister(uint8_t reg, uint8_t value) {
  wire2.beginTransmission(DS3231_ADDR);
  wire2.write(reg);
  wire2.write(value);
  wire2.endTransmission();
}
uint8_t DS3231::_readRegister(uint8_t reg) {
  wire2.beginTransmission(DS3231_ADDR);
  wire2.write(reg);
  wire2.endTransmission();
  wire2.requestFrom(DS3231_ADDR, 1);
  while (wire2.available() < 1)
    ;
  return wire2.read();
}
Time DS3231::getTime() {
  Time t;
  // _burstRead();
  t.sec = _decode(_readRegister(REG_SEC));
  t.min = _decode(_readRegister(REG_MIN));
  t.hour = _decodeH(_readRegister(REG_HOUR));
  t.dow = _decode(_readRegister(REG_DOW));
  t.date = _decode(_readRegister(REG_DATE));
  t.mon = _decode(_readRegister(REG_MON));
  t.year = _decode(_readRegister(REG_YEAR));
  return t;
}
void DS3231::setDOW(uint8_t dow) {
  if ((dow > 0) && (dow) > 8) {
    _writeRegister(REG_DOW, dow);
  }
}
void DS3231::setDOW() {
  int dow;
  byte mArr[12] = { 6, 2, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
  Time _t = getTime();

  dow = (_t.year % 100);
  dow = dow * 1.25;
  dow += _t.date;
  dow += mArr[_t.mon - 1];
  if (((_t.year % 4) == 0) && (_t.mon < 3))
    dow -= 1;
  while (dow > 7)
    dow -= 7;
  _writeRegister(REG_DOW, dow);
}
void DS3231::setOutput(byte enable) {
  uint8_t _reg = _readRegister(REG_CON);
  _reg &= ~(1 << 2);
  _reg |= (enable << 2);
  _writeRegister(REG_CON, _reg);
}
void DS3231::enable32kHz(bool enable) {
  uint8_t _reg = _readRegister(REG_STATUS);
  _reg &= ~(1 << 3);
  _reg |= (enable << 3);
  _writeRegister(REG_STATUS, _reg);
}
float DS3231::getTemp() {
  uint8_t _msb = _readRegister(REG_TEMPM);
  uint8_t _lsb = _readRegister(REG_TEMPL);
  return (float)_msb + ((_lsb >> 6) * 0.25f);
}
void DS3231::setSQWRate(int rate) {
  uint8_t _reg = _readRegister(REG_CON);
  _reg &= ~(3 << 3);
  _reg |= (rate << 3);
  _writeRegister(REG_CON, _reg);
}
void DS3231::EnableOscillator(void) {
  wire2.beginTransmission(DS3231_ADDR);
  wire2.write(REG_CON);
  wire2.write(0x00);
  wire2.write(0x00);
  wire2.endTransmission();
}
void DS3231::_sendStart(byte addr) {
  pinMode(_sda_pin, OUTPUT);
  digitalWrite(_sda_pin, HIGH);
  digitalWrite(_scl_pin, HIGH);
  digitalWrite(_sda_pin, LOW);
  digitalWrite(_scl_pin, LOW);
  shiftOut(_sda_pin, _scl_pin, MSBFIRST, addr);
}
void DS3231::_sendStop() {
  pinMode(_sda_pin, OUTPUT);
  digitalWrite(_sda_pin, LOW);
  digitalWrite(_scl_pin, HIGH);
  digitalWrite(_sda_pin, HIGH);
  pinMode(_sda_pin, INPUT);
}
void DS3231::_sendNack() {
  pinMode(_sda_pin, OUTPUT);
  digitalWrite(_scl_pin, LOW);
  digitalWrite(_sda_pin, HIGH);
  digitalWrite(_scl_pin, HIGH);
  digitalWrite(_scl_pin, LOW);
  pinMode(_sda_pin, INPUT);
}
void DS3231::_sendAck() {
  pinMode(_sda_pin, OUTPUT);
  digitalWrite(_scl_pin, LOW);
  digitalWrite(_sda_pin, LOW);
  digitalWrite(_scl_pin, HIGH);
  digitalWrite(_scl_pin, LOW);
  pinMode(_sda_pin, INPUT);
}
void DS3231::_waitForAck() {
  pinMode(_sda_pin, INPUT);
  digitalWrite(_scl_pin, HIGH);
  while (digitalRead(_sda_pin) == HIGH) {
  }
  digitalWrite(_scl_pin, LOW);
}
uint8_t DS3231::_readByte() {
  pinMode(_sda_pin, INPUT);
  uint8_t value = 0;
  uint8_t currentBit = 0;
  for (int i = 0; i < 8; ++i) {
    digitalWrite(_scl_pin, HIGH);
    currentBit = digitalRead(_sda_pin);
    value |= (currentBit << 7 - i);
    delayMicroseconds(1);
    digitalWrite(_scl_pin, LOW);
  }
  return value;
}
void DS3231::_writeByte(uint8_t value) {
  pinMode(_sda_pin, OUTPUT);
  shiftOut(_sda_pin, _scl_pin, MSBFIRST, value);
}
void DS3231::_burstRead() {
  _sendStart(DS3231_ADDR_W);
  _waitForAck();
  _writeByte(0);
  _waitForAck();
  _sendStart(DS3231_ADDR_R);
  _waitForAck();

  for (int i = 0; i < 7; i++) {
    _burstArray[i] = _readByte();
    if (i < 6)
      _sendAck();
    else
      _sendNack();
  }
  _sendStop();
}