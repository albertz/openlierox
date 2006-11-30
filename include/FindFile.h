/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// File finding routines
// Created 30/9/01
// By Jason Boettcher

// TODO: rename this file

#ifndef __FINDFILE_H__
#define __FINDFILE_H__


#ifndef SYSTEM_DATA_DIR
#	define	SYSTEM_DATA_DIR	"/usr/share/"
#endif


struct filelist_t {
	char filename[64]; // TODO: enough?
	filelist_t* next;
}; 

void	AddToFileList(filelist_t** l, const char* f);
bool	FileListIncludes(const filelist_t* l, const char* f);


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
bool GetExactFileName(const char* searchname, char* filename);

#else // WIN32

// on Windows, we don't need it, so does a simple strcpy
inline bool GetExactFileName(const char* searchname, char* filename) {
	if(searchname == NULL) {
		if(filename != NULL)
			filename[0] = '\0';
		return false;
	}
	
	strcpy(filename, searchname);

	// Return false, if file doesn't exist
	// TODO: it should also not return false for directories
	FILE *f = fopen(searchname,"r");
	if (!f)
		return false;
	fclose(f);

	return true;
}
#endif



extern filelist_t*	basesearchpaths;
void	InitBaseSearchPaths();

// this does a search on all searchpaths for the file and returns the first one found; if none was found, NULL will be returned
char*	GetFullFileName(const char* path);

// replacement for the simple fopen
// this does a search on all searchpaths for the file and opens the first one; if none was found, NULL will be returned
// related to tLXOptions->tSearchPaths
FILE*	OpenGameFile(const char *path, const char *mode);

// returns the gamedir in the home-directory (on unix: ~/.OpenLieroX)
char*	GetHomeDir();

// the dir will be created recursivly
void	CreateRecDir(char* f);


#endif  //  __FINDFILE_H__
