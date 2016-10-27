#ifndef Fifo_U08_h
#define Fifo_U08_h

#include <inttypes.h>

#define FIFO_U08_SIZE 256

class Fifo_U08
{
public:
  Fifo_U08();
  void init(void);
  void push(uint8_t data);
  uint8_t pop(void);
  bool isEmpty(void);
  bool isFull(void);
  void purge(void);
private:
   volatile uint8_t fifo_buff[FIFO_U08_SIZE];
   volatile uint8_t fifo_in;
   volatile uint8_t fifo_out;
};

#endif
