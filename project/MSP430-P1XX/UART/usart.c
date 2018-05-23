/**********************************************************************************/
/*    Demo program for:								  */
/*	  Board: MSP430-P2618     						  */
/*    Manufacture: OLIMEX                                                   	  */
/*	  COPYRIGHT (C) 2010							  */
/*    Designed by: Engineer Penko T. Bozhkov                                      */
/*    Module Name    :  main module                                               */
/*    File   Name    :  main.c                                                    */
/*    Revision       :  initial                                                   */
/*    Date           :  18.03.2010                                                */
/*    Built with IAR Embedded Workbench Version: 4.21                             */
/*    Toggle LED via USART and send message when BUT is pressed!                  */
/**********************************************************************************/
#include  <msp430x24x.h>
#include "usart.h"

void InitUART0 (void)                           //UART0 init
{
   
   //input_data=P3IN;          //Read only register
  P3OUT = 0x00;               // The outputs are low
  P3SEL = 0x30;               // Peripheral module function: enabled for UTXD0 and URXD0
  P3DIR = 0x10;               // 1 -> Output; 0 -> Input; UTXD0 output
  P3REN = 0x00;               // Pullup/pulldown resistor disabled
  
  UCA0CTL1 |= 0x01;  // 1) Set UCSWRST
  // 2) Initialize all USCI registers with UCSWRST = 1 (including UCAxCTL1)
  UCA0CTL0 = 0x00;  // 0x40 -> MSB first
  UCA0CTL1 = 0x41;  
  UCA0BR1 = 0x00;   // N = 32768/9600 = 3.413333
  UCA0BR0 = 0x03;   // Write integer portion of the divisor "N", i.e. UCBRx = INT(N) = 3
  UCA0MCTL = 0x08;  // UCBRSx = round( ( N - INT(N) ) * 8 ) =   4
  
  UCA0CTL1 &= ~0x01;
  // 5) Enable interrupts (optional)
  IE2 |= 0x01;      // USCI_A0 receive interrupt enable
  IFG2 &= ~0x01;    // Clear UCA0 RXIFG
  
  __enable_interrupt();
  for (int i=10; i; i--);  // short delay
}

void UART_transmit(unsigned char Data)
{
  while(UCA0STAT & 0x01);	// Check USCI busy flag
  UCA0TXBUF = Data;
}

void UART0SendTxt(char* a)
{
   for(;*a;++a) UART_transmit(*a);
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void usart0_rx (void)
{
  UART_transmit (UCA0RXBUF+1);                  //transmit Echo + 1
}
