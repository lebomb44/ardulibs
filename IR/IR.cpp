#include <inttypes.h>
#include <Arduino.h> 

#include "wiring_private.h"
#include "IR.h"

#define IR_IN_pin 2

Fifo_U16 * IR_extInt0_Fifo = NULL;

IR::IR()
{
  this->init();
}

void IR::init(void)
{
  uint8_t i = 0;

  pinMode(IR_IN_pin, INPUT_PULLUP);
  this->rx_fifo.init();
  this->samsungCode = 0;
  this->samsungStep = 0;
  disablePrint();

  IR_extInt0_Fifo = &(this->rx_fifo);

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

void IR::run(void)
{
  uint16_t dataU16 = 0;
  uint8_t cBit = 0;

  if(false == this->rxSamsungCodeIsReady())
  {
    if(false == this->rx_fifo.isEmpty())
    {
      dataU16 = this->rx_fifo.pop();
      //Serial.println(dataU16);
      if((1 < this->samsungStep) && (this->samsungStep < 65))
      {
        /* Serial.print(dataU16); Serial.print(": "); Serial.println(this->samsungStep); */
        if(1 == (this->samsungStep%2))
        {
          if(this->isSamsungLowShort(dataU16)) { bitClear(this->samsungCode, (this->samsungStep-2)/2); this->samsungStep++; }
          else
          {
            if(this->isSamsungLowLong(dataU16)) { bitSet(this->samsungCode, (this->samsungStep-2)/2); this->samsungStep++; }
            else
            {
              //Serial.print(dataU16); Serial.print(": Bad Samsung low at step "); Serial.println(this->samsungStep);
              this->rxSamsungRelease();
            }
          }
        }
        else
        {
          if(this->isSamsungHigh(dataU16)) { this->samsungStep++; }
          else
          {
            //Serial.print(dataU16); Serial.print(": Bad Samsung high at step "); Serial.println(this->samsungStep);
            this->rxSamsungRelease();
          }
        }
      }
      if(1 == this->samsungStep)
      {
        if(this->isSamsungSync2(dataU16)) { this->samsungStep++; /*Serial.print(dataU16); Serial.println(": isSamsungSync");*/ }
        else
        {
          //Serial.print(dataU16); Serial.print(": Bad Samsung Sync at step "); Serial.println(this->samsungStep);
          this->rxSamsungRelease();
        }
      }
      if(0 == this->samsungStep) { if(this->isSamsungSync1(dataU16)) { this->samsungStep++; } }
    }
  }
}

bool IR::rxSamsungCodeIsReady(void)
{
  if(65 == this->samsungStep) { return true; }
  else { return false; }
}

uint32_t IR::rxGetSamsungCode(void)
{
  return (this->samsungCode);
}

uint16_t IR::rxGetSamsungData(void)
{
  return (0xFFFF0000 & this->samsungCode) >> 16;
}

uint16_t IR::rxGetSamsungManufacturer(void)
{
  return (0x0000FFFF & this->samsungCode);
}

void IR::rxSamsungRelease(void)
{
  uint8_t i = 0;

  this->samsungCode = 0;
  this->samsungStep = 0;
}

void IR::purge(void)
{
  rx_fifo.purge();
}

void IR::enablePrint(void)
{
  _printIsEnabled = true;
}

void IR::disablePrint(void)
{
  _printIsEnabled = false;
}

bool IR::printIsEnabled(void)
{
  return _printIsEnabled;
}

/******************** PRIVATE ***************************/

bool IR::isSamsungHigh(uint16_t timeU16) /* 0.56 ms */
{
  if((130 < timeU16) && (timeU16 < 180)) { return true; }
  else { return false; }
}

bool IR::isSamsungLowShort(uint16_t timeU16) /* 0.56 ms */
{
  if((80 < timeU16) && (timeU16 < 110)) { return true; }
  else { return false; }
}

bool IR::isSamsungLowLong(uint16_t timeU16) /* 1.69 ms */
{
  if((110 < timeU16) && (timeU16 < 165)) { return true; }
  else { return false; }
}

bool IR::isSamsungSync1(uint16_t timeU16)
{
  if((120 < timeU16) && (timeU16 < 250)) { return true; }
  else { return false; }
}

bool IR::isSamsungSync2(uint16_t timeU16)
{
  if((70 < timeU16) && (timeU16 < 100)) { return true; }
  else { return false; }
}

ISR(INT0_vect)
{
  uint16_t dataU16 = 0;

  dataU16 = TCNT1;
  TCNT1 = 0x0000;

  if((80 < dataU16) && (dataU16 < 250))
  {
    if(false == IR_extInt0_Fifo->isFull())
    {
    	IR_extInt0_Fifo->push(dataU16);
    }
  }
  //else { Serial.println(dataU16); }
}

