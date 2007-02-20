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

#include <vector>

#ifndef SYSTEM_DATA_DIR
#	define	SYSTEM_DATA_DIR	"/usr/share"
#endif

//
//	Drive types
//

// Windows
#ifdef WIN32
#define DRV_UNKNOWN		DRIVE_UNKNOWN		// The drive is unknown
#define DRV_NO_ROOT_DIR DRIVE_NO_ROOT_DIR	// The root path is invalid; for example, there is no volume is mounted at the path.
#define DRV_REMOVABLE	DRIVE_REMOVABLE		// The drive has removable media; for example, a floppy drive or flash card reader.
#define DRV_FIXED		DRIVE_FIXED			// The drive has fixed media; for example, a hard drive, flash drive, or thumb drive.
#define DRV_REMOTE		DRIVE_REMOTE		// The drive is a remote (network) drive.
#define DRV_CDROM		DRIVE_CDROM			// The drive is a CD-ROM drive.
#define DRV_RAMDISK		DRIVE_RAMDISK		// The drive is a RAM disk.

#endif


struct drive_t {
	std::string name;
	unsigned int type;
};

typedef std::vector<drive_t> drive_list;

void	AddToFileList(searchpathlist* l, const std::string& f);
bool	FileListIncludes(const searchpathlist* l, const std::string& f);

// this replaces ${var} in filename with concrete values
// currently, the following variables are handled:
//   ${HOME} - the home-dir, that means under unix ~  and under windows the 'my-documents'
//   ${BIN} - the dir of the executable-binary
//   ${SYSTEM_DATA} - data-dir of the system, that means usually /usr/share
void	ReplaceFileVariables(std::string& filename);

/*
// Routines
int		FindFirst(const std::string& dir, char *ext, char *filename);
int		FindNext(char *filename);

int		FindFirstDir(const std::string& dir, char *name);
int		FindNextDir(char *name);
*/

drive_list GetDrives(void);

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
// related to tLXOptions->tSearchPaths
FILE*	OpenGameFile(const std::string& path, const char *mode);

bool CanReadFile(const std::string& f, bool absolute = false);

// the dir will be created recursivly
// IMPORTANT: filename is absolute; no game-path!
void	CreateRecDir(const std::string& abs_filename, bool last_is_dir = true);

// copy the src-file to the dest
// it will simply fopen(src, "r"), fopen(dest, "w") and write all the stuff
// IMPORTANT: filenames are absolute; no game-path!
bool	FileCopy(const std::string& src, const std::string& dest);

// returns true, if we can write to the dir
bool	CanWriteToDir(const std::string& dir);

// returns the home-directory (used by ReplaceFileVariables)
std::string	GetHomeDir();
// returns the system-data-dir (under Linux, usually /usr/share)
std::string	GetSystemDataDir();
// returns the dir of the executable-binary
std::string	GetBinaryDir();
// returns the temp-dir of the system
std::string	GetTempDir();


typedef uchar filemodes_t;
enum {
	FM_DIR = 1,
	FM_REG = 2
};

// _handler has to be a functor with
// bool op() ( const std::string& path )
// ending pathsep is ensured if needed
// if return is false, it will break
template<typename _handler>
void ForEachSearchpath(_handler handler = _handler()) {
	searchpathlist::const_iterator i;
	for(
		i = tLXOptions->tSearchPaths.begin();
		i != tLXOptions->tSearchPaths.end(); i++) {
		if(!handler(*i + "/")) return;
	}
	for(
		i = basesearchpaths.begin();
		i != basesearchpaths.end(); i++) {
		if(!FileListIncludes(&tLXOptions->tSearchPaths, *i))
			if(!handler(*i + "/")) return;
	}
	handler("./");
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
	
	inline bool operator() (const std::string& path) {
		std::string abs_path = path + dir;
		bool ret = true;
		
#ifdef WIN32
		struct _finddata_t fileinfo;
		long handle = _findfirst(abs_path.c_str(), &fileinfo);
		if(handle < 0) return ret;
		while(!_findnext(handle, &fileinfo)) {
			//If file is not self-directory or parent-directory
			if(fileinfo.name[0] != '.' || (fileinfo.name[1] != '\0' && (fileinfo.name[1] != '.' || fileinfo.name[2] != '\0'))) {
				if((!(fileinfo.attrib&_A_SUBDIR) && modefilter&FM_REG)
				|| fileinfo.attrib&_A_SUBDIR && modefilter&FM_DIR)
					if(!filehandler(dir + "/" + fileinfo.name)) {
						ret = false;
						break;
					}
			}
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
				if(stat(filename.c_str(), &s) != 0)
					if((S_ISREG(s.st_mode) && modefilter&FM_REG)
					|| (S_ISDIR(s.st_mode) && modefilter&FM_DIR))
						if(!filehandler(dir + "/" + entry->d_name)) {
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
	_handler handler,
	const std::string& dir,
	const filemodes_t modefilter = -1,
	const std::string& namefilter = ""
) {
	if(namefilter != "*" && namefilter != "")
		printf("FindFiles: WARNING: filter %s isn't handled yet\n", namefilter.c_str());
	ForEachSearchpath(FindFilesHandler<_handler>(dir, namefilter, modefilter, handler));
}


#endif  //  __FINDFILE_H__
