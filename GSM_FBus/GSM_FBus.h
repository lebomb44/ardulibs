#ifndef GSM_FBus_h
#define GSMFBus_h

#include <inttypes.h>
#include <SoftwareSerial.h>

class GSM_FBus
{
public:
  GSM_FBus(const uint8_t txPin_, const uint8_t rxPin_);
  void init(void);
  uint8_t gsm_status_get(void);
  uint8_t gsm_version_get(void);
  uint8_t gsm_sms_send_with_tel(char * tel, char * msg);
  uint8_t gsm_sms_send(char * msg);
  char* gsm_sms_receive(void);
private:
  SoftwareSerial _serial;
  void fbus_sync(void);
  uint8_t fbus_header(uint8_t *frame, uint8_t cmd, uint16_t data_len);
  uint16_t fbus_frame_len(uint16_t data_len);
  uint8_t fbus_trailer(uint8_t * frame, uint8_t seq);
  uint8_t fbus_send(uint8_t cmd, uint16_t data_len, uint8_t *data, uint8_t seq);
  uint8_t fbus_receive(uint8_t *cmd, uint16_t *data_len, uint8_t **data, uint8_t *seq);
  uint8_t fbus_receive_ack(uint8_t _cmd);
};

#endif
