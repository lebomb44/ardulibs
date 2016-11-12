#ifndef IR_h
#define IR_h

#include <inttypes.h>

class IR
{
public:
  IR();
  void init(void);
  void setSamsung(uint32_t code);
private:
  Fifo_U16 samsungManufacturer_fifo;
  Fifo_U08 samsungCode_fifo;
};

#endif
