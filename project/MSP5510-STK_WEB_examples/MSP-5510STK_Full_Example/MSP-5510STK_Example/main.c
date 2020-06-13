//(c)2010 by Texas Instruments Incorporated, All Rights Reserved.
/*  
* ======== main.c ========
* Needs to run on MSP430F5510-STK board by www.olimex.com
* MSP5510-STK board demo example supports 1 CDCs, 1 HIDs, 1 MSC with 2 LUNs, 1x9LCD, SD card, 2 buttons, 1 LED, Battery measurement ...
* Designed by eng. Georgi Todorov
*
+----------------------------------------------------------------------------+
* Please refer to the MSP430 USB API Stack Programmer's Guide,located
* in the root directory of this installation for more details.
*----------------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <msp430f5510.h>

#include "lcd.h"
#include "adc.h"
#include "ext_test.h"

#include "USB_config/descriptors.h"
#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/types.h"
#include "USB_API/USB_Common/usb.h"

#include "F5xx_F6xx_Core_Lib/HAL_UCS.h"
#include "F5xx_F6xx_Core_Lib/HAL_PMM.h"
#include "F5xx_F6xx_Core_Lib/HAL_FLASH.h"

//MSC #includes
#include "USB_API/USB_MSC_API/UsbMscScsi.h"
#include "USB_API/USB_MSC_API/UsbMsc.h"
#include "USB_API/USB_MSC_API/UsbMscStateMachine.h"

//Board and FatFs #includes
#include "FatFs/diskio.h"
#include "FatFs/mmc.h"
#include "mscFse.h"
#include "LUN0_data.h"

#include "usbConstructs.h"

//=====================Definitions=========================
#define     TIMER1_PERIOD   4000                    //timer1 period in 1/32768 Hz
#define     LED_ON          P4OUT |= BIT7
#define     LED_OFF         P4OUT &= ~BIT7
#define     LED_TOGGLE      P4OUT ^= BIT7
#define     BUT1            (P1IN & BIT0)
#define     BUT2            (P2IN & BIT0)
#define     PRESSED         0
#define     RELEASED        1
#define     STR_SIZE        64


//======================Function prototypes================

BYTE checkInsertionRemoval (void);
VOID Init_Ports (VOID);
VOID Init_Clock (VOID);
VOID read_LUN1 (VOID);
VOID write_LUN1 (VOID);
void Init_Timer1 (void);
void Low_Power_Entering(void);

//The number of bytes per block.  In FAT, this is 512
const WORD BYTES_PER_BLOCK = 512;

volatile BYTE fS1ButtonEvent = 0;                                       //Flag indicating button S1 was pressed (that is, button S1
//on a F552x FET target board)
//extern const BYTE* Root_Dir;
//extern const BYTE* Data559;                // The data block of the storage volume, held in RAM
WORD DataCnt = 0;                                                       //The index of the next available byte to be written within
//Data[]
BYTE logCnt = 0x30;                                                     //Holds an incrementing character to be written into each
//new data log entry
//Data-exchange buffer between the API and the application.  The application allocates it,
//and then registers it with the API.  Later, the API will pass it back to the application when it
//needs the application to "process" it (exchange data with the media).
uint8_t RW_dataBuf[512];

//The API allocates an instance of structure type USBMSC_RWbuf_Info to hold all information describing
//buffers to be processed.  It is a shared resource between the API and application.  It will be initialized
//by calling USBMSC_fetchInfoStruct().
USBMSC_RWbuf_Info *RWbuf_info;

//The application must tell the API about the media.  This information is conveyed in a call to
//USBMSC_updateMediaInfo(), passing an instance of USBMSC_RWbuf_Info.  Since SD-cards are removable, this might
//need to occur at any time, so this is made global.
struct USBMSC_mediaInfoStr mediaInfo;

uint8_t bDetectCard = 0x00;                                             //Global flag by which the timer ISR will trigger main() to                                                                        //check the media status

VOID ASCII (char*);

volatile BYTE bHID_DataReceived_event = FALSE;                          //Flags set by event handler to indicate data has been
//received into USB buffer
volatile BYTE bCDC_DataReceived_event = FALSE;

#define BUFFER_SIZE 130
#define MAX_STR_LENGTH 60

char dataBuffer[BUFFER_SIZE];
char wholeString[MAX_STR_LENGTH] = "";
int temp=0;
extern volatile unsigned int results[Num_of_Results];
char str[STR_SIZE];
char str_temp[8];
volatile BYTE bCDCDataReceived_event = FALSE;   //Flag set by event handler to indicate data has been received into USB buffer
char nl[2] = "\n";
char Test_result_counter = 0;
unsigned char state = 0;
unsigned char CDC_write_flag = 0;
unsigned char k = 0;
unsigned char lock_flag = 0;
unsigned char start_up_flag;
volatile unsigned char LPM_flag=0;
volatile unsigned char but1_flag = RELEASED;
volatile unsigned char but2_flag = RELEASED;
unsigned int temp_ADC;

/*  
* ======== main ========
*/ 
void main (void)  
{
  WDTCTL = WDTPW + WDTHOLD;      //Stop the watchdog                          
  
  SetVCore(3);                                                            
  Init_Clock();                 //Init clocks  
  
  Init_Ports();                 //Init ports
  
  ADC_Init();                   //ADC init
  __bis_SR_register(GIE);
  Delay(5000);
  
  Init_Timer1();                //Init Timer 1 with external 32768 clock source 
  LCD_Init();
  
  Delay(10000);
  //    LCD_Send_Long_STR(1,200000,"MSP430-5510STK DEMO program!");       //show long strings 
  //    LCD_Clear();  
  LCD_Send_STR(1, "FLO&Xav"); 
  
  
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++      
  
  
  disk_initialize(0);                                                 //SD-cards must go through a setup sequence after powerup.
  //This FatFs call does this.
  USB_init();                                                         //Initializes the USB API, and prepares the USB module to
  //detect USB insertion/removal events
  USB_setEnabledEvents(kUSB_allUsbEvents);                            //Enable all USB events
  
  //The API maintains an instance of the USBMSC_RWbuf_Info structure.  If double-buffering were used, it would
  //maintain one for both the X and Y side.  (This version of the API only supports single-buffering,
  //so only one structure is maintained.)  This is a shared resource between the API and application; the
  //application must request the pointers.
  RWbuf_info = USBMSC_fetchInfoStruct();
  
  
  //USBMSC_updateMediaInfo() must be called prior to USB connection.  We check if the card is present, and if so, pull its size
  //and bytes per block.
  //LUN0
  mediaInfo.mediaPresent = 0x01;                                      //The medium is present, because internal flash is
  //non-removable.
  mediaInfo.mediaChanged = 0x00;                                      //It can't change, because it's in internal memory, which is
  //always present.
  mediaInfo.writeProtected = 0x00;                                    //It's not write-protected
  mediaInfo.lastBlockLba = 774;                                       //774 blocks in the volume. (This number is also found twice
  //in the volume itself; see mscFseData.c. They should
  //match.)
  mediaInfo.bytesPerBlock = BYTES_PER_BLOCK;                          //512 bytes per block. (This number is also found in the
  //volume itself; see mscFseData.c. They should match.)
  USBMSC_updateMediaInfo(0, &mediaInfo);
  
  //LUN1
  if (detectCard()){
    mediaInfo.mediaPresent = kUSBMSC_MEDIA_PRESENT;
  } 
  else {  
    mediaInfo.mediaPresent = kUSBMSC_MEDIA_NOT_PRESENT;
  }
  mediaInfo.mediaChanged = 0x00;
  mediaInfo.writeProtected = 0x00;
  disk_ioctl(0,GET_SECTOR_COUNT,&mediaInfo.lastBlockLba);             //Returns the number of blocks (sectors) in the media.
  mediaInfo.bytesPerBlock = BYTES_PER_BLOCK;                          //Block size will always be 512
  USBMSC_updateMediaInfo(1, &mediaInfo);
  
  //The data interchange buffer (used when handling SCSI READ/WRITE) is declared by the application, and
  //registered with the API using this function.  This allows it to be assigned dynamically, giving
  //the application more control over memory management.
  USBMSC_registerBufInfo(0, &RW_dataBuf[0], NULL, sizeof(RW_dataBuf));
  USBMSC_registerBufInfo(1, &RW_dataBuf[0], NULL, sizeof(RW_dataBuf));
  
  //Configure Timer_A0 to prompt detection of the SD card every second
  TA0CCTL0 = CCIE;                                                    //Enable interrupt
  TA0CCR0 = 32768;                                                    //Clock will be 32kHz, so we set the int to occur when it
  //counts to 32768
  TA0CTL = TASSEL_1 + MC_1 + TACLR;                                   //ACLK = 32kHz, up mode, clear TAR... go!
  
  
  //If USB is already connected when the program starts up, then there won't be a USB_handleVbusOnEvent().
  //So we need to check for it, and manually connect if the host is already present.
  if (USB_connectionInfo() & kUSB_vbusPresent){
    if (USB_enable() == kUSB_succeed){
      USB_reset();
      USB_connect();
    }
  }
  
  __enable_interrupt();                                               //Enable general interrupts
  
  while (1)
  {      
    /*      
    ADC10CTL0 &= ~ADC10ENC;                    // Disable conversions
    ADC10MCTL0 |= (ADC10INCH1 | ADC10INCH3);   //select 10 ADC chanel - temp sensor
    ADC10CTL0 |= ADC10ENC;                    // Enable conversions          
    temp = (unsigned long)results[0];
    temp = (temp*1.46 - 688)/2.52 - 20;                //V = 0.00252 x (TEMP ) + 0.688, TEMP = (temp*1500/1024 -688)/2.52 - 20
    sprintf(str_temp,"%d C",temp);                     //According to msp5510 datasheet - The temperature sensor offset can be as much as ±20°C. A single-point calibration is recommended in order to minimize the offset error of the built-in temperature sensor. 20 was subtracted from the result        
    LCD_Send_STR(1, str_temp);
    */
    //-------------------------Buttons scan bedin ---------------------------------------------------------      
    
    if (BUT1 == PRESSED && but1_flag == RELEASED)    //test if BUT1 is pressed   
    {
      ADC10CTL0 &= ~ADC10ENC;                       // Disable conversions
      ADC10MCTL0 &= ~(ADC10INCH0 | ADC10INCH1 | ADC10INCH2 | ADC10INCH3);   //select 0 ADC chanel - battery voltage
      ADC10CTL0 |= ADC10ENC;                        // Enable conversions
      temp = (unsigned long)results[0];
      temp = temp*4.58;                             //approximately 4.58 = (1500/1024)*(1470/470) - Battery voltage in mV
      sprintf(str_temp,"%d mV",temp);               //prepare string
      but1_flag = PRESSED;                                
      LCD_Send_STR(1, str_temp);
      TA1CCR0 = 2000;                               //change LED blinking period  
    }
    if (BUT1 == RELEASED && but1_flag == PRESSED)  //test if BUT1 is released
    {
      but1_flag = RELEASED;          
    }
    
    if (BUT2 == PRESSED && but2_flag == RELEASED)  //test if BUT2 is pressed
    {          
      but2_flag = PRESSED;                    
      LCD_Send_STR(1, " OLIMEX");
      TA1CCR0 = 6000;
    }
    if (BUT2 == RELEASED && but2_flag == PRESSED)  //test if BUT2 is released
    {
      but2_flag = RELEASED;          
    }
    
    //------------------------- Buttons scan end ---------------------------------------------------------      
    
    BYTE ReceiveError = 0, SendError = 0;
    WORD count;
    switch (USB_connectionState())
    {
    case ST_USB_DISCONNECTED:
      //                __bis_SR_register(LPM3_bits + GIE);                   //Enter LPM3 until VBUS-on event
      break;
      
    case ST_USB_CONNECTED_NO_ENUM:
      break;
      
    case ST_ENUM_ACTIVE:
      
      //Call USBMSC_poll() to initiate handling of any received
      //SCSI commands.  Disable interrupts
      //during this function, to avoid conflicts arising from SCSI
      //commands being received from the host
      //AFTER decision to enter LPM is made, but BEFORE it's
      //actually entered (in other words, avoid
      //sleeping accidentally).
      __disable_interrupt();
      if ((USBMSC_poll() == kUSBMSC_okToSleep) && (!bDetectCard)){
        __bis_SR_register(LPM0_bits + GIE);                 //Enable interrupts atomically with LPM0 entry
      }
      __enable_interrupt();
      
      
      //If the API needs the application to process a buffer, it
      //will keep the CPU awake by returning kUSBMSC_processBuffer
      //from USBMSC_poll().  The application should then check the
      //'operation' field of all defined USBMSC_RWbuf_Info
      //structure instances.  If any of them is non-null, then an
      //operation needs to be processed.  A value of
      //kUSBMSC_READ indicates the API is waiting for the
      //application to fetch data from the storage volume, in
      //response
      //to a SCSI READ command from the USB host.  After the
      //application does this, it must indicate whether the
      //operation succeeded, and then close the buffer operation
      //by calling USBMSC_bufferProcessed().
      
      while (RWbuf_info->operation == kUSBMSC_READ)
      {
        switch (RWbuf_info->lun)
        {
        case 0:
          RWbuf_info->returnCode = Read_LBA(RWbuf_info->lba,
                                            RWbuf_info->bufferAddr,
                                            RWbuf_info->lbCount);                       //Fetch a block from the medium, using file system emulation
          USBMSC_bufferProcessed();                   //Close the buffer operation
          break;
          
        case 1:
          read_LUN1();
          break;
        }
      }
      
      //Everything in this section is analogous to READs.
      //Reference the comments above.
      while (RWbuf_info->operation == kUSBMSC_WRITE)
      {
        switch (RWbuf_info->lun)
        {
        case 0:
          RWbuf_info->returnCode = Write_LBA(RWbuf_info->lba,
                                             RWbuf_info->bufferAddr,
                                             RWbuf_info->lbCount);                       //Write the block to the medium, using file system emulation
          USBMSC_bufferProcessed();                   //Close the buffer operation
          break;
        case 1:
          write_LUN1();
          break;
        }
      }
      
      //Every second, the Timer_A ISR sets this flag.  The
      //checking can't be done from within the timer ISR, because
      //the
      //checking enables interrupts, and this is not a recommended
      //practice due to the risk of nested interrupts.
      if (bDetectCard){
        checkInsertionRemoval();
        bDetectCard = 0x00;                                 //Clear the flag, until the next timer ISR
      }
      
      //---------------------------simple Echo begin-------------------------                
      
      if (bCDC_DataReceived_event)
      {                
        
        char dataBuffer[BUFFER_SIZE] = "";
        
        bCDC_DataReceived_event = FALSE;                             //Clear flag early -- just in case execution breaks
        //below because of an error
        count = cdcReceiveDataInBuffer((BYTE*)dataBuffer,
                                       BUFFER_SIZE,
                                       CDC0_INTFNUM);                                          //Count has the number of bytes received into
        //dataBuffer
        strcat(dataBuffer,nl);
        if (cdcSendDataInBackground((BYTE*)dataBuffer,count,CDC0_INTFNUM,1))
        {  	//Echo is back to the host
          SendError = 0x01;                                       			//Something went wrong -- exit
          break;
        }
      }
      
      //---------------------------simple Echo end----------------------------------------------                               
      
      break;                                                                                                              
      
    case ST_ENUM_SUSPENDED:
      //                __bis_SR_register(LPM3_bits + GIE);                     //Enter LPM3, until a resume or VBUS-off event
      break;
      
    case ST_ENUM_IN_PROGRESS:
      break;
      
    case ST_NOENUM_SUSPENDED:
      //                __bis_SR_register(LPM3_bits + GIE);
      break;
      
    case ST_ERROR:
      _NOP();
      break;
    default:;
    }
    if (ReceiveError || SendError){
      //TO DO: User can place code here to handle error
      if (cdcSendDataInBackground("\r\n SENDING ERROR\r\n",strlen("\r\n SENDING ERROR\r\n"),CDC0_INTFNUM,1));
    }
    
  }
}

