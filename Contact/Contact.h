#ifndef Contact_h
#define Contact_h

#include <inttypes.h>

class Contact
{
public:
  Contact(const char * name_, uint8_t pin_);
  bool isOpen(void);
  bool isClose(void);
  void run(bool forceHK);
  void cmdGet(int arg_cnt, char **args);
private:
  const char * _name;
  volatile uint8_t _pin;
  volatile uint8_t _state;
};

#endif

