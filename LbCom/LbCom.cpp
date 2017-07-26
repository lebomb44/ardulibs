#include <inttypes.h>
#include <Arduino.h> 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include "wiring_private.h"
#include <Fifo_U08.h>
#include "LbCom.h"

Fifo_U08 * uart_rx_fifo = NULL;

LbCom::LbCom(): _rx_msg(LBMSG_DATA_MAX_SIZE), _rx_step(0), printIsEnabled(false)
{
  this->init();
}

void LbCom::init(void)
{
  uint16_t i = 0;

  this->rx_fifo.init();
  this->_rx_msg.init(LBMSG_DATA_MAX_SIZE);
  this->rxReleaseMsg();
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
  if(false == this->rxMsgIsReady())
  {
    if(false == this->rx_fifo.isEmpty())
    {
      uint8_t dataU08 = this->rx_fifo.pop();
      if(0 < this->_rx_step)
      {
        if(this->_rx_step < LBMSG_FRAME_MAX_SIZE+1)
        {
          this->_rx_msg.getFrame()[this->_rx_step-1] = dataU08;
          this->_rx_step++;
        }
        else { this->rxReleaseMsg(); }
      }
      if(0 == this->_rx_step) { if(0xAA == dataU08) { this->_rx_step++; } }
    }
  }
}

bool LbCom::rxMsgIsReady(void)
{
  if(this->_rx_msg.getFrameLen() < this->_rx_step) { return true; }
  else { return false; }
}

void LbCom::rxReleaseMsg(void)
{
  for(uint16_t i=0; i<4; i++)
  {
    this->_rx_msg.getFrame()[i] = 0;
  }
  this->_rx_step = 0;
}

void LbCom::rxGetMsg(LbMsg & msg)
{
  msg.setDataLen(_rx_msg.getDataLen());
  uint16_t len = msg.getFrameLen();
  for(uint16_t i=0; i<len ; i++)
  {
    msg.getFrame()[i] = _rx_msg.getFrame()[i];
  }
  rxReleaseMsg();
}

void LbCom::send(LbMsg & msg)
{
  this->send_char(0xAA);
  uint16_t len = msg.getFrameLen();
  for(uint16_t i=0; i<len ; i++)
  {
    this->send_char(msg.getFrame()[i]);
  }
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
  // FIXME polling to do
}

ISR(USART2_RX_vect)
{
  uart_rx_fifo->push(UDR2);
}

