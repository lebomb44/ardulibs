#ifndef LbCom_h
#define LbCom_h

#include <inttypes.h>
#include <Fifo_U08.h>
#include <LbMsg.h>

class LbCom
{
public:
  LbCom();
  void init(void);
  void run(void);
  bool rxMsgIsReady(void);
  uint8_t * rxGetFrame(void);
  void rxReleaseMsg(void);
  void rxGetMsg(LbMsg & msg);
  void send(LbMsg & msg);
  void enablePrint(void);
  void disablePrint(void);
private:
  Fifo_U08 rx_fifo;
  LbMsg _rx_msg;
  uint16_t _rx_step;
  void send_char(uint8_t data);
  bool printIsEnabled;
};

#endif