/*  
* ======== ASCII ========
*/
//As a result, other apps capable of extended character sets may display Hyperterminal-originated characters incorrectly.  This
//function fixes this by blanking the most significant bit.
VOID ASCII (char* string)
{
  BYTE i = 0;
  BYTE len = strlen(string);
  
  while ((string[i] != 0x0A) && (string[i] != 0x0D) && (i < len))
  {
    string[i] = string[i] & 0x7F;
    i++;
  }
}

/*  
* ======== checkInsertionRemoval ========
* 
* This function checks for insertion/removal of the card.  If either is detected, it informs the API
* by calling USBMSC_updateMediaInfo().  Whether it detects it or not, it returns non-zero if the card
* is present, or zero if not present
*/
BYTE checkInsertionRemoval (void)
{
  BYTE newCardStatus = detectCard();      //Check card status -- there or not?
  
  if ((newCardStatus) &&
      (mediaInfo.mediaPresent == kUSBMSC_MEDIA_NOT_PRESENT)){
        //An insertion has been detected -- inform the API
        mediaInfo.mediaPresent = kUSBMSC_MEDIA_PRESENT;
        mediaInfo.mediaChanged = 0x01;
        DRESULT SDCard_result = disk_ioctl(0,
                                           GET_SECTOR_COUNT,
                                           &mediaInfo.lastBlockLba);       //Get the size of this new medium
        USBMSC_updateMediaInfo(1, &mediaInfo);
      }
  
  if ((!newCardStatus) && (mediaInfo.mediaPresent == kUSBMSC_MEDIA_PRESENT)){
    //A removal has been detected -- inform the API
    mediaInfo.mediaPresent = kUSBMSC_MEDIA_NOT_PRESENT;
    mediaInfo.mediaChanged = 0x01;
    USBMSC_updateMediaInfo(1, &mediaInfo);
  }
  
  return ( newCardStatus) ;
}

