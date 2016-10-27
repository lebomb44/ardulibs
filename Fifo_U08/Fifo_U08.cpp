#include <Arduino.h> 

#include "Fifo_U08.h"

Fifo_U08::Fifo_U08()
{
  this->init();
}

void Fifo_U08::init(void)
{
  uint16_t i = 0;

  for(i=0; i<FIFO_U08_SIZE; i++)
  {
    this->fifo_buff[i] = 0;
  }
  this->fifo_in = 0;
  this->fifo_out = 0;
}

void Fifo_U08::push(uint8_t data)
{
  if(false == this->isFull())
  {
    this->fifo_buff[this->fifo_in] = data;
    this->fifo_in++;
  }
}

uint8_t Fifo_U08::pop(void)
{
  uint8_t data = 0;

  if(false == this->isEmpty())
  {
    data = this->fifo_buff[this->fifo_out];
    this->fifo_out++;
  }
  return data;
}

bool Fifo_U08::isEmpty(void)
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

bool Fifo_U08::isFull(void)
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

void Fifo_U08::purge(void)
{
  volatile uint16_t data = 0;

  while(false == isEmpty())
  {
    data = pop();
  }
}

