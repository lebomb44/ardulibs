#include <Arduino.h> 

#include "GSM_FBus.h"

#define LOST_GSM_SMS_CENTER_FREE
#define LOST_GSM1 "0689350159"
#define LOST_GSM2 "0689350159"

#define NOKIA_3310_HWSWV   0xD1
#define NOKIA_3310_NETW    0x0A
#define NOKIA_3310_SMSSEND 0x02
#define NOKIA_3310_ACK     0x7F

GSM_FBus::GSM_FBus(const uint8_t txPin_, const uint8_t rxPin_) : _serial(txPin_, rxPin_)
{
  this->init();
}

void GSM_FBus::init(void)
{
  _serial.begin(115200);
  _serial.setTimeout(3000);
}

void GSM_FBus::fbus_sync(void)
{
  uint8_t i = 0;
  char sync_w = 0x55;

  for(i=0; i<128; i++) { _serial.write(sync_w); } _serial.flush();
}

uint8_t GSM_FBus::fbus_header(uint8_t *frame, uint8_t cmd, uint16_t data_len)
{
  /* Check the frame pointer */
  if(NULL == frame) { return 1; }
  /* Cable link */
  frame[0] = 0x1E;
  /* Destination device (cell phone) */
  frame[1] = 0x00;
  /* Source device (PC) */
  frame[2] = 0x0C;
  /* Command */
  frame[3] = cmd;
  /* Length (16 bits) = data length + sequence number */
  frame[4] = (data_len+1) / 256;
  frame[5] = (data_len+1) % 256;
  return 0;
}

uint16_t GSM_FBus::fbus_frame_len(uint16_t data_len)
{
  /* Header + data length + sequence number + 1 if length is odd + checksum */
  return (6 + data_len + 1 + ((data_len+1)%2) + 2);
}

uint8_t GSM_FBus::fbus_trailer(uint8_t * frame, uint8_t seq)
{
  uint16_t i = 0;
  uint16_t pos = 0;

  /* Check the frame pointer */
  if(NULL == frame) { return 1; }
  /* Get the length from the frame */
  pos = frame[4]; pos = pos << 8; pos += frame[5];
  /* Add header */
  pos = 5 + pos;
  /* Set the sequence number */
  frame[pos] = seq; pos++;
  /* Set the padding byte if necessary */
  if((pos%2)==1) { frame[pos]=0; pos++; }
  /* Compute the checksum on even data number */
  frame[pos] = 0; for(i=0; i<pos; i=i+2) { frame[pos] = frame[pos]^frame[i]; } pos++;
  /* Compute the checksum on odd data number */
  frame[pos] = 0; for(i=1; i<pos; i=i+2) { frame[pos] = frame[pos]^frame[i]; }

  return 0;
}

uint8_t GSM_FBus::fbus_send(uint8_t cmd, uint16_t data_len, uint8_t *data, uint8_t seq)
{
  uint8_t *frame = NULL;

  /* Check the data pointer to send */
  if(NULL == data) { return 1; }
  /* Allocate the frame buffer */
  frame = (uint8_t *) malloc(fbus_frame_len(data_len));
  /* Check the frame pointer */
  if(NULL == frame) { return 2; }
  /* Build the frame header */
  fbus_header(frame, cmd, data_len);
  /* Copy the data in the frame buffer */
  memcpy(&frame[6], data, data_len);
  /* Build the frame trailer */
  fbus_trailer(frame, seq);
  /* Send the frame to the cell phone */
  _serial.write(frame, fbus_frame_len(data_len)); _serial.flush();
  /* Free the frame buffer */
  free(frame);

  return 0;
}

