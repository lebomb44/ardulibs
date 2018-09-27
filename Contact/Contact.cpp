#include <Arduino.h> 
#include <CnC.h>
#include "Contact.h"

Contact::Contact(const char * name_, uint8_t pin_) : _name(name_), _pin(pin_)
{
  pinMode(_pin, INPUT);
  _state = isClose();
}

bool Contact::isOpen(void)
{
  return digitalRead(_pin);
}

bool Contact::isClose(void)
{
  return !isOpen();
}

void Contact::run(bool forceHK)
{
  uint8_t newState = false;

  newState = isClose();
  if((newState != _state) || (true == forceHK))
  {
    cnc_print_hk(_name, newState);
  }
  _state = newState;
}

void Contact::cmdGet(int arg_cnt, char **args)
{
  cnc_print_cmdGet(_name, isClose());
}

