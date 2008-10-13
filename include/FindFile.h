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

#include <fstream>
#include <string>
#include <list>
#include <vector>
#include "Unicode.h"

#ifndef WIN32
#	include <dirent.h>
#	include <unistd.h>
#endif

// file input/output
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#	include <windows.h>
#	include <io.h>
#	include <direct.h>
	// wrappers to provide the standards
	inline int mkdir(const char *path, int mode) { return _mkdir(Utf8ToSystemNative(path).c_str()); }
#	define stat _stat
#ifndef S_ISREG
inline bool S_ISREG(unsigned short s)  { return (s & S_IFREG) != 0; }
inline bool S_ISDIR(unsigned short d)  { return (d & S_IFDIR) != 0; }
#endif
#endif


#ifndef SYSTEM_DATA_DIR
#	define	SYSTEM_DATA_DIR	"/usr/share"
#endif

extern	std::string		binary_dir;

//
//	Drive types
//

// Windows
#ifdef WIN32
#	define DRV_UNKNOWN		DRIVE_UNKNOWN		// The drive is unknown
#	define DRV_NO_ROOT_DIR DRIVE_NO_ROOT_DIR	// The root path is invalid; for example, there is no volume is mounted at the path.
#	define DRV_REMOVABLE	DRIVE_REMOVABLE		// The drive has removable media; for example, a floppy drive or flash card reader.
#	define DRV_FIXED		DRIVE_FIXED			// The drive has fixed media; for example, a hard drive, flash drive, or thumb drive.
#	define DRV_REMOTE		DRIVE_REMOTE		// The drive is a remote (network) drive.
#	define DRV_CDROM		DRIVE_CDROM			// The drive is a CD-ROM drive.
#	define DRV_RAMDISK		DRIVE_RAMDISK		// The drive is a RAM disk.
#endif


class drive_t { public:
	std::string name;
	unsigned int type;
};

typedef std::vector<drive_t> drive_list;

// Define intptr_t if not defined
#ifdef _MSC_VER
#ifndef _INTPTR_T_DEFINED
typedef long intptr_t;
#endif
#endif

typedef std::vector<std::string> searchpathlist;
extern searchpathlist tSearchPaths;

void	AddToFileList(searchpathlist* l, const std::string& f);
bool	FileListIncludesExact(const searchpathlist* l, const std::string& f);

void	initSpecialSearchPathForTheme();
const std::string*	getSpecialSearchPathForTheme();

// this replaces ${var} in filename with concrete values
// currently, the following variables are handled:
//   ${HOME} - the home-dir, that means under unix ~  and under windows the 'my-documents'
//   ${BIN} - the dir of the executable-binary
//   ${SYSTEM_DATA} - data-dir of the system, that means usually /usr/share
void	ReplaceFileVariables(std::string& filename);

drive_list GetDrives(void);

// This function converts relative paths to absolute paths
std::string GetAbsolutePath(const std::string& path);

#ifndef WIN32

// mostly all system but Windows use case sensitive file systems
// this game uses also filenames ignoring the case sensitivity
// this function gives the case sensitive right name of a file
// also, it replaces ${var} in the searchname
// returns false if no success, true else
bool GetExactFileName(const std::string& abs_searchname, std::string& filename);

#else // WIN32

// we don't have case sensitive file systems under windows
// but we still need to replace ${var} in the searchname
// returns true, if file/dir is existing and accessable, false else

inline bool GetExactFileName(const std::string& abs_searchname, std::string& filename) {
	filename = abs_searchname;

	if(abs_searchname.size() == 0) {
		return true;
	}

	ReplaceFileVariables(filename);

	// Remove the ending slash, else stat will fail
	if (filename[filename.length()-1]== '/' || filename[filename.length()-1]== '\\')
		// Don't remove, if this is a root directory, else stat will fail (really crazy)
#ifdef WIN32
		if (filename[filename.length()-2] != ':')
#endif
			filename.erase(filename.length()-1);

	struct stat finfo;

	if(stat(Utf8ToSystemNative(filename).c_str(), &finfo) != 0) {
		// problems stating file
		return false;
	}

	// we got some info, so there is something ...
	return true;
}
#endif



extern searchpathlist	basesearchpaths;
void	InitBaseSearchPaths();

// this does a search on all searchpaths for the file and returns the first one found
// if none was found, NULL will be returned
// if searchpath!=NULL, it will place there the searchpath
std::string GetFullFileName(const std::string& path, std::string* searchpath = NULL);

// this give always a dir like searchpath[0]/path, but it ensures:
// - the filename is correct, if the file exists
// - it replaces ${var} with ReplaceFileVariables
// if create_nes_dirs is set, the nessecary dirs will be created
std::string GetWriteFullFileName(const std::string& path, bool create_nes_dirs = false);

// replacement for the simple fopen
// this does a search on all searchpaths for the file and opens the first one; if none was found, NULL will be returned
// related to tSearchPaths
FILE*	OpenGameFile(const std::string& path, const char *mode);

std::ifstream* OpenGameFileR(const std::string& path);

std::string GetFileContents(const std::string& path, bool absolute = false);
std::string ExtractDirectory(const std::string& path);
std::string JoinPaths(const std::string& path1, const std::string& path2);

bool IsFileAvailable(const std::string& f, bool absolute = false);

// the dir will be created recursivly
// IMPORTANT: filename is absolute; no game-path!
void	CreateRecDir(const std::string& abs_filename, bool last_is_dir = true);

bool	EqualPaths(const std::string& path1, const std::string& path2);

// copy the src-file to the dest
// it will simply fopen(src, "r"), fopen(dest, "w") and write all the stuff
// IMPORTANT: filenames are absolute; no game-path!
bool	FileCopy(const std::string& src, const std::string& dest);

