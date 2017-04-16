#include <Arduino.h> 

#include "Shutter.h"

#define SHUTTER_STOP 0
#define SHUTTER_UP   1
#define SHUTTER_DOWN 2

Shutter::Shutter(uint8_t upButtonPin_, uint8_t downButtonPin_, uint8_t upRelayPin_, uint8_t downRelayPin_) : _upButton(upButtonPin_), _downButton(downButtonPin_), _upRelay(upRelayPin_), _downRelay(downRelayPin_)
{
  stop();
  _buttonState = getButtonState();
  alarmDisable();
  setRelayState();
}

void Shutter::up(void)
{
  _state = SHUTTER_UP;
}

void Shutter::down(void)
{
  _state = SHUTTER_DOWN;
}

void Shutter::stop(void)
{
  _state = SHUTTER_STOP;
}

uint8_t Shutter::getButtonState(void)
{
  if(_upButton.isClose() && _downButton.isOpen())
  {
    return SHUTTER_UP;
  }
  else
  {
    if(_upButton.isOpen() && _downButton.isClose())
    {
      return SHUTTER_DOWN;
    }
    else
    {
      return SHUTTER_STOP;
    }
  }
}

void Shutter::setRelayState(void)
{
  if(_upRelay.isClose() && _downRelay.isClose())
  {
    _upRelay.open(); _downRelay.open(); return;
  }
  if(SHUTTER_UP == _state)
  {
    _upRelay.close(); _downRelay.open(); return;
  }
  if(SHUTTER_DOWN == _state)
  {
    _upRelay.open(); _downRelay.close(); return;
  }

  _upRelay.open(); _downRelay.open(); return;
}

void Shutter::alarmEnable(void)
{
  _alarmState = true;
}

void Shutter::alarmDisable(void)
{
  _alarmState = false;
}

bool Shutter::alarm_isEnabled(void)
{
  return _alarmState;
}

void Shutter::run(void)
{
  uint8_t newButtonState = SHUTTER_STOP;

  newButtonState = getButtonState();
  if(alarm_isEnabled())
  {
    _state = SHUTTER_DOWN;
  }
  else
  {
    if(newButtonState != _buttonState)
    {
      _state = newButtonState;
    }
  }
  _buttonState = newButtonState;
  setRelayState();
}

