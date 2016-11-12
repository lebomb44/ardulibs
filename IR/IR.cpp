#include <inttypes.h>
#include <Arduino.h> 

#include "IR.h"

Fifo_U16 * timer1_samsungManufacturer_fifo = NULL;
Fifo_U08 * timer1_samsungCode_fifo = NULL;

IR::IR()
{
  this->init();
}

void IR::init(void)
{
  this->samsungManufacturer_fifo.init();
  this->samsungCode_fifo.init();

  timer1_samsungManufacturer_fifo = &(this->samsungManufacturer_fifo);
  timer1_samsungCode_fifo = &(this->samsungCode_fifo);
}

void IR::setSamsung(uint16_t manufacturer, uint8_t code)
{
  if((false == this->samsungManufacturer_fifo.isFull()) && (false == this->samsungCode_fifo()))
  {
    this->samsungManufacturer_fifo.push(manufacturer);
    this->samsungCode_fifo.push(code);
  }
}

ISR(TIMER1_COMPA_vect, ISR_NOBLOCK)
{
    if(ptm_test->line.bpos == 1) { sbi(PORTB,4); }

    bit = tm_bit_Extract(&(ptm_test->line));

    if(bit==0) { TCCR1A = 0x82; } /* S/C (10xx xxxx) + Fast PWM TOP=OCRnA (xxxx xx10)*/

          else { TCCR1A = 0xC2; } /* S/C (11xx xxxx) + Fast PWM TOP=OCRnA (xxxx xx10)*/

}

/********* START ************/
cbi(SREG,7);
  EECR   = 0;
/*  EIMSK  = 0; */
/*  TIMSK  = 0; */
  TIFR   = 0xFF;   // Clear interrupt flags
  ETIMSK = 0;
  SPCR   = 0;
  TWCR   = 0;
  ACSR   = 0;
  ADCSRA = 0;
/*  SPMCSR = 0; */
sbi(SREG,7);

  TCCR1A=0x00;
  TCCR1B=0x00;
  TCNT1=0x0000; // Clear counter

  tm_build_Line(ptm_config, &(ISR_tm_test.line));
  ICR1  = ((uint32_t)NutGetCpuClock()) / ((uint32_t)(ptm_config_test->bit_rate));
  OCR1A = ICR1/2;
  sbi(TIMSK,4);
  bit = tm_bit_Extract(&(ISR_tm_test.line));
  if(bit==0) { TCCR1A = 0x82; } /* S/C (10xx xxxx) + Fast PWM TOP=OCRnA (xxxx xx10)*/
        else { TCCR1A = 0xC2; } /* S/C (11xx xxxx) + Fast PWM TOP=OCRnA (xxxx xx10)*/
  TCCR1B = 0x19; /* Fast PWM TOP=OCRnA (xxx11xxx) + Clk/1 (xxxx x001)*/

/************ STOP ***************/
TCCR1A=0x00;
if(TCCR1B) {
    cbi(SREG,7);
    TCCR1B=0x00;
    TIFR=0xFF;    // Clear interrupt flags
    TCNT1=0x0000; // Clear counter
    sbi(SREG,7);
}
