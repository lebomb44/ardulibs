#include <inttypes.h>
#include <Arduino.h> 

#include <util/crc16.h>
#include "LbMsg.h"

LbMsg::LbMsg(uint8_t dataLen): _frame(NULL) {
  init(dataLen);
}

LbMsg::~LbMsg(void) {
  if(NULL != getFrame()) { delete _frame; _frame = NULL; }
}

void LbMsg::init(uint8_t dataLen) {
  setDataLen(dataLen);
  setSrc(0);
  setDst(0);
  setCmd(0);
}

uint8_t LbMsg::getSrc(void) { return getFrame()[0]; }
void LbMsg::setSrc(uint8_t src) { getFrame()[0] = src; }
uint8_t LbMsg::getDst(void) { return getFrame()[1]; }
void LbMsg::setDst(uint8_t dst) { getFrame()[1] = dst; }
uint8_t LbMsg::getCmd(void) { return getFrame()[2]; }
void LbMsg::setCmd(uint8_t cmd) { getFrame()[2] = cmd; }
uint8_t LbMsg::getDataLen(void) { return getFrame()[3]; }
void LbMsg::setDataLen(uint8_t dataLen) {
  if(NULL != _frame) {
    if(getDataLen() < dataLen) {
      delete _frame; _frame = NULL;
    }
    _frame = new uint8_t[4+dataLen+1];
  }
  _frame[3] = dataLen;
}
uint8_t * LbMsg::getData(void) { return &(getFrame()[4]); }
uint8_t LbMsg::getCKS(void) { return _frame[getFrameLen()-1]; }
void LbMsg::setCKS(uint8_t cks) { _frame[getFrameLen()-1] = cks; }
uint16_t LbMsg::getFrameLen(void) { return 4+((uint16_t)getDataLen())+1; }
uint8_t * LbMsg::getFrame(void) { return _frame; }

void LbMsg::compute(void) {
  setCKS(0);
  for(uint16_t i=0; i<(getFrameLen()-1); i++) {
    setCKS(getCKS() + getFrame()[i]);
  }
}

bool LbMsg::check(void) {
  uint8_t cks = 0;
  for(uint16_t i=0; i<(getFrameLen()-1); i++) {
    cks = cks + getFrame()[i];
  }
  if(getCKS() == cks) { return true; }
  else { return false; }
}

void LbMsg::print(void) {
  for(uint16_t i=0; i<(getFrameLen()); i++) {
    Serial.print(getFrame()[i], HEX); Serial.print(" ");
  }
}

