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
// this function gives the case sensitive right name of a file
// returns false if no success, true else
int GetExactFileName(const char* searchname, char* filename);
#else // WIN32
// on Windows, we don't need it, so does a simple strcpy
inline int GetExactFileName(const char* searchname, char* filename) {
	strcpy(filename, searchname);
	// TODO: return false, if file doesn't exist
	return true;
}
#endif

// case insensitive fopen
inline FILE *fopen_i(const char *path, const char *mode) {
#ifndef WIN32	
	static char fname[256] = "";
//	printf("fopen %s\n", path);
	GetExactFileName(path, fname);
//	printf("fopen -> %s\n", fname);
	return fopen(fname, mode);
#else
	return fopen(path, mode);
#endif
}


#endif  //  __FINDFILE_H__
