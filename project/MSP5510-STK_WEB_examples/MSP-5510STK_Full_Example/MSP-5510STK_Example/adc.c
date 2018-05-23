/**********************************************************************************/
/*Test program for:						 	          */
/*    Board: MSP430F5510-STK							  */
/*    Manufacture: OLIMEX                                                   	  */
/*    COPYRIGHT (C) 2012							  */
/*    Designed by:  Georgi Todorov                                                */
/*    Module Name    :  GDSC-0801WP-01-MENT                                       */
/*    File   Name    :  adc.c                                                     */
/*    Revision       :  Rev.A                                                     */
/*    Date           :  20.01.2012					          */
/**********************************************************************************/

#include <msp430f5510.h>
#include "adc.h"

  
volatile unsigned int results[Num_of_Results];
extern unsigned int temp_ADC;
                                            // Needs to be global in this
                                            // example. Otherwise, the
                                            // compiler removes it because it
                                            // is not used for anything.

void ADC_Init(void)
{
  REFCTL0 |= REFON;                          //turn on internal VREF to 1.5V
  
  P6SEL |= BIT0;                            // Enable A/D channel A0
  ADC10CTL0 = ADC10ON+ADC10SHT_8+ADC10MSC;   // Turn on ADC10, set sampling time
                                            // set multiple sample conversion
  ADC10CTL1 = ADC10SHP+ADC10CONSEQ_2;       // Use sampling timer, set mode
  
  ADC10MCTL0 |= ADC10SREF0;                 //select internal VREF
//  ADC10MCTL0 |= (ADC10INCH1 | ADC10INCH3);   //select 10 ADC chanel - temp sensor
//    ADC10MCTL0 &= ~(ADC10INCH0 | ADC10INCH1 | ADC10INCH2 | ADC10INCH3);   //select 0 ADC chanel - battery voltage
//  ADC10CTL1 |= ADC10DIV_7 + ADC10SSEL1;
  ADC10CTL2 |= ADC10PDIV_2;                 //Divide MODOSC-5MHz(ADC_CLK) with 64
  
  ADC10IE = 0x01;                           // Enable ADC12IFG.0
  ADC10CTL0 |= ADC10ENC;                    // Enable conversions
  ADC10CTL0 |= ADC10SC;                     // Start conversion
  
  __no_operation();                         // For debugger
  
}


#pragma vector=ADC10_VECTOR
__interrupt void ADC10ISR (void)
{
  static unsigned char index = 0;

  switch(__even_in_range(ADC10IV,12))
  {
  case  0: break;                           // Vector  0:  No interrupt
  case  2: break;                           // Vector  2:  ADC overflow
  case  4: break;                           // Vector  4:  ADC timing overflow
  case  12:                                  // Vector  12:  ADC10IFG0
    results[index] = ADC10MEM0;             // Move results
    index++;                                // Increment results index, modulo; Set Breakpoint1 here   
    
    if (index == Num_of_Results)
    {
      index = 0;
    }
  
  default: break; 
  }
}