// returns true, if we can write to the dir
bool	CanWriteToDir(const std::string& dir);

size_t	FileSize(const std::string& path);

// returns the home-directory (used by ReplaceFileVariables)
std::string	GetHomeDir();
// returns the system-data-dir (under Linux, usually /usr/share)
std::string	GetSystemDataDir();
// returns the dir of the executable-binary
std::string	GetBinaryDir();
// returns the temp-dir of the system
std::string	GetTempDir();


typedef char filemodes_t;
enum {
	FM_DIR = 1,
	FM_REG = 2
};

bool PathListIncludes(const std::list<std::string>& list, const std::string& path);

// _handler has to be a functor with
// bool op() ( const std::string& path )
// ending pathsep is ensured if needed
// if return is false, it will break
// HINT: it does no GetExactFileName with the paths
template<typename _handler>
void ForEachSearchpath(_handler& handler) {
	std::list<std::string> handled_dirs;
	std::string path;
	searchpathlist::const_iterator i;

	{
		const std::string* themeDir = getSpecialSearchPathForTheme();
		if(themeDir) {
			if(!handler(*themeDir + "/")) return;
			handled_dirs.push_back(*themeDir);
		}
	}

	for(
			i = tSearchPaths.begin();
			i != tSearchPaths.end(); i++) {
		path = *i;
		if(!PathListIncludes(handled_dirs, path)) {
			if(!handler(path + "/")) return;
			handled_dirs.push_back(path);
		}
	}

	for(
			i = basesearchpaths.begin();
			i != basesearchpaths.end(); i++) {
		if(!FileListIncludesExact(&tSearchPaths, *i)) {
			path = *i;
			if(!PathListIncludes(handled_dirs, path)) {
				if(!handler(path + "/")) return;
				handled_dirs.push_back(path);
			}
		}
	}
}


// functor for ForEachSearchpath, used by FindFiles
// it will search a subdir of a given searchpath for files
template<typename _filehandler>
class FindFilesHandler {
public:
	const std::string& dir;
	const std::string& namefilter;
	const filemodes_t modefilter;
	_filehandler& filehandler;

	FindFilesHandler(
			const std::string& dir_,
			const std::string& namefilter_,
			const filemodes_t modefilter_,
			_filehandler& filehandler_) :
		dir(dir_),
		namefilter(namefilter_),
		modefilter(modefilter_),
		filehandler(filehandler_) {}

	bool operator() (const std::string& path) {
		std::string abs_path = path;
		if(!GetExactFileName(path + dir, abs_path)) return true;
		bool ret = true;

#ifdef WIN32  // uses UTF16
		struct _finddata_t fileinfo;
		abs_path.append("/");
		intptr_t handle = _findfirst(Utf8ToSystemNative(abs_path + "*").c_str(), &fileinfo);
		while(handle > 0) {
			//If file is not self-directory or parent-directory
			if(fileinfo.name[0] != '.' || (fileinfo.name[1] != '\0' && (fileinfo.name[1] != '.' || fileinfo.name[2] != '\0'))) {
				if((!(fileinfo.attrib&_A_SUBDIR) && modefilter&FM_REG)
				|| fileinfo.attrib&_A_SUBDIR && modefilter&FM_DIR)
					if(!filehandler(abs_path + SystemNativeToUtf8(fileinfo.name))) {
						ret = false;
						break;
					}
			}

			if(_findnext(handle,&fileinfo))
				break;
		}
#else /* not WIN32 */

		std::string filename;
		dirent* entry;
		struct stat s;
		DIR* handle = opendir(abs_path.c_str());
		if(!handle) return ret;
		while((entry = readdir(handle)) != 0) {
			//If file is not self-directory or parent-directory
			if(entry->d_name[0] != '.' || (entry->d_name[1] != '\0' && (entry->d_name[1] != '.' || entry->d_name[2] != '\0'))) {
				filename = abs_path + "/" + entry->d_name;
				if(stat(filename.c_str(), &s) == 0)
					if((S_ISREG(s.st_mode) && modefilter&FM_REG)
					|| (S_ISDIR(s.st_mode) && modefilter&FM_DIR))
						if(!filehandler(filename)) {
							ret = false;
							break;
						}
			}
		}
		closedir(handle);
#endif /* WIN32 */
		return ret;
	}
};

// FindFiles searches for files
// _handler has to be a functor with
// bool op()( const std::string& abs_filename )
// if it returns false, it will break
template<typename _handler>
void FindFiles(
	_handler& handler,
	const std::string& dir,
	bool absolutePath = false,
	const filemodes_t modefilter = -1,
	const std::string& namefilter = ""
) {
	if(namefilter != "*" && namefilter != "")
		printf("FindFiles: WARNING: filter %s isn't handled yet\n", namefilter.c_str());
	if(absolutePath)
		FindFilesHandler<_handler>(dir, namefilter, modefilter, handler) ("");
	else {
		FindFilesHandler<_handler> searchpathHandler(dir, namefilter, modefilter, handler);
		ForEachSearchpath(searchpathHandler);
	}
}


// File pointer to SDL RWops conversion
SDL_RWops *RWopsFromFP(FILE *fp, bool autoclose);

// Platform-independent stat() - use S_ISREG( st.st_mode ) and S_ISDIR( st.st_mode ) on stat struct
inline bool StatFile( const std::string & file, struct stat * st )
{
	std::string fname = GetFullFileName( file );
	std::string exactfname;
	if( ! GetExactFileName( fname, exactfname ) )
		return false;
	if( stat( Utf8ToSystemNative(fname).c_str(), st ) != 0 )
		return false;

	return true;
};

#endif  //  __FINDFILE_H__