/*  
* ======== read_LUN1 ========
*/
VOID read_LUN1 (VOID)
{/*TODO added comment XB
  //A READ operation is underway, and the app has been requested to access the medium.  So, call file system to read
  //to do so.  Note this is a low level FatFs call -- we are not attempting to open a file ourselves.  The host is
  //in control of this access, we're just carrying it out.
  DRESULT dresult = disk_read(0,                                      //Physical drive number (0)
                              RWbuf_info->bufferAddr,                                         //Pointer to the user buffer
                              RWbuf_info->lba,                                                //First LBA of this buffer operation
                              RWbuf_info->lbCount);                                           //The number of blocks being requested as part of this
  
  //The result of the file system call needs to be communicated to the host.  Different file system software uses
  //different return codes, but they all communicate the same types of results.  This code ultimately gets passed to the
  //host app that issued the command to read (or if the user did it the host OS, perhaps in a dialog box).
  switch (dresult)
  {
  case RES_OK:
    RWbuf_info->returnCode = kUSBMSC_RWSuccess;
    break;
  case RES_ERROR:                     //In FatFs, this result suggests the medium may have been removed recently.
    if (!checkInsertionRemoval()){  //This application function checks for the SD-card, and if missing, calls
      //USBMSC_updateMediaInfo() to inform the API
      RWbuf_info->returnCode = kUSBMSC_RWMedNotPresent;
    }
    break;
  case RES_NOTRDY:
    RWbuf_info->returnCode = kUSBMSC_RWNotReady;
    break;
  case RES_PARERR:
    RWbuf_info->returnCode = kUSBMSC_RWLbaOutOfRange;
    break;
  }
  
  USBMSC_bufferProcessed();*/
}