uint8_t GSM_FBus::fbus_receive(uint8_t *cmd, uint16_t *data_len, uint8_t **data, uint8_t *seq)
{
  uint16_t buff_len = 0;
  uint16_t i = 0;
  uint8_t xor1 = 0;
  uint8_t xor2 = 0;
  int ret = 0;

  if(0x1E != _serial.read()) { Serial.print("ERROR header0\n"); return 1; }
  if(0x0C != _serial.read()) { Serial.print("ERROR header1\n"); return 2; }
  if(0x00 != _serial.read()) { Serial.print("ERROR header2\n"); return 3; }

  ret = _serial.read(); if(0>ret) { /*printf("ERROR data lenH\n");*/ return 4; }
  *cmd = ret;

  ret = _serial.read(); if(0>ret) { /*printf("ERROR data lenH\n");*/ return 5; }
  *data_len = (((ret)&0x00FF)<<8);
  ret = _serial.read(); if(0>ret) { /*printf("ERROR data lenL\n");*/ return 6; }
  *data_len = *data_len + ret;
  if(1>*data_len) { /*printf("ERROR data len\n");*/ return 7; }
  *data_len = (*data_len) - 1;

  buff_len = (*data_len) + ((*data_len)+1)%2 + 1 + 2;
  *data = (uint8_t *) malloc(buff_len);
  if(NULL == *data) { /*printf("ERROR malloc\n");*/ return 8; }

  for(i=0; i<buff_len; i++)
  {
    ret = _serial.read();
    if(0>ret) { /*printf("ERROR data\n");*/ free(*data); *data=NULL; return 9; }
    else { (*data)[i] = ret; }
  }

  *seq = (*data)[*data_len];

  /* Compute the checksum on even data number */
  xor1 = 0x1E^0x00^((((*data_len)+1)>>8)&0x00FF); for(i=0; i<(buff_len-2); i=i+2) { xor1 = xor1^(*data)[i]; }
  if(xor1 != (*data)[buff_len-2]) { /*printf("ERROR xor1\n");*/ free(*data); *data=NULL; return 10; }
  /* Compute the checksum on odd data number */
  xor2 = 0x0C^(*cmd)^(((*data_len)+1)&0x00FF); for(i=1; i<(buff_len-2); i=i+2) { xor2 = xor2^(*data)[i]; }
  if(xor2 != (*data)[buff_len-1]) { /*printf("ERROR xor2\n");*/ free(*data); *data=NULL; return 11; }

  return 0;
}

uint8_t GSM_FBus::fbus_receive_ack(uint8_t _cmd)
{
  uint8_t cmd = 0;
  uint16_t data_len = 0;
  uint8_t *data = NULL;
  uint8_t seq = 0;

  if(0 != fbus_receive(&cmd, &data_len, &data, &seq)) { /*printf("ERROR receive\n");*/ return 1; }
  if(NOKIA_3310_ACK != cmd) { free(data); /*printf("ERROR ack\n");*/ return 2; }
  if(1 != data_len) { free(data); /*printf("ERROR ack len\n");*/ return 3; }
  if(data[0] != _cmd) { free(data); /*printf("ERROR ack cmd\n");*/ return 4; }
//if(seq != 0x00) { free(data); /*printf("ERROR ack seq\n");*/ return 5; }
  free(data);
  return 0;
}

/*
uchar getver[] =   {0xd1,0x05,0x00,0x01,0x00,0x03,0x00};
uchar getsmsc[]=   {0x02,0x06,0x00,0x01,0x00,0x33,0x64,0x01};
uchar getimei[]=   {0x1b,0x04,0x00,0x01,0x00,0x01};
uchar getbatt[]=   {0x17,0x04,0x00,0x01,0x00,0x02};
uchar getrf[]  =   {0x0a,0x04,0x00,0x01,0x00,0x81};
uchar getnetwork[]={0x0a,0x04,0x00,0x01,0x00,0x70};
*/

