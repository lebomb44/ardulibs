#ifndef LbMsg_h
#define LbMsg_h

#include <inttypes.h>

#define LBMSG_DATA_MAX_SIZE (255)
#define LBMSG_FRAME_MAX_SIZE (4+LBMSG_DATA_MAX_SIZE+1)

class LbMsg
{
public:
  LbMsg(uint8_t);
  ~LbMsg(void);
  void init(uint8_t);
  uint8_t getSrc(void);
  void setSrc(uint8_t);
  uint8_t getDst(void);
  void setDst(uint8_t);
  uint8_t getCmd(void);
  void setCmd(uint8_t);
  uint8_t getDataLen(void);
  void setDataLen(uint8_t);
  uint8_t * getData(void);
  uint8_t getCRC(void);
  void setCRC(uint8_t);
  uint16_t getFrameLen(void);
  uint8_t * getFrame(void);
  void compute(void);
  bool check(void);
  void print(void);
private:
  uint8_t * _frame;
};

#endif

