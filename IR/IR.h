#ifndef IR_h
#define IR_h

#include <inttypes.h>
#include <Fifo_U16.h>

class IR
{
public:
  IR();
  void init(void);
  void run(void);
  bool rxSamsungCodeIsReady(void);
  uint32_t rxGetSamsungCode(void);
  uint16_t rxGetSamsungData(void);
  uint16_t rxGetSamsungManufacturer(void);
  void rxSamsungRelease(void);
  void purge(void);
  void enablePrint(void);
  void disablePrint(void);
  bool printIsEnabled(void);
private:
  Fifo_U16 rx_fifo;
  uint32_t samsungCode;
  uint16_t samsungStep;
  bool _printIsEnabled;
  bool isSamsungHigh(uint16_t timeU16);
  bool isSamsungLowShort(uint16_t timeU16);
  bool isSamsungLowLong(uint16_t timeU16);
  bool isSamsungSync1(uint16_t timeU16);
  bool isSamsungSync2(uint16_t timeU16);
};

#endif
