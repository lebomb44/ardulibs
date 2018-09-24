#include <Arduino.h> 

#include "Relay.h"

Relay::Relay(String name_, uint8_t pin_) : _name(name_), _pin(pin_)
{
  pinMode(_pin, OUTPUT);
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

void Relay::run(bool forceHK)
{
  uint8_t newState = false;

  newState = isClose();
  if((newState != _state) || (true == forceHK))
  {
    Serial.println(_name + "_hk " + newState);
  }
  _state = newState;
}

void Relay::cmdGet(int arg_cnt, char **args)
{
  Serial.println(_name + "_get " + isClose());
}

void Relay::cmdSet(int arg_cnt, char **args)
{
  if(2 == arg_cnt)
  {
    if(1 == strtoul(args[1], NULL, 10)) { close(); } else { open(); }
  }
  Serial.println(_name + "_get " + isClose());
}