/*  
* ======== write_LUN1 ========
*/
VOID write_LUN1 (VOID)
{
  DRESULT dresult = disk_write(0,         //Physical drive number (0)
                               RWbuf_info->bufferAddr,             //Pointer to the user buffer
                               RWbuf_info->lba,                    //First LBA of this buffer operation
                               RWbuf_info->lbCount);               //The number of blocks being requested as part of this operation
  
  switch (dresult)
  {
  case RES_OK:
    RWbuf_info->returnCode = kUSBMSC_RWSuccess;
    break;
  case RES_ERROR:
    if (!checkInsertionRemoval()){
      RWbuf_info->returnCode = kUSBMSC_RWMedNotPresent;
    }
    break;
  case RES_NOTRDY:
    RWbuf_info->returnCode = kUSBMSC_RWNotReady;
    break;
  case RES_PARERR:
    RWbuf_info->returnCode = kUSBMSC_RWLbaOutOfRange;
    break;
  default:
    RWbuf_info->returnCode = kUSBMSC_RWNotReady;
    break;
  }
  USBMSC_bufferProcessed();
}

/*  
* ======== TIMER0_A0_ISR ========
*/
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void)
{
  bDetectCard = 0x01;                                                 //Set the flag that will trigger main() to detect the card
  __bic_SR_register_on_exit(LPM0_bits);                               //Wake from ISR, if sleeping
}

