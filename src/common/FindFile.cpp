/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// File finding routines
// Created 30/9/01
// By Jason Boettcher


#include <iostream>

#ifdef _MSC_VER
#pragma warning(disable: 4786)  // WARNING: identifier XXX was truncated to 255 characters in the debug info
#pragma warning(disable: 4503)  // WARNING: decorated name length exceeded, name was truncated
#endif

#include "FindFile.h"
#include "StringUtils.h"


#ifdef WIN32
#	ifndef _WIN32_IE
// Some functions that are required are unavailable if this is not defined:
// TODO: which functions?
#   define  _WIN32_IE  0x0400  // Because of Dev-cpp
#	endif

#	include <shlobj.h>

#else // WIN32

// include hash_set support
#	ifndef STLPORT
#		include <ext/hash_set>
#		ifdef _GLIBCXX_DEBUG
			using namespace __gnu_debug_def;
#		else			
			using namespace __gnu_cxx;
#		endif		
#	else // !STLPORT
#		include <hash_set>
		using std::hash_set;
#	endif // !STLPORT

// for getpwduid
#	include <pwd.h>

// for realpath
#	include <sys/param.h>
#	include <unistd.h>

#endif


searchpathlist tSearchPaths;

using namespace std;

bool IsFileAvailable(const std::string& f, bool absolute) {
	static std::string abs_f;
	if(absolute) {
		abs_f = f;
	} else
		if((abs_f = GetFullFileName(f)) == "") return false;

	// remove trailing slashes
	// don't remove them on WIN, if it is a drive-letter
	while(abs_f.size() > 0 && (abs_f[abs_f.size()-1] == '\\' || abs_f[abs_f.size()-1] == '/')) {
#ifdef WIN32
		if(abs_f.size() > 2 && abs_f[abs_f.size()-2] == ':') break;
#endif
		abs_f.erase(abs_f.size()-1);
	}

	// HINT: this should also work on WIN32, as we have _stat here
	struct stat s;
#ifdef WIN32  // uses UTF16
	if(wstat((wchar_t *)Utf8ToUtf16(abs_f).c_str(), &s) != 0 || !S_ISREG(s.st_mode)) {
#else // not win32
	if(stat(abs_f.c_str(), &s) != 0 || !S_ISREG(s.st_mode)) {
#endif
		// it's not stat-able or not a reg file
		return false;
	}

	// it's stat-able and a file
	return true;
}



/*

	Drives

*/

////////////////////
//
drive_list GetDrives(void)
{
static drive_list list;
list.clear();
#ifdef WIN32
	static char drives[34];
	int len = GetLogicalDriveStrings(sizeof(drives),drives); // Get the list of drives
	drive_t tmp;
	if (len)  {
		for (register int i=0; i<len; i+=(int)strnlen(&drives[i],4)+1)  {
			// Create the name (for example: C:\)
			tmp.name = &drives[i];
			// Get the type
			tmp.type = GetDriveType((LPCTSTR)tmp.name.c_str());
			// Add to the list
			list.push_back(tmp);
		}
	}


#else
	// there are not any drives on Linux/Unix/MacOSX/...
	// it's only windows which uses this crazy drive-letters
	
	// perhaps not the best way
	// home-dir of user is in other applications the default
	// but it's always possible to read most other stuff
	// and it's not uncommon that a user hase a shared dir like /mp3s
	drive_t tmp;
	tmp.name = "/";
	tmp.type = 0;
	list.push_back(tmp);
	
	// we could communicate with dbus and ask it for all connected
	// and mounted hardware-stuff
#endif
 
	return list;
}


#ifndef WIN32

// checks, if path is statable (that means, it's existing)
// HINT: absolute path and there is not case fixing
// (used by GetExactFileName)
bool IsPathStatable(const std::string& f) {
	static std::string abs_f;
	abs_f = f;

	// remove trailing slashes
	// don't remove them on WIN, if it is a drive-letter
	while(abs_f.size() > 0 && (abs_f[abs_f.size()-1] == '\\' || abs_f[abs_f.size()-1] == '/')) {
#ifdef WIN32
		if(abs_f.size() > 2 && abs_f[abs_f.size()-2] == ':') break;
#endif
		abs_f.erase(abs_f.size()-1);
	}

	// HINT: this should also work on WIN32, as we have _stat here
	struct stat s;
#ifdef WIN32  // uses UTF16
	return (wstat(Utf8ToUtf16(abs_f).c_str(), &s) == 0); // ...==0, if successfull
#else // other systems
	return (stat(abs_f.c_str(), &s) == 0); // ...==0, if successfull
#endif
}


// used by unix-GetExactFileName
// HINT: it only reads the first char of the seperators
// it returns the start of the subdir (the pos _after_ the sep.)
// HINT: it returns position in bytes, not in characters
size_t GetNextName(const std::string& fullname, const char** seperators, std::string& nextname)
{
	std::string::const_iterator pos;
	size_t p = 0;
	unsigned short i;

	for(pos = fullname.begin(); pos != fullname.end(); pos++, p++) {
		for(i = 0; seperators[i] != NULL; i++)
			if(*pos == seperators[i][0]) {
				nextname = fullname.substr(0, p);
				return p + 1;
			}
	}

	nextname = fullname;
	return 0;
}

// get ending filename of a path
size_t GetLastName(const std::string& fullname, const char** seperators)
{
	std::string::const_reverse_iterator pos;
	size_t p = fullname.size()-1;
	unsigned short i;

	for(pos = fullname.rbegin(); pos != fullname.rend(); pos++, p--) {
		for(i = 0; seperators[i] != NULL; i++)
			if(*pos == seperators[i][0]) {
				return p;
			}
	}

	// makes no difference if we return 0 here
	return 0;
}

// used by unix-GetExactFileName
// does a case insensitive search for searchname in dir
// sets filename to the first search result
// returns true, if any file found
bool CaseInsFindFile(const std::string& dir, const std::string& searchname, std::string& filename) {
	if(searchname == "") {
		filename = "";
		return true;
	}

	// TODO: unicode support!!

	DIR* dirhandle = opendir((dir == "") ? "." : dir.c_str());
	if(dirhandle == 0) return false;

	dirent* direntry;
	while((direntry = readdir(dirhandle))) {
		if(strcasecmp(direntry->d_name, searchname.c_str()) == 0) {
			filename = direntry->d_name;
			closedir(dirhandle);
#ifdef DEBUG
			if(filename != searchname)
				cerr << "filename case mismatch: " << searchname << " <-> " << filename << endl;
#endif
			return true;
		}
	}

	closedir(dirhandle);
	return false;
}

class strcasecomparer {
public:
	inline bool operator()(const std::string& str1, const std::string& str2) const {
		return stringcasecmp(str1, str2) == 0;
	}
};

typedef hash_set<std::string, simple_reversestring_hasher, strcasecomparer> exactfilenamecache_t;
exactfilenamecache_t exactfilenamecache;

bool is_searchname_in_exactfilenamecache(
		const std::string& searchname,
		std::string& exactname
) {
	exactfilenamecache_t::iterator it = exactfilenamecache.find(searchname);
	if(it != exactfilenamecache.end()) {
		exactname = *it;
		return true;
	} else
		return false;	
}

void add_searchname_to_exactfilenamecache(const std::string& exactname) {
	exactfilenamecache.insert(exactname);
}


// does case insensitive search for file
bool GetExactFileName(const std::string& abs_searchname, std::string& filename) {
	const char* seps[] = {"\\", "/", (char*)NULL};
	if(abs_searchname.size() == 0) {
		filename = "";
		return false;
	}

	std::string sname = abs_searchname;
	ReplaceFileVariables(sname);
	
	std::string nextname = "";
	std::string nextexactname = "";
	size_t pos;

	bool first_iter = true; // this is used in the bottom loop

	// search in cache
	
	// sname[0..pos-1] is left rest, excluding the /
	pos = sname.size();
	std::string rest;
	while(true) {
		rest = sname.substr(0,pos);
		if(is_searchname_in_exactfilenamecache(rest, filename)) {
			if(IsPathStatable(filename)) {
				if(pos == sname.size()) // do we got the whole filename?
					return true;
				
				// filename is the correct one here			
				sname.erase(0,pos+1);
				first_iter = false; // prevents the following loop from not adding a "/" to filename
				break;
			}
		}
		pos = GetLastName(rest, seps);
		if(pos == 0) {
			filename = "";
			break;
		}
	}
	


	// search the filesystem for the name

	// sname contains the rest of the path
	// filename contains the start (including a "/" if necces.)
	// if first_iter is set to true, don't add leading "/"
	while(true) {
		pos = GetNextName(sname, seps, nextname);
		// pos>0  => found a sep (pos is right behind the sep)
		// pos==0  => none found
		if(pos > 0) sname.erase(0,pos);
		
		filename += (first_iter ? "" : "/");
		if(nextname == "") {
			// simply ignore this case
			// (we accept sth like /usr///share/)
			nextexactname = "";
		} else if(!CaseInsFindFile(
				filename, // dir
				nextname, // ~name
				nextexactname // resulted name
		)) {
			// we doesn't get any result
			// just add rest to it
			filename += nextname;
			if(pos > 0) filename += "/" + sname; 
			return false; // error (not found)
		}
		
		filename += nextexactname;
		if(nextexactname != "")
			add_searchname_to_exactfilenamecache(filename);

		if(pos == 0) break;
		first_iter = false;
	}

	return true;
}

#endif // not WIN32


searchpathlist	basesearchpaths;
void InitBaseSearchPaths() {
	basesearchpaths.clear();
#if defined(MACOSX)
	AddToFileList(&basesearchpaths, "${HOME}/Library/Application Support/OpenLieroX");
	AddToFileList(&basesearchpaths, ".");
	AddToFileList(&basesearchpaths, "${BIN}/../Resources/gamedir");
	AddToFileList(&basesearchpaths, SYSTEM_DATA_DIR"/OpenLieroX");
#elif defined(WIN32)
	AddToFileList(&basesearchpaths, "${HOME}/OpenLieroX");
	AddToFileList(&basesearchpaths, ".");
	AddToFileList(&basesearchpaths, "${BIN}");
#else // all other systems (Linux, *BSD, OS/2, ...)
	AddToFileList(&basesearchpaths, "${HOME}/.OpenLieroX");
	AddToFileList(&basesearchpaths, ".");
	AddToFileList(&basesearchpaths, SYSTEM_DATA_DIR"/OpenLieroX"); // no use of ${SYSTEM_DATA}, because it is uncommon and could cause confusion to the user
#endif
}

void CreateRecDir(const std::string& abs_filename, bool last_is_dir) {
	static std::string tmp;
	std::string::const_iterator f = abs_filename.begin();
	for(tmp = ""; f != abs_filename.end(); f++) {
		if(*f == '\\' || *f == '/')
			mkdir(tmp.c_str(), 0777);
		tmp += *f;
	}
	if(last_is_dir)
		mkdir(tmp.c_str(), 0777);
}

std::string GetFirstSearchPath() {
	if(tSearchPaths.size() > 0)
		return tSearchPaths.front();
	else if(basesearchpaths.size() > 0)
		return basesearchpaths.front();
	else
		return GetHomeDir();
}



	class CheckSearchpathForFile { public: 
		const std::string& filename;
		std::string* result;
		std::string* searchpath;
		CheckSearchpathForFile(const std::string& f, std::string* r, std::string* s) : filename(f), result(r), searchpath(s) {}
		
		bool operator() (const std::string& spath) {
			std::string tmp = spath + filename;
			if(GetExactFileName(tmp, *result)) {
				// we got here, if the file exists
				if(searchpath) *searchpath = spath;
				return false; // stop checking next searchpaths
			}
	
			// go to the next searchpath
			return true;
		}
	};

std::string GetFullFileName(const std::string& path, std::string* searchpath) {
	if(searchpath) *searchpath = "";
	if(path == "") return GetFirstSearchPath();

	std::string fname;
	// this also do lastly a check for an absolute filename
	ForEachSearchpath(CheckSearchpathForFile(path, &fname, searchpath));
	return fname;
}

std::string GetWriteFullFileName(const std::string& path, bool create_nes_dirs) {
	std::string tmp;
	std::string fname;

	// get the dir, where we should write into
	if(tSearchPaths.size() == 0 && basesearchpaths.size() == 0) {
		printf("ERROR: we want to write somewhere, but don't know where => we are writing to your temp-dir now...\n");
		tmp = GetTempDir() + "/" + path;
	} else {
		GetExactFileName(GetFirstSearchPath(), tmp);

		CreateRecDir(tmp);
		if(!CanWriteToDir(tmp)) {
			printf("ERROR: we cannot write to %s => we are writing to your temp-dir now...\n", tmp.c_str());
			tmp = GetTempDir();
		}

		tmp += "/";
		tmp += path;
	}

	GetExactFileName(tmp, fname);
	if(create_nes_dirs) CreateRecDir(fname, false);
	return tmp;
}

FILE *OpenGameFile(const std::string& path, const char *mode) {
	if(path.size() == 0)
		return NULL;

#ifdef WIN32
	// Convert the mode to wide char for wfopen
	wchar_t wide_mode[32];
	for (int i=0; i < (int)strlen(mode) && i < (int)(sizeof(wide_mode)/sizeof(wchar_t))-1;)  {
		wide_mode[i] = (wchar_t)mode[i];
		wide_mode[++i] = 0;
	}
#endif


	std::string fullfn = GetFullFileName(path);

	bool write_mode = strchr(mode, 'w') != 0;
	bool append_mode = strchr(mode, 'a') != 0;
	if(write_mode || append_mode) {
		std::string writefullname = GetWriteFullFileName(path, true);
		if(append_mode && fullfn != "") { // check, if we should copy the file
			if(IsFileAvailable(fullfn, true)) { // we found the file
				// GetWriteFullFileName ensures an exact filename,
				// so no case insensitive check is needed here
				if(fullfn != writefullname) {
					// it is not the file, we would write to, so copy it to the wanted destination
					if(!FileCopy(fullfn, writefullname)) {
						printf("ERROR: problems while copying, so I cannot open this file in append-mode somewhere else\n");
						return NULL;
					}
				}
			}
		}
		//printf("opening file for writing (mode %s): %s\n", mode, writefullname);
#ifdef WIN32 // uses UTF16
		return wfopen((wchar_t *)Utf8ToUtf16(writefullname).c_str(), wide_mode);
#else // other systems
		return fopen(writefullname.c_str(), mode);
#endif
	}

	if(fullfn.size() != 0) {
		//printf("open file for reading (mode %s): %s\n", mode, fullfn);
#ifdef WIN32 // uses UTF16
		return wfopen((wchar_t *)Utf8ToUtf16(fullfn).c_str(), wide_mode);
#else // uses Unicode
		return fopen(fullfn.c_str(), mode);
#endif
	}

	return NULL;
}



std::ifstream* OpenGameFileR(const std::string& path) {
	// TODO: unicode support! (?)

	if(path.size() == 0)
		return NULL;

	std::string fullfn = GetFullFileName(path);
	if(fullfn.size() != 0) {
		try {
			std::ifstream* f = new std::ifstream(fullfn.c_str(), std::ios::in | std::ios::binary);
			return f;
		} catch(...) {}
		return NULL;
	}

	return NULL;
}



void AddToFileList(searchpathlist* l, const std::string& f) {
	if(!FileListIncludesExact(l, f)) l->push_back(f);
}

void removeEndingSlashes(std::string& s) 
{
	while(s.size() > 0 && (*s.rbegin() == '\\' || *s.rbegin() == '/'))
		s.erase(s.size() - 1);	
}

/////////////////
// Returns true, if the list contains the path
bool FileListIncludesExact(const searchpathlist* l, const std::string& f) {
	std::string tmp1 = f;
	removeEndingSlashes(tmp1);
	ReplaceFileVariables(tmp1);
	replace(tmp1,"\\","/");
	
	// Go through the list, checking each item
	for(searchpathlist::const_iterator i = l->begin(); i != l->end(); i++) {
		std::string tmp2 = *i;
		removeEndingSlashes(tmp2);
		ReplaceFileVariables(tmp2);
		replace(tmp2,"\\","/");
		if(stringcasecmp(tmp1, tmp2) == 0)
			return true;
	}
	
	return false;
}

std::string GetHomeDir() {
#ifndef WIN32
	char* home = getenv("HOME");
	if(home == NULL || home[0] == '\0') {
		passwd* userinfo = getpwuid(getuid());
		if(userinfo)
			return userinfo->pw_dir;
		return ""; // both failed, very strange system...
	}
	return home;	
#else
	static wchar_t tmp[1024];
	if (!SHGetSpecialFolderPathW(NULL,tmp,CSIDL_PERSONAL,FALSE))  {
		// TODO: get dynamicaly another possible path
		// the following is only a workaround!
		return "C:\\OpenLieroX";
	}
	return Utf16ToUtf8((Utf16Char *)(&tmp[0]));
#endif
}


std::string GetSystemDataDir() {
#ifndef WIN32
	return SYSTEM_DATA_DIR;
#else
	// windows don't have such dir, don't it?
	// or should we return windows/system32 (which is not exactly intended here)?
	return "";
#endif
}


std::string	binary_dir; // given by argv[0], set by main()

std::string GetBinaryDir() {
	return binary_dir;
}

std::string GetTempDir() {
#ifndef WIN32
	return "/tmp"; // year, it's so simple :)
#else
	static wchar_t buf[256] = L"";
	if(buf[0] == L'\0') { // only do this once
		GetTempPathW(sizeof(buf)/sizeof(wchar_t),buf);
		fix_markend(buf);
	}
	return Utf16ToUtf8((Utf16Char *)(&buf[0]));
#endif
}

void ReplaceFileVariables(std::string& filename) {
	if(filename.compare(0,2,"~/")==0
	|| filename.compare(0,2,"~\\")==0
	|| filename == "~") {
		filename.erase(0,1);
		filename.insert(0,GetHomeDir());
	}
	replace(filename, "${HOME}", GetHomeDir());
	replace(filename, "${SYSTEM_DATA}", GetSystemDataDir());
	replace(filename, "${BIN}", GetBinaryDir());
}

// WARNING: not multithreading aware
// returns true, if successfull
bool FileCopy(const std::string& src, const std::string& dest) {
	static char tmp[2048];

	printf("FileCopy: %s -> %s\n", src.c_str(), dest.c_str());
#ifdef WIN32 // uses UTF16
	FILE* src_f = wfopen((wchar_t *)Utf8ToUtf16(src).c_str(), L"r");
#else // other systems
	FILE* src_f = fopen(src.c_str(), "r");
#endif
	if(!src_f) {
		printf("FileCopy: ERROR: cannot open source\n");
		return false;
	}

#ifdef WIN32 // uses UTF16
	FILE* dest_f = wfopen((wchar_t *)Utf8ToUtf16(dest).c_str(), L"w");
#else // other systems
	FILE* dest_f = fopen(dest.c_str(), "w");
#endif
	if(!dest_f) {
		fclose(src_f);
		printf("  ERROR: cannot open destination\n");
		return false;
	}

	printf("  ");
	bool success = true;
	unsigned short count = 0;
	size_t len = 0;
	while((len = fread(tmp, 1, sizeof(tmp), src_f)) > 0) {
		if(count == 0) printf("."); count++; count %= 20;
		if(len != fwrite(tmp, 1, len, dest_f)) {
			printf("  ERROR: problem while writing\n");
			success = false;
			break;
		}
		if(len != sizeof(tmp)) break;
	}
	if(success) {
		success = feof(src_f) != 0;
		if(!success) printf("  ERROR: problem while reading\n");
	}

	fclose(src_f);
	fclose(dest_f);
	if(success)	printf("  success :)\n");
	return success;
}

bool CanWriteToDir(const std::string& dir) {
	// TODO: we have to make this a lot better!
	std::string fname = dir + "/.some_stupid_temp_file";
#ifdef WIN32 // uses UTF16
	FILE* fp = wfopen((wchar_t *)Utf8ToUtf16(fname).c_str(), L"w");
#else // other systems
	FILE* fp = fopen(fname.c_str(), "w");
#endif
	if(fp) {
		fclose(fp);
#ifdef WIN32 // uses UTF16
		wremove((wchar_t *)Utf8ToUtf16(fname).c_str());
#else // other systems
		remove(fname.c_str());
#endif
		return true;
	}
	return false;
}


std::string GetAbsolutePath(const std::string& path) {
#ifdef WIN32
	static wchar_t buf[1024];
	static int len;
	len = GetFullPathNameW((wchar_t *)Utf8ToUtf16(path).c_str(),sizeof(buf)/sizeof(wchar_t),buf,NULL);
	fix_markend(buf);
	if (len)
		return Utf16ToUtf8((Utf16Char *)(&buf[0]));
	else  // Failed
		return path;
#else
	static char buf[1024];
	if(realpath(path.c_str(), buf) != NULL) {
		fix_markend(buf);
		return buf;
	} else
		return path;
#endif
}

bool PathListIncludes(const std::list<std::string>& pathlist, const std::string& path) {
	static std::string abs_path;
	abs_path = GetAbsolutePath(path);
	
	// Go through the list, checking each item
	for(std::list<std::string>::const_iterator i = pathlist.begin(); i != pathlist.end(); i++) {
		if(abs_path == GetAbsolutePath(*i)) {
			return true;
		}
	}
	
	return false;
}

