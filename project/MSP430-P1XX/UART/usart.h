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
#ifndef __USART_H
#define __USART_H

/***** 1.All functions prototypes *****/
void InitUART0 (void);
void UART_transmit(unsigned char Data);
void UART0SendTxt(char* a);

#endif //__USART_H
