#ifndef Relay_h
#define Relay_h

#include <inttypes.h>

class Relay
{
public:
  Relay(String name_, uint8_t pin_);
  bool isClose(void);
  bool isOpen(void);
  void close(void);
  void open(void);
  void run(bool forceHK);
  void cmdGet(int arg_cnt, char **args);
  void cmdSet(int arg_cnt, char **args);
private:
  String _name;
  volatile uint8_t _pin;
  volatile bool _state;
};

#endif