/*  
* ======== Init_Clock ========
*/
VOID Init_Clock (VOID)
{
  if (USB_PLL_XT == 2){
#if defined (__MSP430F552x) || defined (__MSP430F550x )
    P5SEL |= 0x0C;                                      //enable XT2 pins for F5529
#elif defined (__MSP430F563x_F663x)
    P7SEL |= 0x0C;
#endif
    //--------------------------------------------------------------                        
    int timeout = 5000;
    //      P1DIR |= 0x01;                   //set P1.0 as output
    //      P1SEL |= 0x01;                   //set P1.0 as ACLK function
    
    P5DIR &= ~(BIT5 + BIT4);
    P5SEL |= BIT5 + BIT4;              //enable XT1 pins
    
    UCSCTL6 |= XCAP0 + XCAP1;
    UCSCTL6 = UCSCTL6 & (~XT1OFF);    //set XT1OFF=0 (bit0 = 0) --> enable XT1
    
    
    //use REFO for FLL 
    UCSCTL3 = (UCSCTL3 & ~(SELREF_7)) | (SELREF__REFOCLK);
    //use XT1CLK for ACLK
    UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | (SELA__XT1CLK);      
    
    do
    {
      if (!(UCSCTL7 & XT1LFOFFG)) break;
      UCSCTL7 = UCSCTL7 & ~(0x0F);            // Clear XT2,XT1,DCO fault flags
      SFRIFG1 = SFRIFG1 & ~(0x02);            // Clear fault flags (SFRIFG1.OFIFG = 0)
      timeout--;
    }
    while ((SFRIFG1 & OFIFG) && timeout);      // Test oscillator fault flag
    
    UCSCTL6 = (UCSCTL6 & ~(1<<6)) & ~(1<<7);  //XT1Drive = 00; --> oscilator range 4-8 MHz
    //--------------------------------------------------------------     
    
    
    
    //MCLK will be driven by the FLL (not by XT2), referenced to the REFO
    Init_FLL_Settle(USB_MCLK_FREQ / 1000, USB_MCLK_FREQ / 32768);         //Start the FLL, at the freq indicated by the config
    //constant USB_MCLK_FREQ
    
    XT2_Start(XT2DRIVE_0);                                                //Start the "USB crystal"
    
    
  } 
  else {
#if defined (__MSP430F552x) || defined (__MSP430F550x)
    P5SEL |= 0x10;                                      //enable XT1 pins
#endif
    //Use the REFO oscillator to source the FLL and ACLK
    UCSCTL3 = SELREF__REFOCLK;
    UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | (SELA__REFOCLK);
    
    //MCLK will be driven by the FLL (not by XT2), referenced to the REFO
    Init_FLL_Settle(USB_MCLK_FREQ / 1000, USB_MCLK_FREQ / 32768);       //set FLL (DCOCLK)
    
    XT1_Start(XT1DRIVE_0);                                              //Start the "USB crystal"
    
  }
}

