#ifndef LB_NRF24_h
#define LB_NRF24_h

#include <inttypes.h>

#define LB_NRF24_DATA_MAX_SIZE (4+255+1)

class LB_NRF24
{
public:
  LB_NRF24();
  void init(void);
  void run(void);
  bool rxIsReady(void);
  uint8_t rxGetSrc(void);
  uint8_t rxGetDst(void);
  uint8_t rxGetCmd(void);
  uint8_t rxGetLen(void);
  uint8_t * rxGetData(void);
  void rxRelease(void);
  bool txIsReady(void);
  void send(uint8_t src, uint8_t dst, uint8_t cmd, uint8_t len, uint8_t * data);
private:
   uint8_t rawData[LB_NRF24_DATA_MAX_SIZE];
};

#endif
