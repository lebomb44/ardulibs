#ifndef LB_Com_h
#define LB_Com_h

#include <inttypes.h>
#include <Fifo_U08.h>

#define LB_COM_DATA_MAX_SIZE (4+255)

class LB_Com
{
public:
  LB_Com();
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
  Fifo_U08 rx_fifo;
  uint8_t rxData[LB_COM_DATA_MAX_SIZE];
  uint16_t rx_step;
  Fifo_U08 tx_fifo;
  void send_char(uint8_t data);
};

#endif
