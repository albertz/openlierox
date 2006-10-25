/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// File finding routines
// Created 30/9/01
// By Jason Boettcher


#ifndef __FINDFILE_H__
#define __FINDFILE_H__


// Routines
int		FindFirst(char *dir, char *ext, char *filename);
int		FindNext(char *filename);

int		FindFirstDir(char *dir, char *name);
int		FindNextDir(char *name);




#endif  //  __FINDFILE_H__