/*  
* ======== Init_Ports ========
*/
VOID Init_Ports (VOID)
{
  //Initialization of ports (all unused pins as outputs with low-level
  P1REN |= BIT0;                //Enable BUT1 pullup
  P1OUT = 0x01;
  P1DIR = 0xFE;                 //LCD pins are outputs
  P2REN |= BIT0;                //Enable BUT2 pullup
  P2OUT = 0x01;
  P2DIR = 0x00;
  P4OUT = 0x00;
  P4DIR = 0x80;                 //LED oin is output    
  P5DS  = 0x02;                 //Increase drive strainght of P5.1(LCD-PWR)
  P5OUT = 0x02;                 //LCD_PWR_E, Battery measurement enable
  P5DIR = 0x03;
  P6OUT = 0x00;                 
  P6DIR = 0x02;                 //STNBY_E    
}

/*  
* ======== UNMI_ISR ========
*/

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
* ======== UNMI_ISR ========
*/
#pragma vector = UNMI_VECTOR
__interrupt VOID UNMI_ISR (VOID)
{
  switch (__even_in_range(SYSUNIV, SYSUNIV_BUSIFG ))
  {
  case SYSUNIV_NONE:
    __no_operation();
    break;
  case SYSUNIV_NMIIFG:
    __no_operation();
    break;
  case SYSUNIV_OFIFG:
    UCSCTL7 &= ~(DCOFFG + XT1LFOFFG + XT2OFFG); //Clear OSC flaut Flags fault flags
    SFRIFG1 &= ~OFIFG;                          //Clear OFIFG fault flag
    break;
  case SYSUNIV_ACCVIFG:
    __no_operation();
    break;
  case SYSUNIV_BUSIFG:
    //If bus error occured - the cleaning of flag and re-initializing of USB is
    //required.
    SYSBERRIV = 0;                              //clear bus error flag
    USB_disable();                              //Disable
  }
}

