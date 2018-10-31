#include <Arduino.h> 
#include <CnC.h>
#include "ShutterButton.h"

#define SHUTTER_BUTTON_STOP 0
#define SHUTTER_BUTTON_UP   1
#define SHUTTER_BUTTON_DOWN 2

ShutterButton::ShutterButton(const char * name_, uint8_t upButtonPin_, uint8_t downButtonPin_) : _name(name_), _upButton(name_, upButtonPin_), _downButton(name_, downButtonPin_)
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

void ShutterButton::run(bool forceHK)
{
  uint8_t newButtonState = SHUTTER_BUTTON_STOP;

  newButtonState = getButtonState();
  if((newButtonState != _buttonState) || (true == forceHK))
  {
    cnc_print_hk_u32(_name, newButtonState);
  }
  _buttonState = newButtonState;
}

void ShutterButton::cmdGet(int arg_cnt, char **args)
{
  cnc_print_cmdGet_u32(_name, getButtonState());
}

