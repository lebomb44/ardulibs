#include <Arduino.h> 

#include "ShutterButton.h"

#define SHUTTER_BUTTON_STOP 0
#define SHUTTER_BUTTON_UP   1
#define SHUTTER_BUTTON_DOWN 2

ShutterButton::ShutterButton(String name_, uint8_t upButtonPin_, uint8_t downButtonPin_) : _name(name_), _upButton(upButtonPin_), _downButton(downButtonPin_)
{
  _buttonState = getButtonState();
}

uint8_t ShutterButton::getButtonState(void)
{
  if(_upButton.isClose() && _downButton.isOpen())
  {
    return SHUTTER_BUTTON_UP;
  }
  else
  {
    if(_upButton.isOpen() && _downButton.isClose())
    {
      return SHUTTER_BUTTON_DOWN;
    }
    else
    {
      return SHUTTER_BUTTON_STOP;
    }
  }
}

void ShutterButton::run(void)
{
  uint8_t newButtonState = SHUTTER_BUTTON_STOP;

  newButtonState = getButtonState();
  if(newButtonState != _buttonState)
  {
    Serial.println(_name + " " + newButtonState);
  }
  _buttonState = newButtonState;
}

