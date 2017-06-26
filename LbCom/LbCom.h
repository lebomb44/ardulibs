#ifndef LbCom_h
#define LbCom_h

#include <inttypes.h>
#include <Fifo_U08.h>

#define LBCOM_FRAME_MAX_SIZE (4+255+1)

class LbCom
{
public:
  LbCom();
  void init(void);
  void run(void);
  bool rxIsReady(void);
  uint8_t rxGetSrc(void);
  uint8_t rxGetDst(void);
  uint8_t rxGetCmd(void);
  uint8_t rxGetDataLen(void);
  uint16_t rxGetFrameLen(void);
  uint8_t * rxGetData(void);
  uint8_t * rxGetFrame(void);
  void rxRelease(void);
  void send(uint8_t src, uint8_t dst, uint8_t cmd, uint8_t len, uint8_t * data);
  void enablePrint(void);
  void disablePrint(void);
private:
  Fifo_U08 rx_fifo;
  uint8_t * rxFrame;
  uint16_t rx_step;
  void send_char(uint8_t data);
  bool printIsEnabled;
};

#endif