/*  
* ======== TIMER1_ISR ========
*/
#pragma vector = TIMER1_A0_VECTOR
__interrupt VOID TIMER1_A0_ISR (VOID)
{
  if (!(SFRIFG1 & OFIFG)) LED_TOGGLE;                 //check if the external 32768Hz crystal is failed
  else LED_ON;
}

/*  
* ======== LOW POWER ENTERING ========
*/
void Low_Power_Entering(void)
{
  //Initialization of ports (all unused pins as outputs with low-level
  
  P1OUT = 0x00;
  P1DIR = 0xFE;                 //LCD pins are outputs
  
  P2OUT = 0x00;
  P2DIR = 0x00;
  
  P4OUT = 0x00;                 //0b00000000
  P4DIR = 0xFF;                 //0b
  
  P5OUT = 0x00;                 //LCD_PWR_DIS
  P5DIR = 0xFF;
  
  P6SEL = 0x01;
  P6OUT = 0x00;                 
  P6DIR = 0xFC;                 //STNBY_E    
  
  PJOUT = 0x00;
  PJDIR = 0xFF;
  
  REFCTL0 = 0;                        //turn off internal VREF
  
  // Disable VUSB LDO and SLDO
  USBKEYPID   =     0x9628;           // set USB KEYandPID to 0x9628 
  // access to USB config registers enabled  
  USBPWRCTL &= ~(SLDOEN+VUSBEN);      // Disable the VUSB LDO and the SLDO
  USBKEYPID   =    0x9600;            // access to USB config registers disabled 
  
  __disable_interrupt();
  XT2_Stop();
  __bis_SR_register(LPM3_bits);       // Enter LPM3 
  __no_operation();
  
}