uint8_t GSM_FBus::gsm_status_get(void)
{
  uint8_t data[5] = {0x00, 0x01, 0x00, 0x70, 0x01};
  uint8_t cmd = 0;
  uint16_t data_len = 0;
  uint8_t * data_recv = NULL;
  uint8_t seq = 0;
  uint8_t ret = 0;

  /* Purge the receiving buffer of all previous data */
  _serial.begin(115200);
  /* Synchronize the phone for the start of FBUS exchange */
  fbus_sync();

  ret = fbus_send(NOKIA_3310_NETW, 5, data, 0x60);
  if(0 != ret) { return (10+ret); }
// 1E 00 0C 0A 00 06 00 01 00 70 01 60 13 1D

  ret = fbus_receive_ack(NOKIA_3310_NETW);
  if(0 != ret) { return (20+ret); }
// 1E 0C 00 7F 00 02 0A 00 14 71

  ret = fbus_receive(&cmd, &data_len, &data_recv, &seq);
  if(0 != ret) { return (30+ret); }
// 0x0071, ?,?,?,length,netstatus,netsel,cellIDH,cellIDL,lacH,lacL,netcode,netcode,netcode
// 1E 0C 00 0A 00 15 01 08 00 71 01 00 01 0B 01 02 0C 97 1D 4D 02 F8 01 00 00 01 47 00 4B 40
  if(NULL == data_recv) { return 1; }
  /* Check the received data */
  ret = 0;
  if(NOKIA_3310_NETW != cmd) { ret = 2; }
  if(20 != data_len)         { ret = 3; }
  if((0x01 != data_recv[8]) && (0x02 != data_recv[8]))   { ret = 4; } // Home Network
  if(0x02 != data_recv[9])   { ret = 5; } // Automatic Network selection

  free(data_recv);
  if(0 != ret) { return ret; }

  data[0] = NOKIA_3310_NETW;
  ret = fbus_send(NOKIA_3310_ACK, 1, data, 0x01);
  if(0 != ret) { return (40+ret); }

  return 0;
}

uint8_t GSM_FBus::gsm_version_get(void)
{
  uint8_t data[6] = {0, 1, 0, 3, 0, 1};
  uint8_t cmd = 0;
  uint16_t data_len = 0;
  uint8_t * data_recv = NULL;
  uint8_t seq = 0;

  /* Purge the receiving buffer of all previous data */
  _serial.begin(115200);
  /* Synchronize the phone for the start of FBUS exchange */
  fbus_sync();

  if(0 != fbus_send(NOKIA_3310_HWSWV, 6, data, 0x60)) { return 1; }
// 1E 00 0C D1 00 07 00 01 00 03 00 01 60 00 72 D5

  if(0 != fbus_receive_ack(NOKIA_3310_HWSWV)) { return 2; }
// 1E 0C 00 7F 00 02 D1 00 CF 71

  if(0 != fbus_receive(&cmd, &data_len, &data_recv, &seq)) { return 3; }
// NOKIA 3310 : 1E 0C 00 D2 00 26 01 00 00 03 56 20 30 34 2E 33 35 0A 32 39 2D 30 33 2D 30 31 0A 4E 48 4D 2D 35 0A 28 63 29 20 4E 4D 50 2E 00 01 42 3A A8
// NOKIA 3330 : 1E 0C 00 D2 00 26 01 00 00 03 56 20 30 34 2E 35 30 0A 31 32 2D 31 30 2D 30 31 0A 4E 48 4D 2D 36 0A 28 63 29 20 4E 4D 50 2E 00 01 XX XX XX
  if(NULL != data_recv)
  {
    free(data_recv);
    if((NOKIA_3310_HWSWV+1) != cmd) { return 4; }
    if(37 != data_len) { return 5; }
  }
  else { return 6; }

  data[0] = NOKIA_3310_HWSWV;
  if(0 != fbus_send(NOKIA_3310_ACK, 1, data, 0x01)) { return 7; }
// 1E 00 0C 7F 00 02 D2 01 C0 7C

  return 0;
}

