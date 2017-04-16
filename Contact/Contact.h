#ifndef Contact_h
#define Contact_h

#include <inttypes.h>

class Contact
{
public:
  Contact(uint8_t pin_);
  bool isOpen(void);
  bool isClose(void);
private:
   volatile uint8_t _pin;
};

#endif

