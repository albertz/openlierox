/////////////////////////////////////////
//
//         LieroX Game Script Compiler
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Main compiler
// Created 12/7/03
// Jason Boettcher


#ifndef __MAIN_H__
#define __MAIN_H__


// Routines
void    writeScript(void);
char    *newString(char *szString);
void    lx_strncpy(char *dest, char *src, int count);
char    *trimSpaces(char *szLine);
char    *lx_sprintf(char *dst, char *fmt, ...);
void    v_printf(char *fmt, ...);
void    writePascalString(char *szString, FILE *fp);
char    *readPascalString(char *szString, FILE *fp);



#endif  //  __MAIN_H__