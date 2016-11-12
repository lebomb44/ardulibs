#include <Arduino.h> 

#include "LB_NRF24.h"
#include <SPI.h>

LB_NRF24::LB_NRF24()
{
  this->init();
}

void LB_NRF24::init(void)
{
    pinMode(cePin,OUTPUT);
    pinMode(csnPin,OUTPUT);

    ceLow();
    csnHi();

    // Initialize spi module
    spi->begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV2); // 8MHz SPI clock

    ce(LOW);
    csn(HIGH);

    delay( 5 ) ;
    write_register(SETUP_RETR,(B0100 << ARD) | (B1111 << ARC));
    setPALevel( RF24_PA_MAX ) ;
    setDataRate( RF24_1MBPS ) ;
    setCRCLength( RF24_CRC_16 ) ;
    setCRCLength( RF24_CRC_16 ) ;
    write_register(DYNPD,0);
    write_register(STATUS,_BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT) );
    setChannel(76);

    spiWriteRegister(NRF24_REG_07_STATUS, NRF24_RX_DR | NRF24_TX_DS | NRF24_MAX_RT)
    powerDown();
    flush_rx();
    flush_tx();
    powerUpRx();

    setRetries(15,15);
}

void LB_NRF24::run(void)
{
}

bool LB_NRF24::rxIsReady(void)
{

}

uint8_t LB_NRF24::rxGetSrc(void)
{
  return this->rawData[0];
}

uint8_t LB_NRF24::rxGetDst(void)
{
  return this->rawData[1];
}

uint8_t LB_NRF24::rxGetCmd(void)
{
  return this->rawData[2];
}

uint8_t LB_NRF24::rxGetLen(void)
{
  return this->rawData[3];
}

uint8_t * LB_NRF24::rxGetData(void)
{
  return &(this->rawData[4]);
}

void LB_NRF24::rxRelease(void)
{
}

bool LB_NRF24::txIsReady(void)
{

}

void LB_NRF24::send(uint8_t src, uint8_t dst, uint8_t cmd, uint8_t len, uint8_t * data)
{
/*
  Serial.write(0xAA);
  Serial.write(src); Serial.write(dst); Serial.write(cmd); Serial.write(len);
  Serial.write(data, len);
*/
}

