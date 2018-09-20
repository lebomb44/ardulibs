#ifndef ShutterButton_h
#define ShutterButton_h

#include <inttypes.h>
#include <Contact.h>
#include <Relay.h>

class ShutterButton
{
public:
  ShutterButton(String name_, uint8_t upButtonPin_, uint8_t downButtonPin_);
  void run(void);
private:
  String _name;
  Contact _upButton;
  Contact _downButton;
  volatile uint8_t _buttonState;

  uint8_t getButtonState(void);
};

#endif

