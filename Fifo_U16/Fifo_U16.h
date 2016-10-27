#ifndef Fifo_U16_h
#define Fifo_U16_h

#include <inttypes.h>

#define FIFO_U16_SIZE 256

class Fifo_U16
{
public:
  Fifo_U16();
  void init(void);
  void push(uint16_t data);
  uint16_t pop(void);
  bool isEmpty(void);
  bool isFull(void);
  void purge(void);
private:
   volatile uint16_t fifo_buff[FIFO_U16_SIZE];
   volatile uint8_t fifo_in;
   volatile uint8_t fifo_out;
};

#endif
