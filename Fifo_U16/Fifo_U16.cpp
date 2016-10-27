#include <Arduino.h> 

#include "Fifo_U16.h"

Fifo_U16::Fifo_U16()
{
  this->init();
}

void Fifo_U16::init(void)
{
  uint16_t i = 0;

  for(i=0; i<FIFO_U16_SIZE; i++)
  {
    this->fifo_buff[i] = 0;
  }
  this->fifo_in = 0;
  this->fifo_out = 0;
}

void Fifo_U16::push(uint16_t data)
{
  if(false == this->isFull())
  {
    this->fifo_buff[this->fifo_in] = data;
    this->fifo_in++;
  }
}

uint16_t Fifo_U16::pop(void)
{
  uint16_t data = 0;

  if(false == this->isEmpty())
  {
    data = this->fifo_buff[this->fifo_out];
    this->fifo_out++;
  }
  return data;
}

bool Fifo_U16::isEmpty(void)
{
  if(this->fifo_in == this->fifo_out)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool Fifo_U16::isFull(void)
{
  if((this->fifo_in+1) == this->fifo_out)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void Fifo_U16::purge(void)
{
  volatile uint16_t data = 0;

  while(false == isEmpty())
  {
    data = pop();
  }
}

