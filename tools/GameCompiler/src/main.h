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
char    *newString(const char* szString);
void    lx_strncpy(const char* dest, const char* src, int count);
char    *trimSpaces(const char* szLine);
char    *lx_sprintf(const char* dst, const char* fmt, ...);
void    v_printf(const char* fmt, ...);
void    writePascalString(const char* szString, FILE *fp);
char    *readPascalString(const char* szString, FILE *fp);



#endif  //  __MAIN_H__
