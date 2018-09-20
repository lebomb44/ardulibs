#include <inttypes.h>
#include <Arduino.h> 

#include "wiring_private.h"
#include "HT12E.h"

Fifo_U16 * ht12e_extInt_Fifo = NULL;

HT12E::HT12E()
{
  this->init();
}

void HT12E::init(void)
{
  uint8_t i = 0;

  this->rx_fifo.init();
  this->code = 0;
  for(i=0; i<24; i++) { this->codeBitStream[i] = false; }
  this->step = 0;

  ht12e_extInt_Fifo = &(this->rx_fifo);

  cbi(TCCR1B, CS12); // Bit 2:0 - CS12:0: Clock Select : 000 = No clock source (Timer/Counter stopped).
  cbi(TCCR1B, CS11);
  cbi(TCCR1B, CS10);

  cbi(TCCR1A, COM1A1); // Bit 7:6 - COM1A1:0: Compare Output Mode for Channel A : Normal port operation, OC1A disconnected.
  cbi(TCCR1A, COM1A0);
  cbi(TCCR1A, COM1B1); // Bit 5:4 - COM1B1:0: Compare Output Mode for Channel B : Normal port operation, OC1B disconnected.
  cbi(TCCR1A, COM1B0);
  // Bit 3 : reserved
  // Bit 2 : reserved
  cbi(TCCR1A, WGM11); // Bit 1:0 - WGM11:0: Waveform Generation Mode : 0 = 0000 = Normal
  cbi(TCCR1A, WGM10);

  cbi(TCCR1B, ICNC1); // Bit 7 - ICNC1: Input Capture Noise Canceler : Disabled
  cbi(TCCR1B, ICES1); // Bit 6 - ICES1: Input Capture Edge Select : Disabled
  // Bit 5 : reserved
  cbi(TCCR1B, WGM13); // Bit 4:3 - WGM13:2: Waveform Generation Mode : 0 = 0000 = Normal
  cbi(TCCR1B, WGM12);

  cbi(TCCR1C, FOC1A); // Bit 7 - FOC1A: Force Output Compare for Channel A
  cbi(TCCR1C, FOC1A); // Bit 6 - FOC1B: Force Output Compare for Channel B
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
  cbi(TIMSK1, ICIE1); // Bit 5 - ICIE1: Timer/Counter1, Input Capture Interrupt Enable : Disabled
  // Bit 4 : reserved
  // Bit 3 : reserved
  cbi(TIMSK1, OCIE1B); // Bit 2 - OCIE1B: Timer/Counter1, Output Compare B Match Interrupt Enable : Disabled
  cbi(TIMSK1, OCIE1A); // Bit 1 - OCIE1A: Timer/Counter1, Output Compare A Match Interrupt Enable : Disabled
  cbi(TIMSK1,TOIE1); // Bit 0 - TOIE1: Timer/Counter1, Overflow Interrupt Enable : Disabled

  // Bit 7 : reserved
  // Bit 6 : reserved
  cbi(TIFR1, ICF1); // Bit 5 - ICF1: Timer/Counter1, Input Capture Flag
  // Bit 4 : reserved
  // Bit 3 : reserved
  cbi(TIFR1, OCF1B); // Bit 2 - OCF1B: Timer/Counter1, Output Compare B Match Flag
  cbi(TIFR1, OCF1A); // Bit 1 - OCF1A: Timer/Counter1, Output Compare A Match Flag
  cbi(TIFR1, TOV1); // Bit 0 - TOV1: Timer/Counter1, Overflow Flag

  cbi(TCCR1B, CS12); // Bit 2:0 - CS12:0: Clock Select : 010 = clkI/O/8 (From prescaler)
  sbi(TCCR1B, CS11);
  cbi(TCCR1B, CS10);

  // Bit 7 : reserved
  // Bit 6 : reserved
  // Bit 5 : reserved
  // Bit 4 : reserved
  cbi(EICRA, ISC11); // Bit 3, 2 - ISC11, ISC10: Interrupt Sense Control 1 Bit 1 and Bit 0 : 01 = Any logical change on INT1 generates an interrupt request.
  sbi(EICRA, ISC10);
  //cbi(EICRA, ISC01); // Bit 1, 0 - ISC01, ISC00: Interrupt Sense Control 0 Bit 1 and Bit 0 : 00 = The low level of INT0 generates an interrupt request.
  //cbi(EICRA, ISC00);

  // Bit 7 : reserved
  // Bit 6 : reserved
  // Bit 5 : reserved
  // Bit 4 : reserved
  // Bit 3 : reserved
  // Bit 2 : reserved
  sbi(EIMSK, INT1); // Bit 1 - INT1: External Interrupt Request 1 Enable : Enabled
  //cbi(EIMSK, INT0); // Bit 0 - INT0: External Interrupt Request 0 Enable : Disabled
}

