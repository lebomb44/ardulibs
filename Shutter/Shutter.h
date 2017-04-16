#ifndef Shutter_h
#define Shutter_h

#include <inttypes.h>
#include <Contact.h>
#include <Relay.h>

class Shutter
{
public:
  Shutter(uint8_t upButtonPin_, uint8_t downButtonPin_, uint8_t upRelayPin_, uint8_t downRelayPin_);
  void up(void);
  void down(void);
  void stop(void);
  void alarmEnable(void);
  void alarmDisable(void);
  bool alarm_isEnabled(void);
  void run(void);
private:
  Contact _upButton;
  Contact _downButton;
  Relay _upRelay;
  Relay _downRelay;
  volatile uint8_t _state;
  volatile uint8_t _buttonState;
  volatile bool _alarmState;

  uint8_t getButtonState(void);
  void setRelayState(void);
};

#endif

