#include <msp430f5510.h>
#include <string.h>
#include <stdio.h>
#include "ext_test.h"
#include "lcd.h"
//#include "intrinsics.h"
//#include <stdlib.h>


unsigned char g_f=1, f, count, flag=6, e_0=1, e_1=1, e_2=1, e_3=1;
unsigned char aray[12][8];
unsigned int d;
unsigned char error_counter = 3;
unsigned char P1MASK;
unsigned char P2MASK;
unsigned char P4MASK;
unsigned char P5MASK;
unsigned char P6MASK;
unsigned char PULL_PIN_DIR;
unsigned char PULL_PIN_OUT;
unsigned char EXT_flag = 1;


void Set_EXT_MASKs(void)
{
  P1MASK = 0xFF;		//bit 0:7 (14-21)
  P2MASK = 0x01;		//bit 0   (22)
  P4MASK = 0x7F;		//bit 0:6 (29-36) //test withou pull pin
  P5MASK = 0x03;		//bit 0:1 (5-6)
  P6MASK = 0x0F;		//bit 0:3 (1-4)	
  EXT_flag = 1;
}

void Set_UEXT_MASKs(void)
{
  P1MASK = 0x00;		//
  P2MASK = 0x00;		//
  P4MASK = 0x3E;		//bit 1:5 (30-34) //test withou pull pin
  P5MASK = 0x00;		//
  P6MASK = 0x0C;		//bit 2:3 (3-4)	
  EXT_flag = 0;
}

//optional init
void PIN_Init()
{
  int i;
  //PORT1 +
  for (i=0; i<=7; i++)
    aray[1][i] = 14+i;
  //PORT1 -
  
  //PORT2 +
  for (i=0; i<=1; i++)
    aray[2][i] = 22+i;
  //PORT2 -  
    
  //PORT4 +
  for (i=0; i<=7; i++)
    aray[4][i] = 29+i;
  //PORT4 -
  
  //PORT5 +
  aray[5][0] = 9; aray[5][1] = 10;
  for (i=4; i<=2; i++)
    aray[5][i] = 5+i;
  //PORT5 -
  
  //PORT6 +
  for (i=0; i<=4; i++)
      aray[6][i] = 1+i;
  //PORT6 -
    
  
}


void TEST_EXT_Init()
{
  
  P1DIR = 0;
  P2DIR = 0;
  P4DIR = 0;  
  P5DIR = 0;
  P6DIR = 0;  
//  P1REN = 0;
//  P1REN = 0;
  
  //Extension pins set as PORT function and input +
  P1SEL = P1SEL & ~P1MASK;
  P1DIR = P1DIR & ~P1MASK;
  
  P2SEL = P2SEL & ~P2MASK;
  P2DIR = P2DIR & ~P2MASK;
  
  P4SEL = P4SEL & ~P4MASK;
  P4DIR = P4DIR & ~P4MASK;
  
  P5SEL = P5SEL & ~P5MASK;
  P5DIR = P5DIR & ~P5MASK;
  
  P6SEL = P6SEL & ~P6MASK;
  P6DIR = P6DIR & ~P6MASK;
  
  Delay(SET_TIME * 10);
  //Extension pins set as PORT function and input -
  
  PIN_Init();
}


void Locate(unsigned char p, unsigned char port)
{
  unsigned char q,j;  
  for (int i=0;i<STR_SIZE;i++) str[i]=0;        //null str
  for (q=0; q<=7; q++)
  {
    if (p & BIT0)
    {
      j = aray[port][q];
      sprintf(str, "\r\n NOT SOLDERED or SHORT with GND or VCC PORT %d.%d (pin %d) \r\n", port,q,aray[port][q]);
      break;
    }
    p >>= 1;
  }
  return;
}

void Locate_PIN(unsigned char port1, unsigned char pin1, unsigned port2, unsigned char pin2)
{
  unsigned char q;
  for (int i=0;i<STR_SIZE;i++) str[i]=0;        //null str
  for (q=0; q<=7; q++)
  {
    if (pin2 & BIT0)
    {      
      sprintf(str, "\r\n SHORT between PORT %d.%d and PORT %d.%d (pin %d to pin %d)\r\n", port1,pin1,port2,q,aray[port1][pin1],aray[port2][q]);
    }
    pin2 >>= 1;
  }
}

