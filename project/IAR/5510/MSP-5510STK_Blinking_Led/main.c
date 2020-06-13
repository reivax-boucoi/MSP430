#include <msp430f5510.h>
#include "lcd.h"

#define     TIMER1_PERIOD   10000
#define     LED_ON          P4OUT |= BIT7
#define     LED_OFF         P4OUT &= ~BIT7
#define     LED_TOGGLE      P4OUT ^= BIT7
#define     BUT1            (P1IN & BIT0)
#define     BUT2            (P2IN & BIT0)
#define     PRESSED         0
#define     RELEASED        1

void Init_Ports (void);
void Init_Timer1 (void);


volatile unsigned char but1_flag = RELEASED;
volatile unsigned char but2_flag = RELEASED;


void main (void){
    WDTCTL = WDTPW + WDTHOLD;                   //Stop the watchdog                          
    
   
    Init_Ports();                      
    Init_Timer1();  
    LCD_Init();
    __enable_interrupt();

    while (1){      

        if (BUT1 == PRESSED && but1_flag == RELEASED){          
          but1_flag = PRESSED;    
          TA1CCR0 = 6000;
          LCD_Send_Long_STR(1,10000,"Test");
        }
        if (BUT1 == RELEASED && but1_flag == PRESSED){
          but1_flag = RELEASED;          
        }
        if (BUT2 == PRESSED && but2_flag == RELEASED){          
          but2_flag = PRESSED;
          TA1CCR0 = 2000;
        }
        if (BUT2 == RELEASED && but2_flag == PRESSED){
          but2_flag = RELEASED;          
        }
    }
}

void Init_Ports (void){
    P1REN |= BIT0;
    P1OUT = 0x01;
    P1DIR = 0xFE;
    P2REN |= BIT0;
    P2OUT = 0x01;
    P2DIR = 0x00;
    P4OUT = 0x00;
    P4DIR = 0x80;
    P5DS  = 0x02;
    P5OUT = 0x02;
    P5DIR = 0x03;
    P6OUT = 0x00;                 
    P6DIR = 0x02;
}

void Init_Timer1 (void){  
   TA1CTL |= TASSEL0 + MC0 + TACLR;
   TA1CCTL0 |= CCIE;
   TA1CCR0 = TIMER1_PERIOD;   
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR (void){
    LED_TOGGLE;                 
}