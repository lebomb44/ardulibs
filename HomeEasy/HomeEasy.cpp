#include <inttypes.h>
#include <Arduino.h> 

#include "wiring_private.h"
#include "HomeEasy.h"

Fifo_U16 * extInt0_Fifo = NULL;

HomeEasy::HomeEasy()
{
  this->init();
}

void HomeEasy::init(void)
{
  uint8_t i = 0;

  this->rx_fifo.init();
  this->code = 0;
  for(i=0; i<64; i++) { this->codeBitStream[i] = false; }
  this->step = 0;
  disablePrint();

  extInt0_Fifo = &(this->rx_fifo);

  cbi(TCCR1B, CS12); // Bit 2:0 � CS12:0: Clock Select : 000 = No clock source (Timer/Counter stopped).
  cbi(TCCR1B, CS11);
  cbi(TCCR1B, CS10);

  cbi(TCCR1A, COM1A1); // Bit 7:6 � COM1A1:0: Compare Output Mode for Channel A : Normal port operation, OC1A disconnected.
  cbi(TCCR1A, COM1A0);
  cbi(TCCR1A, COM1B1); // Bit 5:4 � COM1B1:0: Compare Output Mode for Channel B : Normal port operation, OC1B disconnected.
  cbi(TCCR1A, COM1B0);
  // Bit 3 : reserved
  // Bit 2 : reserved
  cbi(TCCR1A, WGM11); // Bit 1:0 � WGM11:0: Waveform Generation Mode : 0 = 0000 = Normal
  cbi(TCCR1A, WGM10);

  cbi(TCCR1B, ICNC1); // Bit 7 � ICNC1: Input Capture Noise Canceler : Disabled
  cbi(TCCR1B, ICES1); // Bit 6 � ICES1: Input Capture Edge Select : Disabled
  // Bit 5 : reserved
  cbi(TCCR1B, WGM13); // Bit 4:3 � WGM13:2: Waveform Generation Mode : 0 = 0000 = Normal
  cbi(TCCR1B, WGM12);

  cbi(TCCR1C, FOC1A); // Bit 7 � FOC1A: Force Output Compare for Channel A
  cbi(TCCR1C, FOC1A); // Bit 6 � FOC1B: Force Output Compare for Channel B
  // Bit 5 : reserved
  // Bit 4 : reserved
  // Bit 3 : reserved
  // Bit 2 : reserved
  // Bit 1 : reserved
  // Bit 0 : reserved

  TCNT1 = 0x0000;
  OCR1A = 0x0000;
  OCR1B = 0x0000;
  ICR1 = 0x0000;

  // Bit 7 : reserved
  // Bit 6 : reserved
  cbi(TIMSK1, ICIE1); // Bit 5 � ICIE1: Timer/Counter1, Input Capture Interrupt Enable : Disabled
  // Bit 4 : reserved
  // Bit 3 : reserved
  cbi(TIMSK1, OCIE1B); // Bit 2 � OCIE1B: Timer/Counter1, Output Compare B Match Interrupt Enable : Disabled
  cbi(TIMSK1, OCIE1A); // Bit 1 � OCIE1A: Timer/Counter1, Output Compare A Match Interrupt Enable : Disabled
  cbi(TIMSK1,TOIE1); // Bit 0 � TOIE1: Timer/Counter1, Overflow Interrupt Enable : Disabled

  // Bit 7 : reserved
  // Bit 6 : reserved
  cbi(TIFR1, ICF1); // Bit 5 � ICF1: Timer/Counter1, Input Capture Flag
  // Bit 4 : reserved
  // Bit 3 : reserved
  cbi(TIFR1, OCF1B); // Bit 2 � OCF1B: Timer/Counter1, Output Compare B Match Flag
  cbi(TIFR1, OCF1A); // Bit 1 � OCF1A: Timer/Counter1, Output Compare A Match Flag
  cbi(TIFR1, TOV1); // Bit 0 � TOV1: Timer/Counter1, Overflow Flag

  cbi(TCCR1B, CS12); // Bit 2:0 � CS12:0: Clock Select : 010 = clkI/O/8 (From prescaler)
  sbi(TCCR1B, CS11);
  cbi(TCCR1B, CS10);

  // Bit 7 : reserved
  // Bit 6 : reserved
  // Bit 5 : reserved
  // Bit 4 : reserved
  cbi(EICRA, ISC11); // Bit 3, 2 � ISC11, ISC10: Interrupt Sense Control 1 Bit 1 and Bit 0 : 00 = The low level of INT1 generates an interrupt request.
  cbi(EICRA, ISC10);
  cbi(EICRA, ISC01); // Bit 1, 0 � ISC01, ISC00: Interrupt Sense Control 0 Bit 1 and Bit 0 : 01 = Any logical change on INT0 generates an interrupt request.
  sbi(EICRA, ISC00);

  // Bit 7 : reserved
  // Bit 6 : reserved
  // Bit 5 : reserved
  // Bit 4 : reserved
  // Bit 3 : reserved
  // Bit 2 : reserved
  cbi(EIMSK, INT1); // Bit 1 � INT1: External Interrupt Request 1 Enable : Disabled
  sbi(EIMSK, INT0); // Bit 0 � INT0: External Interrupt Request 0 Enable : Enabled
}

