/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// Error system
// Created 12/11/01
// By Jason Boettcher


#ifndef __ERROR_H__
#define	__ERROR_H__



void SetError(char *fmt, ...);
void ShowError(void);
void EndError(void);

void SystemError(char *fmt, ...);

void LxSetLastError(char *desc);
char *LxGetLastError(void);




#endif  //  __ERROR_H__