uint8_t GSM_FBus::gsm_sms_send_with_tel(char * tel, char * msg)
{
  uint16_t up_msg_len = 0;
  uint16_t p_msg_len = 0;
  uint8_t *frame = NULL;
  unsigned int i = 0;
  unsigned int j = 0;
  uint16_t pos = 0;

  uint8_t sms_header[8] = {0x00, 0x01, 0x00, 0x01, 0x02, 0x00, 0x07, 0x91};
#ifdef LOST_GSM_SMS_CENTER_SFR
  uint8_t sms_tel_center[10] = {0x33, 0x06, 0x09, 0x10, 0x93, 0xF0, 0x00, 0x00, 0x00, 0x00};
#endif
#ifdef LOST_GSM_SMS_CENTER_FREE
  uint8_t sms_tel_center[10] = {0x33, 0x96, 0x05, 0x00, 0x96, 0xF5, 0x00, 0x00, 0x00, 0x00};
#endif
#ifdef LOST_GSM_SMS_CENTER_ORANGE
  uint8_t sms_tel_center[10] = {0x33, 0x86, 0x09, 0x40, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00};
#endif
  /* SMS Center
   * SFR    : +33 6 09 00 13 90
   * FREE   : +33 6 95 00 06 95
   * ORANGE : +33 6 89 00 40 00
   */
  uint8_t sms_body[7] = {0x15, 0x00, 0x00, 0x00, 0x33, 0x0A, 0xA1};
  uint8_t sms_trailer[7] = {0xA7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//uint8_t sms_msg[46] = {0xC8, 0x34, 0x28, 0xC8, 0x66, 0xBB, 0x40, 0x54, 0x74, 0x7A, 0x0E, 0x6A, 0x97, 0xE7, 0xF3, 0xF0, 0xB9, 0x0C, 0xBA, 0x87, 0xE7, 0xA0, 0x79, 0xD9, 0x4D, 0x07, 0xD1, 0xD1, 0xF2, 0x77, 0xFD, 0x8C, 0x06, 0x19, 0x5B, 0xC2, 0xFA, 0xDC, 0x05, 0x1A, 0xBE, 0xDF, 0xEC, 0x50, 0x08, 0x01};
// Hi All. This message was sent through F-Bus. Cool!!

  /* Check the phone number */
  if(10 != strnlen(tel, 10)) { /*printf ("TEL bad len\n");*/ return 1; }
  /* Get the message length */
  up_msg_len = strnlen(msg, 160);
  /* Force end of message */
  msg[up_msg_len] = '\0';
  /* Calculate packed message lenght */
  p_msg_len = (up_msg_len*7)/8 + 1;
  /* Alloc the frame buffer */
  frame = (uint8_t *) malloc(fbus_frame_len(8+10+7+10+7+p_msg_len+1));
  /* Check the frame pointer */
  if(NULL == frame) { return 2; }
  /* Build the frame header */
  fbus_header(frame, NOKIA_3310_SMSSEND, 8+10+7+10+7+p_msg_len+1);
  /* Build the data frame header */
  memcpy(&frame[6], &sms_header[0], 8);
  /* Build the SMS center phone number */
  memcpy(&frame[6+8], &sms_tel_center[0], 10);
  /* Build the data frame body */
  memcpy(&frame[6+8+10], &sms_body[0], 7);
  /* Overwrite the message lenght */
  frame[6+8+10+4] = up_msg_len;
  /* Build the destination phone number */
  j=0; for(i=0; i<5; i++) { frame[6+8+10+7+i] = ((tel[j]-'0')&0x0F) + (((tel[j+1]-'0')<<4)&0xF0); j=j+2; }
  /* Build the data frame trailer */
  memcpy(&frame[6+8+10+7+10], &sms_trailer[0], 7);

  /* Copy the data in the frame buffer */
  for(i=0; i<p_msg_len; i++)
  {
    frame[6+8+10+7+10+7+i] = 0;
    for(j=0; j<8; j++) { pos = i*8+j; frame[6+8+10+7+10+7+i] = frame[6+8+10+7+10+7+i] | ((msg[pos/7]>>(pos%7))<<j); }
  }
  frame[6+8+10+7+10+7+p_msg_len]=0x01;
  /* Build the frame trailer */
  fbus_trailer(frame, 0x60);

  /* Send the frame to the cell phone */
  _serial.begin(115200);
  fbus_sync();
  _serial.write(frame, fbus_frame_len(8+10+7+10+7+p_msg_len+1)); _serial.flush();
//fwrite(frame, fbus_frame_len(8+10+7+10+7+p_msg_len+1), 1, stdout); fflush(stdout);
  /* Free the frame buffer */
  free(frame);
// 1E 0C 00 7F 00 02 02 00 1C 71
// 1E 0C 00 02 00 09 01 08 00 02 64 95 00 01 40 00 3B 99
// 1E 00 0C 7F 00 02 02 04 10 79

  return 0;
}

uint8_t GSM_FBus::gsm_sms_send(char * msg)
{
  uint8_t ret1 = 0;
  uint8_t ret2 = 0;

  ret1 = gsm_sms_send_with_tel(LOST_GSM1, msg);
  delay(2000);
  ret2 = gsm_sms_send_with_tel(LOST_GSM2, msg);

  return (ret1 + ret2);
}

char* GSM_FBus::gsm_sms_receive(void)
{
  return NULL;
}