void HomeEasy::run(void)
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
        /* Serial.print(dataU16); Serial.print(": "); Serial.println(this->step); */
        if(0 == (this->step%2))
        {
          if(this->isHigh(dataU16)) { this->step++; }
          else
          {
            /* Serial.print(dataU16); Serial.print(": Bad high at step "); Serial.println(this->step); */
            this->step = 0;
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
              /* Serial.print(dataU16); Serial.print(": Bad low at step "); Serial.println(this->step); */
              this->step = 0;
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
                /* Serial.print("Bad sequence-");Serial.print(cBit);Serial.print("=");Serial.print(codeBitStream[2*cBit]);Serial.println(codeBitStream[(2*cBit) +1]); */
                this->rxRelease();
              }
            }
          }
        }
      }
      if(1 == this->step)
      {
        if(this->isLowSync(dataU16)) { this->step++; }
        else
        {
          /* Serial.print(dataU16); Serial.print(": Bad lowSync at step "); Serial.println(this->step); */
          this->step = 0;
        }
      }
      if(0 == this->step) { if(this->isHigh(dataU16)) { this->step++; } }
    }
  }
}

bool HomeEasy::rxCodeIsReady(void)
{
  if(130 == this->step) { return true; }
  else { return false; }
}

uint32_t HomeEasy::rxGetCode(void)
{
  return (this->code);
}

uint8_t HomeEasy::rxGetDevice(void)
{
  return (0x0000000F & this->code);
}

bool HomeEasy::rxGetStatus(void)
{
  return (0x00000010 & this->code) >> 4;
}

bool HomeEasy::rxGetGroup(void)
{
  return (0x00000020 & this->code) >> 5;
}

uint32_t HomeEasy::rxGetManufacturer(void)
{
  return (0xFFFFFFC0 & this->code) >> 6;
}

void HomeEasy::rxRelease(void)
{
  uint8_t i = 0;

  this->code = 0;
  for(i=0; i<64; i++) { this->codeBitStream[i] = false; }
  this->step = 0;
}

bool HomeEasy::txIsReady(void)
{
  /* FIXME : Not implemented */
  return true;
}

void HomeEasy::send(uint32_t code)
{
  /* FIXME : Not implemented */
}

void HomeEasy::purge(void)
{
  rx_fifo.purge();
}

void HomeEasy::enablePrint(void)
{
  _printIsEnabled = true;
}

void HomeEasy::disablePrint(void)
{
  _printIsEnabled = false;
}

bool HomeEasy::printIsEnabled(void)
{
  return _printIsEnabled;
}

/******************** PRIVATE ***************************/

bool HomeEasy::isHigh(uint16_t timeU16)
{
  //if((495 < timeU16) && (timeU16 < 640)) { return true; }
  if((200 < timeU16) && (timeU16 < 900)) { return true; }
  else { return false; }
}

bool HomeEasy::isLowShort(uint16_t timeU16)
{
  if((200 < timeU16) && (timeU16 < 900)) { return true; }
  else { return false; }
}

bool HomeEasy::isLowLong(uint16_t timeU16)
{
  //if((2340 < timeU16) && (timeU16 < 2860)) { return true; }
  if((2340 < timeU16) && (timeU16 < 3000)) { return true; }
  else { return false; }
}

bool HomeEasy::isLowSync(uint16_t timeU16)
{
  //if((4815 < timeU16) && (timeU16 < 5885)) { return true; }
  if((3000 < timeU16) && (timeU16 < 7000)) { return true; }
  else { return false; }
}

ISR(INT0_vect)
{
  uint16_t dataU16 = 0;

  dataU16 = TCNT1;
  TCNT1 = 0x0000;

  if((200 < dataU16) && (dataU16 < 7000))
  {
    if(false == extInt0_Fifo->isFull())
    {
    	extInt0_Fifo->push(dataU16);
    }
  }
  /* else { Serial.println(dataU16); } */
}
