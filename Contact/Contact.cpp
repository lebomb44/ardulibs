#include <Arduino.h> 

#include "Contact.h"

Contact::Contact(String name_, uint8_t pin_) : _name(name_), _pin(pin_)
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
    Serial.println(_name + "_hk " + newState);
  }
  _state = newState;
}

void Contact::cmdGet(int arg_cnt, char **args)
{
  Serial.println(_name + "_get " + isClose());
}

