#ifndef HT12E_h
#define HT12E_h

#include <inttypes.h>
#include <Fifo_U16.h>

class HT12E
{
public:
  HT12E();
  void init(void);
  void run(void);
  bool rxCodeIsReady(void);
  uint32_t rxGetCode(void);
  uint16_t rxGetAddress(void);
  uint8_t rxGetData(void);
  void rxRelease(void);
  bool txIsReady(void);
  void send(uint32_t code);
  void purge(void);
private:
  Fifo_U16 rx_fifo;
  uint32_t code;
  bool codeBitStream[24];
  uint16_t step;

  bool isShort(uint16_t timeU16);
  bool isLong(uint16_t timeU16);
  bool isPilot(uint16_t timeU16);
};

#endif
