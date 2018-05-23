//(c)2010 by Texas Instruments Incorporated, All Rights Reserved.
/*  
 * ======== main.c ========
 * Needs to run on MSP430F5510-STK board by www.olimex.com
 *
 * Designed by eng. Georgi Todorov
 *
 +----------------------------------------------------------------------------+
 * Please refer to the MSP430 USB API Stack Programmer's Guide,located
 * in the root directory of this installation for more details.
 *----------------------------------------------------------------------------*/

#include <msp430f5510.h>

//=====================Definitions=========================
#define     TIMER1_PERIOD   10000
#define     LED_ON          P4OUT |= BIT7
#define     LED_OFF         P4OUT &= ~BIT7
#define     LED_TOGGLE      P4OUT ^= BIT7
#define     BUT1            (P1IN & BIT0)
#define     BUT2            (P2IN & BIT0)
#define     PRESSED         0
#define     RELEASED        1


//======================Function prototypes================

void Init_Ports (void);
void Init_Timer1 (void);


volatile unsigned char but1_flag = RELEASED;
volatile unsigned char but2_flag = RELEASED;


/*  
 * ======== main ========
 */ 
void main (void)  
{
    WDTCTL = WDTPW + WDTHOLD;                   //Stop the watchdog                          
    
   
    Init_Ports();                      
    Init_Timer1();  
            
    __enable_interrupt();                      //Enable general interrupts

    while (1)
    {      

        if (BUT1 == PRESSED && but1_flag == RELEASED)   //Check if BUT1 is pressed
        {          
          but1_flag = PRESSED;          
          TA1CCR0 = 2000;
        }
        if (BUT1 == RELEASED && but1_flag == PRESSED)  //Check if BUT1 is released
        {
          but1_flag = RELEASED;          
        }
        
        if (BUT2 == PRESSED && but2_flag == RELEASED)  //Check if BUT2 is pressed
        {          
          but2_flag = PRESSED;                             
          TA1CCR0 = 6000;
        }
        if (BUT2 == RELEASED && but2_flag == PRESSED)  //Check if BUT2 is released
        {
          but2_flag = RELEASED;          
        }
        
    }
}

void Init_Ports (void)
{
    //Initialization of ports (all unused pins as outputs with low-level
    P1REN |= BIT0;                //Enable BUT1 pullup
    P1OUT = 0x01;
    P1DIR = 0xFE;                 //LCD pins are outputs
    P2REN |= BIT0;                //Enable BUT2 pullup
    P2OUT = 0x01;
    P2DIR = 0x00;
    P4OUT = 0x00;
    P4DIR = 0x80;                 //LED is output    
    P5DS  = 0x02;                 //Increase drive strainght of P5.1(LCD-PWR)
    P5OUT = 0x02;                 //LCD_PWR_E, Battery measurement enable
    P5DIR = 0x03;
    P6OUT = 0x00;                 
    P6DIR = 0x02;                 //STNBY_E    
}

/*  
 * ======== Init_Timer1 ========
 */
void Init_Timer1 (void)
{  
   TA1CTL |= TASSEL0 + MC0 + TACLR;              //ACLK for T1 source
                                                 //Up mode: Timer counts up to TAxCCR0
                                                 //reset timer A1                                         
   TA1CCTL0 |= CCIE;                             //Interrupt enabled
   TA1CCR0 = TIMER1_PERIOD;   
}

/*  
 * ======== TIMER1_ISR ========
 */
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR (void)
{
    LED_TOGGLE;                 
}



