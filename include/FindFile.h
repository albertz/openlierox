/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   work by JasonB
//   code under LGPL
//   enhanced by Dark Charlie and Albert Zeyer
//
/////////////////////////////////////////


// File finding routines
// Created 30/9/01
// By Jason Boettcher

// TODO: rename this file

#ifndef __FINDFILE_H__
#define __FINDFILE_H__


#ifndef SYSTEM_DATA_DIR
#	define	SYSTEM_DATA_DIR	"/usr/share"
#endif


struct filelist_t {
	std::string filename;
	filelist_t* next;
}; 

void	AddToFileList(filelist_t** l, const std::string f);
bool	FileListIncludes(const filelist_t* l, const std::string f);

// this replaces ${var} in filename with concrete values
// currently, the following variables are handled:
//   ${HOME} - the home-dir, that means under unix ~  and under windows the 'my-documents'
//   ${BIN} - the dir of the executable-binary
//   ${SYSTEM_DATA} - data-dir of the system, that means usually /usr/share
void	ReplaceFileVariables(std::string& filename);

// Routines
int		FindFirst(char *dir, char *ext, char *filename);
int		FindNext(char *filename);

int		FindFirstDir(char *dir, char *name);
int		FindNextDir(char *name);

#ifndef WIN32

// mostly all system but Windows use case sensitive file systems
// this game uses also filenames ignoring the case sensitivity
// this function gives the case sensitive right name of a file
// also, it replaces ${var} in the searchname
// returns false if no success, true else
bool GetExactFileName(const std::string abs_searchname, std::string& filename);

#else // WIN32

// we don't have case sensitive file systems under windows
// but we still need to replace ${var} in the searchname
// returns true, if file/dir is existing and accessable, false else
inline bool GetExactFileName(const std::string abs_searchname, std::string& filename) {
	if(abs_searchname.size() == 0) {
		filename = "";
		return false;
	}
	
	filename = abs_searchname;
	ReplaceFileVariables(filename);

	// Return false, if file doesn't exist
	// TODO: it should also return true for directories
	FILE *f = fopen(filename.c_str(),"r");
	if (!f)
		return false;
	fclose(f);

	return true;
}
#endif



extern filelist_t*	basesearchpaths;
void	InitBaseSearchPaths();

// this does a search on all searchpaths for the file and returns the first one found
// if none was found, NULL will be returned
// if searchpath!=NULL, it will place there the searchpath
std::string GetFullFileName(const std::string path, std::string* searchpath = NULL);

// this give always a dir like searchpath[0]/path, but it ensures:
// - the filename is correct, if the file exists
// - it replaces ${var} with ReplaceFileVariables
// if create_nes_dirs is set, the nessecary dirs will be created
std::string GetWriteFullFileName(const std::string path, bool create_nes_dirs = false);

// replacement for the simple fopen
// this does a search on all searchpaths for the file and opens the first one; if none was found, NULL will be returned
// related to tLXOptions->tSearchPaths
FILE*	OpenGameFile(const std::string path, const char *mode);

// the dir will be created recursivly
// IMPORTANT: filename is absolute; no game-path!
void	CreateRecDir(const std::string abs_filename, bool last_is_dir = true);

// copy the src-file to the dest
// it will simply fopen(src, "r"), fopen(dest, "w") and write all the stuff
// IMPORTANT: filenames are absolute; no game-path!
bool	FileCopy(const std::string src, const std::string dest);

// returns true, if we can write to the dir
bool	CanWriteToDir(const std::string dir);

// returns the home-directory (used by ReplaceFileVariables)
std::string	GetHomeDir();
// returns the system-data-dir (under Linux, usually /usr/share)
std::string	GetSystemDataDir();
// returns the dir of the executable-binary
std::string	GetBinaryDir();
// returns the temp-dir of the system
std::string	GetTempDir();

#endif  //  __FINDFILE_H__
