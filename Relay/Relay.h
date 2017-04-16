#ifndef Relay_h
#define Relay_h

#include <inttypes.h>

class Relay
{
public:
  Relay(uint8_t pin_);
  bool isClose(void);
  bool isOpen(void);
  void close(void);
  void open(void);
private:
  volatile uint8_t _pin;
  volatile bool _state;
};

#endif

