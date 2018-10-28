#include <Arduino.h> 
#include <CnC.h>
#include "Relay.h"

Relay::Relay(const char * name_, uint8_t pin_) : _name(name_), _pin(pin_)
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
  bool newState = false;

  newState = isClose();
  if((newState != _state) || (true == forceHK))
  {
    cnc_print_hk_bool(_name, newState);
  }
  _state = newState;
}

void Relay::cmdGet(int arg_cnt, char **args)
{
  cnc_print_cmdGet_bool(_name, isClose());
}

void Relay::cmdSet(int arg_cnt, char **args)
{
  if(4 == arg_cnt)
  {
    if(1 == strtoul(args[3], NULL, 10)) { close(); } else { open(); }
  }
  cmdGet(arg_cnt, args);
}

