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


#ifndef WIN32
// mostly all system but Windows use case sensitive file systems
// this game uses also filenames ignoring the case sensitivity
// this function results the case sensitive right name of a file
int GetExactFileName(const char* searchname, char* filename)
#endif

#endif  //  __FINDFILE_H__
