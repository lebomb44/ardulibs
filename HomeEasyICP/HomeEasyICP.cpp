#include <inttypes.h>
#include <Arduino.h> 

#include "wiring_private.h"
#include "HomeEasyICP.h"
//#include "histogram.h"

Fifo_U16 * icp4_Fifo = NULL;

HomeEasyICP::HomeEasyICP()
{
  this->init();
}

void HomeEasyICP::init(void)
{
  uint8_t i = 0;

  this->rx_fifo.init();
  this->code = 0;
  for(i=0; i<64; i++) { this->codeBitStream[i] = false; }
  this->step = 0;
  disablePrint();
  _dataLowSyn = 0;
  _previousBitLen = 0;

  icp4_Fifo = &(this->rx_fifo);

  cbi(TCCR4B, CS42); // Bit 2:0 � CS42:0: Clock Select : 000 = No clock source (Timer/Counter stopped).
  cbi(TCCR4B, CS41);
  cbi(TCCR4B, CS40);

  cbi(TCCR4A, COM4A1); // Bit 7:6 � COM4A1:0: Compare Output Mode for Channel A : Normal port operation, OC4A disconnected.
  cbi(TCCR4A, COM4A0);
  cbi(TCCR4A, COM4B1); // Bit 5:4 � COM4B1:0: Compare Output Mode for Channel B : Normal port operation, OC4B disconnected.
  cbi(TCCR4A, COM4B0);
  cbi(TCCR4A, COM4C1); // Bit 3:2 � COM4C1:0: Compare Output Mode for Channel C : Normal port operation, OC4B disconnected.
  cbi(TCCR4A, COM4C0);
  // Bit 3 : reserved
  // Bit 2 : reserved
  cbi(TCCR4A, WGM41); // Bit 1:0 � WGM41:0: Waveform Generation Mode : 0 = 0000 = Normal
  cbi(TCCR4A, WGM40);

  sbi(TCCR4B, ICNC4); // Bit 7 � ICNC4: Input Capture Noise Canceler : Enabled
  sbi(TCCR4B, ICES4); // Bit 6 � ICES4: Input Capture Edge Select : Disabled
  // Bit 5 : reserved
  cbi(TCCR4B, WGM43); // Bit 4:3 � WGM43:2: Waveform Generation Mode : 0 = 0000 = Normal
  cbi(TCCR4B, WGM42);

  cbi(TCCR4C, FOC1A); // Bit 7 � FOC1A: Force Output Compare for Channel A
  cbi(TCCR4C, FOC1A); // Bit 6 � FOC1B: Force Output Compare for Channel B
  // Bit 5 : reserved
  // Bit 4 : reserved
  // Bit 3 : reserved
  // Bit 2 : reserved
  // Bit 1 : reserved
  // Bit 0 : reserved

  TCNT4 = 0x0000;
  OCR4A = 0x0000;
  OCR4B = 0x0000;
  OCR4C = 0x0000;
  ICR4 = 0x0000;

  // Bit 7 : reserved
  // Bit 6 : reserved
  sbi(TIMSK4, ICIE4); // Bit 5 � ICIE4: Timer/Counter4, Input Capture Interrupt Enable : Enabled
  // Bit 4 : reserved
  cbi(TIMSK4, OCIE4C); // Bit 3 � OCIE4C: Timer/Counter4, Output Compare C Match Interrupt Enable : Disabled
  cbi(TIMSK4, OCIE4B); // Bit 2 � OCIE4B: Timer/Counter4, Output Compare B Match Interrupt Enable : Disabled
  cbi(TIMSK4, OCIE4A); // Bit 1 � OCIE4A: Timer/Counter4, Output Compare A Match Interrupt Enable : Disabled
  cbi(TIMSK4, TOIE4); // Bit 0 � TOIE4: Timer/Counter4, Overflow Interrupt Enable : Disabled

  // Bit 7 : reserved
  // Bit 6 : reserved
  cbi(TIFR4, ICF4); // Bit 5 � ICF4: Timer/Counter4, Input Capture Flag
  // Bit 4 : reserved
  cbi(TIFR4, OCF4C); // Bit 3 � OCF4C: Timer/Counter4, Output Compare C Match Flag
  cbi(TIFR4, OCF4B); // Bit 2 � OCF4B: Timer/Counter4, Output Compare B Match Flag
  cbi(TIFR4, OCF4A); // Bit 1 � OCF4A: Timer/Counter4, Output Compare A Match Flag
  cbi(TIFR4, TOV4); // Bit 0 � TOV4: Timer/Counter4, Overflow Flag

  cbi(TCCR4B, CS42); // Bit 2:0 � CS42:0: Clock Select : 010 = clkI/O/8 (From prescaler)
  sbi(TCCR4B, CS41);
  cbi(TCCR4B, CS40);

  //histo_nb_interval(100);
  //histo_set_delta(100);
  //histo_init();
}

