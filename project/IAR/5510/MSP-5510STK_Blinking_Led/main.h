#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef __cplusplus
extern "C"
{
#endif

VOID Init_StartUp(VOID);
VOID Init_Ports(VOID);
VOID Init_Clocks(VOID);
BYTE retInString (char*);
VOID Init_TimerA1(VOID);


#ifdef __cplusplus
}
#endif
#endif