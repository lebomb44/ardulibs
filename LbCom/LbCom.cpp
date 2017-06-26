#include <inttypes.h>
#include <Arduino.h> 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include "wiring_private.h"
#include <Fifo_U08.h>
#include "LbCom.h"

Fifo_U08 * uart_rx_fifo = NULL;

LbCom::LbCom(): rxFrame(NULL)
{
  this->init();
}

void LbCom::init(void)
{
  uint16_t i = 0;

  this->rx_fifo.init();
  if(NULL == rxFrame) {
    rxFrame = malloc(LBCOM_FRAME_MAX_SIZE);
  }
  this->rxRelease();
  printIsEnabled = false;

  uart_rx_fifo = &(this->rx_fifo);

  digitalWrite(1, HIGH);

  cbi(UCSR2B, RXEN2); // Bit 4 : RXENn: Receiver Enable n : Disabled
  cbi(UCSR2B, TXEN2); // Bit 3 : TXENn: Transmitter Enable n : Disabled

  // Bit 7 (RO) : RXCn: USART Receive Complete
  cbi(UCSR2A, TXC2); // Bit 6 : TXCn: USART Transmit Complete
  // Bit 5 (RO) : UDREn: USART Data Register Empty
  // Bit 4 (RO) : FEn: Frame Error
  // Bit 3 (RO) : DORn: Data OverRun
  // Bit 2 (RO) : UPEn: USART Parity Error
  cbi(UCSR2A, U2X2); // Bit 1 : U2Xn: Double the USART Transmission Speed : Disabled
  cbi(UCSR2A, MPCM2); // Bit 0 : MPCMn: Multi-processor Communication Mode : Disabled

  sbi(UCSR2B, RXCIE2); // Bit 7 : RXCIEn: RX Complete Interrupt Enable n : Enabled
  cbi(UCSR2B, TXCIE2); // Bit 6 : TXCIEn: TX Complete Interrupt Enable n : Disabled
  //FIXME sbi(UCSR2B, UDRIE2); // Bit 5 : UDRIEn: USART Data Register Empty Interrupt Enable n : Enabled
  cbi(UCSR2B, UCSZ22); // Bit 2 : UCSZn2: Character Size n : 011 = 8-bit
  // Bit 1 (RO) : RXB8n: Receive Data Bit 8 n
  cbi(UCSR2B, TXB82); // Bit 0 : TXB8n: Transmit Data Bit 8 n : Disabled

  cbi(UCSR2C, UMSEL21); // Bits 7:6 : UMSELn1:0 USART Mode Select : Asynchronous USART
  cbi(UCSR2C, UMSEL20);
  cbi(UCSR2C, UPM21); // Bits 5:4 : UPMn1:0: Parity Mode : Disabled
  cbi(UCSR2C, UPM20);
  cbi(UCSR2C, USBS2); // Bit 3 : USBSn: Stop Bit Select : 1-bit
  sbi(UCSR2C, UCSZ21); // Bit 2:1 : UCSZn1:0: Character Size : 011 = 8-bit
  sbi(UCSR2C, UCSZ20);
  cbi(UCSR2C, UCPOL2); // Bit 0 : UCPOLn: Clock Polarity : Not used

  UBRR2H=0x00;    // Baud Rate 115200 bps = 8@16Mhz
  UBRR2L=8;

  sbi(UCSR2B, RXEN2); // Bit 4 : RXENn: Receiver Enable n : Enabled
  sbi(UCSR2B, TXEN2); // Bit 3 : TXENn: Transmitter Enable n : Enabled
}

void LbCom::run(void)
{
  uint8_t dataU08 = 0;

  if(false == this->rxIsReady())
  {
    if(false == this->rx_fifo.isEmpty())
    {
      dataU08 = this->rx_fifo.pop();
      if(0 < this->rx_step)
      {
        if(this->rx_step < LBCOM_FRAME_MAX_SIZE+1)
        {
          if(this->rxGetFrameLen() == this->rx_step)
          {
            uint8_t crc = 0;
            uint16_t i = 0;
            for(i=0; i<(this->rx_step-1); i++)
            {
              crc = _crc_ibutton_update(crc, this->rxFrame[i]);
            }
            if(dataU08 == crc)
            {
              this->rxFrame[this->rx_step-1] = dataU08;
              this->rx_step++;
            }
            else { this->rxRelease(); }
          }
          else
          {
            this->rxFrame[this->rx_step-1] = dataU08;
            this->rx_step++;
          }
        }
        else { this->rxRelease(); }
      }
      if(0 == this->rx_step) { if(0xAA == dataU08) { this->rx_step++; } }
    }
  }
}

bool LbCom::rxIsReady(void)
{
  if(this->rxGetFrameLen() < this->rx_step) { return true; }
  else { return false; }
}

uint8_t LbCom::rxGetSrc(void)
{
  return this->rxFrame[0];
}

uint8_t LbCom::rxGetDst(void)
{
  return this->rxFrame[1];
}

uint8_t LbCom::rxGetCmd(void)
{
  return this->rxFrame[2];
}

uint8_t LbCom::rxGetDataLen(void)
{
  return this->rxFrame[3];
}

uint16_t LbCom::rxGetFrameLen(void)
{
  return 4+rxGetDataLen()+1;
}

uint8_t * LbCom::rxGetData(void)
{
  return &(this->rxFrame[4]);
}

uint8_t * LbCom::rxGetFrame(void)
{
  return &(this->rxFrame[0]);
}

void LbCom::rxRelease(void)
{
  uint16_t i = 0;

  for(i=0; i<(LBCOM_FRAME_MAX_SIZE); i++)
  {
    this->rxFrame[i] = 0;
  }
  this->rx_step = 0;
}

void LbCom::send(uint8_t src, uint8_t dst, uint8_t cmd, uint8_t len, uint8_t * data)
{
  uint16_t i = 0;
  uint8_t crc = 0;
  this->send_char(0xAA); crc = _crc_ibutton_update(crc, 0xAA);
  this->send_char(src); crc = _crc_ibutton_update(crc, src);
  this->send_char(dst); crc = _crc_ibutton_update(crc, dst);
  this->send_char(cmd); crc = _crc_ibutton_update(crc, cmd);
  this->send_char(len); crc = _crc_ibutton_update(crc, len);
  for(i=0; i<len ; i++)
  {
    this->send_char(data[i]); crc = _crc_ibutton_update(crc, data[i]);
  }
  this->send_char(crc);
}

void LbCom::enablePrint(void)
{
  printIsEnabled = true;
}

void LbCom::disablePrint(void)
{
  printIsEnabled = false;
}

void LbCom::send_char(uint8_t data)
{
  sbi(UCSR2B, UDRIE2);
}

ISR(USART2_RX_vect)
{
  uart_rx_fifo->push(UDR2);
}