unsigned char TEST_Extension()
{
  int i;
  
  //Test PIN to GND +
  error_counter = 0;
//("Test GND");
  f=1;
  if (EXT_flag != 0)
  {
    P4DIR = P4DIR | 1<<7;         //P4.7 (PullPin) set as output
    P4OUT = P4OUT | 1<<7;         //P4.7 = 1  
  }
  else
  {
    P4DIR = P4DIR | 1<<0;         //P4.0 (PullPin) set as output
    P4OUT = P4OUT | 1<<0;         //P4.0 = 1  
  }
  
  Delay(SET_TIME * 10);           //simple delay to set connected pins
  if (P1MASK & ~P1IN) {Locate(P1MASK & ~P1IN, 1); f=0;return 1;}                                    //1
  if (P2MASK & ~P2IN) {Locate(P2MASK & ~P2IN, 2); f=0;return 1;}
  if (P4MASK & ~P4IN) {Locate(P4MASK & ~P4IN, 4); f=0;return 1;}
  if (P5MASK & ~P5IN) {Locate(P5MASK & ~P5IN, 5); f=0;return 1;}
  if (P6MASK & ~P6IN) {Locate(P6MASK & ~P6IN, 6); f=0;return 1;}

  g_f = g_f && f;
  if (f)
    ;
  else
    error_counter++;
  
  //Test PIN to GND -

   
  //Test PIN to VCC +
  
//("Test VCC");
  f=1;
  if (EXT_flag != 0)
  {
    P4DIR = P4DIR | 1<<7;         //P4.7 (PullPin) set as output
    P4OUT = P4OUT & ~(1<<7);      //P4.7 = 0  
  }
  else
  {
    P4DIR = P4DIR | 1<<0;         //P4.0 (PullPin) set as output
    P4OUT = P4OUT & ~(1<<0);      //P4.0 = 0  
  }
  Delay(SET_TIME *5);            //simple delay to clear connected pins
  if (P1MASK & P1IN) {Locate(P1MASK & P1IN, 1); f=0; return 2;}                                      //2
  if (P2MASK & P2IN) {Locate(P2MASK & P2IN, 2); f=0; return 2;}
  if (P4MASK & P4IN) {Locate(P4MASK & P4IN, 4); f=0; return 2;}
  if (P5MASK & P5IN) {Locate(P5MASK & P5IN, 5); f=0; return 2;}
  if (P6MASK & P6IN) {Locate(P6MASK & P6IN, 6); f=0; return 2;}

  g_f = g_f && f;
  if (f)
    ;
  else
    error_counter++;    
  
  //Test PIN to VCC -  
  
  //Test PIN to PullPin +
  
//("Test PullPin (P4.7 - 29)");
    f=1;
    //Running 0 +
    if (EXT_flag != 0)
    {
      P4DIR = P4DIR | 1<<7;         //P4.7 (PullPin) set as output
      P4OUT = P4OUT | 1<<7;         //P4.7 = 1  
    }
    else
    {
      P4DIR = P4DIR | 1<<0;         //P4.0 (PullPin) set as output
      P4OUT = P4OUT | 1<<0;         //P4.0 = 1  
    }

    for (int i=0;i<STR_SIZE;i++) str[i]=0;        //null str
    
    for (i=0; i<=7; i++)
    {
      if (P1MASK & 1<<i)
      {
        P1DIR = P1DIR | 1<<i;
        P1OUT = P1OUT & ~(1<<i);
        Delay(SET_TIME);            //simple delay
        if (!(P4IN & 1<<7))  
          {sprintf(str, "\r\n Pull pin to PORT %d.%d \r\n", 1,i); error_counter++; return 3;}             //3
        P1DIR = P1DIR & ~(1<<i);
      }
    }
    
    for (i=0; i<=7; i++)
    {
      if (P2MASK & 1<<i)
      {
        P2DIR = P2DIR | 1<<i;
        P2OUT = P2OUT & ~(1<<i);
        Delay(SET_TIME);            //simple delay
        if (!(P4IN & 1<<7))  
          {sprintf(str, "\r\n Pull pin to PORT %d.%d \r\n", 2,i); error_counter++;return 3;}             
        P2DIR = P2DIR & ~(1<<i);
      }
    }

    
    for (i=0; i<=7; i++)
    {
      if (P4MASK & 1<<i)
      {
        P4DIR = P4DIR | 1<<i;
        P4OUT = P4OUT & ~(1<<i);
        Delay(SET_TIME);            //simple delay
        if (!(P4IN & 1<<7))  
          {sprintf(str, "\r\n Pull pin to PORT %d.%d \r\n", 4,i); error_counter++;return 3;}             
        P4DIR = P4DIR & ~(1<<i);
      }
    }
    
    for (i=0; i<=7; i++)
    {
      if (P5MASK & 1<<i)
      {
        P5DIR = P5DIR | 1<<i;
        P5OUT = P5OUT & ~(1<<i);
        Delay(SET_TIME);            //simple delay
        if (!(P4IN & 1<<7))  
          {sprintf(str, "\r\n Pull pin to PORT %d.%d \r\n", 5,i); error_counter++;return 3;}            
        P5DIR = P5DIR & ~(1<<i);
      }
    }
    
    for (i=0; i<=7; i++)
    {
      if (P6MASK & 1<<i)
      {
        P6DIR = P6DIR | 1<<i;
        P6OUT = P6OUT & ~(1<<i);
        Delay(SET_TIME);            //simple delay
        if (!(P4IN & 1<<7))  
          {sprintf(str, "\r\n Pull pin to PORT %d.%d \r\n", 6,i); error_counter++;return 3;}             
        P6DIR = P6DIR & ~(1<<i);
      }
    }
    
    g_f = g_f && f;
    if (f)
      ;
    else
      error_counter++;
    //Running 0 -
  
  //Test PIN to PullPin -
    
  if (!g_f) return 0;  
    
  //Test PIN to PIN ++++++++++++++++++++++++++++++++++++++++++++++++++++++
  

  if (EXT_flag != 0)
    {
      P4DIR = P4DIR | 1<<7;         //P4.7 (PullPin) set as output
      P4OUT = P4OUT & ~(1<<7);      //P4.7 = 0  
    }
    else
    {
      P4DIR = P4DIR | 1<<0;         //P4.0 (PullPin) set as output
      P4OUT = P4OUT & ~(1<<0);      //P4.0 = 0  
    }
//("Test PIN to PIN:");
  
    //Test PORT1 +
      f=1;
//("Test PORT1");++
      for (i=0; i<=7; i++)
        if (P1MASK & 1<<i)
        {
          P1DIR = P1DIR | 1<<i;
          P1OUT = P1OUT | 1<<i;
          Delay(SET_TIME * 100);            //simple delay
          if (P1MASK & P1IN & ~(1<<i)) {Locate_PIN(1, i, 1, (P1MASK & P1IN) & ~(1<<i)); f=0;return 4;}        //4
          if (P2MASK & P2IN) {Locate_PIN(1, i, 2, (P2MASK & P2IN)); f=0;return 4;}
          if (P4MASK & P4IN) {Locate_PIN(1, i, 4, (P4MASK & P4IN)); f=0;return 4;}
          if (P5MASK & P5IN) {Locate_PIN(1, i, 5, (P5MASK & P5IN)); f=0;return 4;}
          if (P6MASK & P6IN) {Locate_PIN(1, i, 6, (P6MASK & P6IN)); f=0;return 4;}

          P1DIR = P1DIR & ~(1<<i);
          P1OUT = P1OUT & ~(1<<i);
        }
      g_f = g_f && f;
      if (f)
        ;
      else
      //  return 0;
      error_counter++;
      
    //Test PORT1 -
  
    
    //Test PORT2 +
      f=1;
//      ("Test PORT2");
      for (i=0; i<=7; i++)
        if (P2MASK & 1<<i)
        {
          P2DIR = P2DIR | 1<<i;
          P2OUT = P2OUT | 1<<i;
          Delay(SET_TIME);            //simple delay
          if (P1MASK & P1IN) {Locate_PIN(2, i, 1, (P1MASK & P1IN)); f=0;return 5;}                            //5
          if (P2MASK & P2IN & ~(1<<i)) {Locate_PIN(2, i, 2, (P2MASK & P2IN) & ~(1<<i)); f=0;return 5;}
          if (P4MASK & P4IN) {Locate_PIN(2, i, 4, (P4MASK & P4IN)); f=0;return 5;}
          if (P5MASK & P5IN) {Locate_PIN(2, i, 5, (P5MASK & P5IN)); f=0;return 5;}
          if (P6MASK & P6IN) {Locate_PIN(2, i, 6, (P6MASK & P6IN)); f=0;return 5;}

          P2DIR = P2DIR & ~(1<<i);
          P2OUT = P2OUT & ~(1<<i);
        }
      g_f = g_f && f;
      if (f)
        ;
      else
      //  return 0;
      error_counter++;
    //Test PORT2 -
   

      //Test PORT4 +
      f=1;
//("Test PORT4");
      for (i=0; i<=7; i++)
        if (P4MASK & 1<<i)
        {
          P4DIR = P4DIR | 1<<i;
          P4OUT = P4OUT | 1<<i;
          Delay(SET_TIME);            //simple delay
          if (P1MASK & P1IN) {Locate_PIN(4, i, 1, (P1MASK & P1IN)); f=0;return 6;}                                //6
          if (P2MASK & P2IN) {Locate_PIN(4, i, 2, (P2MASK & P2IN)); f=0;return 6;}
          if (P4MASK & P4IN & ~(1<<i)) {Locate_PIN(4, i, 4, (P4MASK & P4IN) & ~(1<<i)); f=0;return 6;}
          if (P5MASK & P5IN) {Locate_PIN(4, i, 5, (P5MASK & P5IN)); f=0;return 6;}
          if (P6MASK & P6IN) {Locate_PIN(4, i, 6, (P6MASK & P6IN)); f=0;return 6;}

          P4DIR = P4DIR & ~(1<<i);
          P4OUT = P4OUT & ~(1<<i);
        }
      g_f = g_f && f;
      if (f)
        ;
      else
      //  return 0;
      error_counter++;
    //Test PORT4 -
  
 
    //Test PORT5 +
      f=1;
//("Test PORT5");
      for (i=0; i<=7; i++)
        if (P5MASK & 1<<i)
        {
          P5DIR = P5DIR | 1<<i;
          P5OUT = P5OUT | 1<<i;
          Delay(SET_TIME * 100);            //simple delay
          if (P1MASK & P1IN) {Locate_PIN(5, i, 1, (P1MASK & P1IN)); f=0;return 7;}                        //7
          if (P2MASK & P2IN) {Locate_PIN(5, i, 2, (P2MASK & P2IN)); f=0;return 7;}
          if (P4MASK & P4IN) {Locate_PIN(5, i, 4, (P4MASK & P4IN)); f=0;return 7;}
          if (P5MASK & P5IN & ~(1<<i)) {Locate_PIN(5, i, 5, (P5MASK & P5IN) & ~(1<<i)); f=0;return 7;}
          if (P6MASK & P6IN) {Locate_PIN(5, i, 6, (P6MASK & P6IN)); f=0;return 7;}

          P5DIR = P5DIR & ~(1<<i);
          P5OUT = P5OUT & ~(1<<i);
        }
      g_f = g_f && f;
      if (f)
        ;
      else
      //  return 0;
      error_counter++;
    //Test PORT5 -
  
    //Test PORT6 +
      f=1;
//("Test PORT6");
      for (i=0; i<=7; i++)
        if (P6MASK & 1<<i)
        {
          P6DIR = P6DIR | 1<<i;
          P6OUT = P6OUT | 1<<i;
          Delay(SET_TIME * 100);            //simple delay
          if (P1MASK & P1IN) {Locate_PIN(6, i, 1, (P1MASK & P1IN)); f=0;return 8;}
          if (P2MASK & P2IN) {Locate_PIN(6, i, 2, (P2MASK & P2IN)); f=0;return 8;}
          if (P4MASK & P4IN) {Locate_PIN(6, i, 4, (P4MASK & P4IN)); f=0;return 8;}
          if (P5MASK & P5IN) {Locate_PIN(6, i, 5, (P5MASK & P5IN)); f=0;return 8;}
          if (P6MASK & P6IN & ~(1<<i)) {Locate_PIN(6, i, 6, (P6MASK & P6IN) & ~(1<<i)); f=0;return 8;}        //8

          P6DIR = P6DIR & ~(1<<i);
          P6OUT = P6OUT & ~(1<<i);
        }
      g_f = g_f && f;
      if (f)
        ;
      else
      //  return 0;
      error_counter++;
    //Test PORT6 - 
  return 0;  
  //Test PIN to PIN -
        
}