void HomeEasyICP::run(void)
{
  uint16_t dataU16 = 0;
  uint8_t cBit = 0;

  if(false == this->rxCodeIsReady())
  {
    if(false == this->rx_fifo.isEmpty())
    {
      dataU16 = this->rx_fifo.pop();
      //Serial.println(dataU16);
      if((1 < this->step) && (this->step < 130))
      {
        //histo_enter(dataU16);
        /* Serial.print(dataU16); Serial.print(": "); Serial.println(this->step); */
        if(0 == (this->step%2))
        {
          if(this->isHigh(dataU16)) { this->step++; }
          else
          {
            //if(80<this->step) {Serial.print(dataU16); Serial.print(": Bad high at step "); Serial.println(this->step); histo_dump("Bad High"); }
            this->step = 0;
            if(this->isHigh(this->_previousBitLen) && this->isLowSync(dataU16)) { this->step = 2; }
          }
        }
        else
        {
          if(this->isLowShort(dataU16)) { this->codeBitStream[(this->step-3)/2] = false; this->step++; }
          else
          {
            if(this->isLowLong(dataU16)) { this->codeBitStream[(this->step-3)/2] = true; this->step++; }
            else
            {
              //if(80<this->step) {Serial.print(dataU16); Serial.print(": Bad low at step "); Serial.println(this->step); histo_dump("Bad Low"); }
              this->step = 0;
              if(this->isHigh(this->_previousBitLen) && this->isLowSync(dataU16)) { this->step = 2; }
            }
          }
        }
        if(true == this->rxCodeIsReady())
        {
          for(cBit=0; cBit<32; cBit++)
          {
            /* Serial.print(chaconBitStream[2*cBit]); Serial.print(chaconBitStream[(2*cBit) + 1]); */
            if((false == this->codeBitStream[2*cBit]) && (true == this->codeBitStream[(2*cBit) + 1]))
            {
              bitClear(this->code, 31-cBit);
            }
            else
            {
              if((true == this->codeBitStream[2*cBit]) && (false == this->codeBitStream[(2*cBit) + 1]))
              {
                bitSet(this->code, 31-cBit);
              }
              else
              {
                //Serial.print("Bad sequence-");Serial.print(cBit);Serial.print("=");Serial.print(codeBitStream[2*cBit]);Serial.println(codeBitStream[(2*cBit) +1]);
                this->rxRelease();
              }
            }
          }
        }
      }
      if(1 == this->step)
      {
        if(this->isLowSync(dataU16)) { this->step++; /*Serial.print(dataU16); Serial.println(": isLowSync");*/ }
        else
        {
          //Serial.print(dataU16); Serial.print(": Bad lowSync at step "); Serial.println(this->step);
          this->step = 0;
          //histo_init();
        }
      }
      if(0 == this->step) { if(this->isHigh(dataU16)) { this->step++; } }
      // Save the current value for futur analysis
      _previousBitLen = dataU16;
    }
  }
}

bool HomeEasyICP::rxCodeIsReady(void)
{
  if(130 == this->step) { return true; }
  else { return false; }
}

uint32_t HomeEasyICP::rxGetCode(void)
{
  return (this->code);
}

uint8_t HomeEasyICP::rxGetDevice(void)
{
  return (0x0000000F & this->code);
}

bool HomeEasyICP::rxGetStatus(void)
{
  return (0x00000010 & this->code) >> 4;
}

bool HomeEasyICP::rxGetGroup(void)
{
  return (0x00000020 & this->code) >> 5;
}

uint32_t HomeEasyICP::rxGetManufacturer(void)
{
  return (0xFFFFFFC0 & this->code) >> 6;
}

void HomeEasyICP::rxRelease(void)
{
  uint8_t i = 0;

  this->code = 0;
  for(i=0; i<64; i++) { this->codeBitStream[i] = false; }
  this->step = 0;
}

bool HomeEasyICP::txIsReady(void)
{
  /* FIXME : Not implemented */
  return true;
}

void HomeEasyICP::send(uint32_t code)
{
  /* FIXME : Not implemented */
}

void HomeEasyICP::purge(void)
{
  rx_fifo.purge();
}

void HomeEasyICP::enablePrint(void)
{
  _printIsEnabled = true;
}

void HomeEasyICP::disablePrint(void)
{
  _printIsEnabled = false;
}

bool HomeEasyICP::printIsEnabled(void)
{
  return _printIsEnabled;
}

void HomeEasyICP::histoDump(void)
{
  //Serial.print("Low Sync="); Serial.println(_dataLowSyn);
  //histo_dump("HomeEasyICP");
}

void HomeEasyICP::histoInit(void)
{
  //histo_init();
}


/******************** PRIVATE ***************************/

bool HomeEasyICP::isHigh(uint16_t timeU16)
{
  if((200 < timeU16) && (timeU16 < 1500)) { return true; }
  else { return false; }
}

bool HomeEasyICP::isLowShort(uint16_t timeU16)
{
  if((200 < timeU16) && (timeU16 < 1500)) { return true; }
  else { return false; }
}

bool HomeEasyICP::isLowLong(uint16_t timeU16)
{
  if((1600 < timeU16) && (timeU16 < 4000)) { return true; }
  else { return false; }
}

bool HomeEasyICP::isLowSync(uint16_t timeU16)
{
  _dataLowSyn = timeU16;
  if((3000 < timeU16) && (timeU16 < 7000)) { return true; }
  else { return false; }
}

ISR(TIMER4_CAPT_vect)
{
  uint16_t dataU16 = 0;

  dataU16 = TCNT4;
  TCNT4 = 0x0000;
  TCCR4B =  TCCR4B ^ (1<<ICES4);

  if((200 < dataU16) && (dataU16 < 7000))
  {
    if(false == icp4_Fifo->isFull())
    {
    	icp4_Fifo->push(dataU16);
    }
  }
  //Serial.println(dataU16);
}

