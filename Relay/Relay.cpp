#include <Arduino.h> 

#include "Relay.h"

Relay::Relay(uint8_t pin_) : _pin(pin_)
{
  pinMode(_pin, INPUT);
  open();
}

bool Relay::isClose(void)
{
  return _state;
}

bool Relay::isOpen(void)
{
  return !isClose();
}

void Relay::close(void)
{
  digitalWrite(_pin, HIGH);
  _state = true;
}

void Relay::open(void)
{
  digitalWrite(_pin, LOW);
  _state = false;
}

