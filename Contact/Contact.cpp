#include <Arduino.h> 

#include "Contact.h"

Contact::Contact(uint8_t pin_) : _pin(pin_)
{
  pinMode(_pin, INPUT);
}

bool Contact::isOpen(void)
{
  return digitalRead(_pin);
}

bool Contact::isClose(void)
{
  return !isOpen();
}