void HT12E::run(void)
{
  uint16_t dataU16 = 0;
  uint8_t cBit = 0;

  if(false == this->rxCodeIsReady())
  {
    if(false == this->rx_fifo.isEmpty())
    {
      dataU16 = this->rx_fifo.pop();
      //Serial.println(dataU16);
      if((0 < this->step) && (this->step < 25))
      {
        //Serial.print(dataU16); Serial.print(": "); Serial.println(this->step);
        if(this->isShort(dataU16))
        {
          //Serial.print(dataU16); Serial.print(": Good short bit at step "); Serial.println(this->step);
          this->codeBitStream[(this->step)-1] = false;
          this->step++;
        }
        else
        {
          if(this->isLong(dataU16))
          {
            //Serial.print(dataU16); Serial.print(": Good long bit at step "); Serial.println(this->step);
            this->codeBitStream[(this->step)-1] = true;
            this->step++;
          }
          else
          {
            //Serial.print(dataU16); Serial.print(": Bad bit at step "); Serial.println(this->step);
            this->step = 0;
          }
        }

        if(true == this->rxCodeIsReady())
        {
          this->code = 0;
          for(cBit=0; cBit<24; cBit++)
          {
            //Serial.print(codeBitStream[23-cBit]);
            if(false == this->codeBitStream[cBit])
            {
              bitClear(this->code, cBit);
            }
            else
            {
              bitSet(this->code, cBit);
            }
          }
          //Serial.println();
        }
      }
      if(0 == this->step)
      {
        if(this->isShort(dataU16))
        {
          //Serial.print(dataU16); Serial.print(": Good sync at step "); Serial.println(this->step);
          this->step++;
        }
        else
        {
          //Serial.print(dataU16); Serial.print(": Bad sync at step "); Serial.println(this->step);
        }
      }
    }
  }
}

bool HT12E::rxCodeIsReady(void)
{
  if(25 == this->step) { return true; }
  else { return false; }
}

uint32_t HT12E::rxGetCode(void)
{
  return (this->code);
}

uint16_t HT12E::rxGetAddress(void)
{
  return (0x0000FFFF & this->code);
}

uint8_t HT12E::rxGetData(void)
{
  return (0x00FF0000 & this->code) >> 16;
}

void HT12E::rxRelease(void)
{
  uint8_t i = 0;

  this->code = 0;
  for(i=0; i<24; i++) { this->codeBitStream[i] = false; }
  this->step = 0;
}

bool HT12E::txIsReady(void)
{
  /* FIXME : Not implemented */
  return true;
}

void HT12E::send(uint32_t code)
{
  /* FIXME : Not implemented */
}

void HT12E::purge(void)
{
  rx_fifo.purge();
}

/******************** PRIVATE ***************************/

bool HT12E::isShort(uint16_t timeU16)
{
  if((300 < timeU16) && (timeU16 < 1000)) { return true; }
  else { return false; }
}

bool HT12E::isLong(uint16_t timeU16)
{
  if((1100 < timeU16) && (timeU16 < 1500)) { return true; }
  else { return false; }
}


ISR(INT1_vect)
{
  uint16_t dataU16 = 0;

  dataU16 = TCNT1;
  TCNT1 = 0x0000;

  if((300 < dataU16) && (dataU16 < 1500))
  {
    if(false == ht12e_extInt_Fifo->isFull())
    {
    	ht12e_extInt_Fifo->push(dataU16);
    }
  }
}

