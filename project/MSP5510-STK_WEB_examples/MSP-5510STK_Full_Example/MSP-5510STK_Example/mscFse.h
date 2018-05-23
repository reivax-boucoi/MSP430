//(c)2010 by Texas Instruments Incorporated, All Rights Reserved.
/*----------------------------------------------------------------------------+
 |                                                                             |
 |                              Texas Instruments                              |
 |                                                                             |
 |                          MSP430 USB-Example (MSC Driver)                    |
 |                                                                             |
 +-----------------------------------------------------------------------------+
 |  Source: Msc_user.h, File Version 1.01                                       |
 |  Description: This file contains declarations of structure,APIs to be       |
 |               updated by user.                                              |
 |                                                                             |
 |  Author: Biju,MSP                                                           |
 |                                                                             |
 |  WHO          WHEN         WHAT                                             |
 |  ---          ----------   ------------------------------------------------ |
 |  Biju, MSP    2010/06/08  born                                              |
 |                                                                             |
 +----------------------------------------------------------------------------*/

#ifndef _MSC_USER_H_
#define _MSC_USER_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*----------------------------------------------------------------------------+
 | Function Prototypes                                                         |
 +----------------------------------------------------------------------------*/
BYTE Read_LBA (DWORD LBA, BYTE *buff,  BYTE lbaCount);
BYTE Write_LBA (DWORD LBA, BYTE *buff, BYTE lbaCount);
VOID flashWrite_LBA (PBYTE flashAddr, BYTE* data);


#ifdef __cplusplus
}
#endif
#endif  /* _MSC_USER_H_ */

