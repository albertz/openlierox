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


#include "defs.h"
#include "LieroX.h"

#ifdef WIN32
	#include <shlobj.h>
#else
	#include <pwd.h>
	#include <sys/types.h>
#endif

// TODO: merge windows and linux code in this file
/*
bool reset_nextsearchpath = true;
filelist_t* nextsearchpath = NULL;

// TODO: this does not handle the FindFile and FindDir seperatly
std::string getNextFullFileName(const std::string& f) {
	if(nextsearchpath != NULL) {
		return nextsearchpath->filename + "/" + f;
	} else
		return f;
}
*/
bool CanReadFile(const std::string& f, bool absolute) {
	if(absolute) {
#ifndef WIN32
		struct stat s;
		if(stat(f.c_str(), &s) != 0 || !S_ISREG(s.st_mode)) {
			// it's not stat-able or not a reg file
			return false;
		}
#else
		// fopen will return NULL on Windows, if fname is a dir
#endif
		FILE* fp = fopen(f.c_str(), "r");
		if(fp) {
			// we can read the file and it is not a directory
			fclose(fp);
			return true;
		}

		return false;
	}
	
	FILE* h = OpenGameFile(f, "r");
	if(!h) return false;
	fclose(h);
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
		for (register int i=0; i<len; i+=strnlen(&drives[i],4)+1)  {
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

/*
==========================

	  File Finding

==========================
*/
/*
std::string	_dir;
DIR*	handle = NULL;
dirent* entry = NULL;

///////////////////
// Find the first file
int FindFirst(const std::string& dir, char *ext, char *filename)
{
	if(reset_nextsearchpath) nextsearchpath = tLXOptions->tSearchPaths;
	GetExactFileName(getNextFullFileName(dir), _dir);
	if(_dir.size() == 0)
		return false;

	if(strcmp(ext, "*") != 0)
		printf("FindFirst: WARNING: I can't (and will not!) handle anything else than * as extension\n");

	handle = opendir(_dir.c_str());
	_dir = dir;

	while(handle != 0 && (entry = readdir(handle)) != 0) // Keep going until we found the first file
	{
		//If file is not self-directory or parent-directory
		if(strcmp(entry->d_name, "."))
		 if(strcmp(entry->d_name, ".."))
		 {
			sprintf(filename,"%s/%s",_dir.c_str(),entry->d_name);

			if(CanReadFile(filename))
				return true;
		}

	}

	if(handle) closedir(handle);
	handle = NULL;
	if(!nextsearchpath)	return false;
		
	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	char tmp[512]; fix_strncpy(tmp, _dir.c_str()); // TODO ...
	int ret = FindFirst(tmp, "*", filename);
	reset_nextsearchpath = true;
	return ret;
}


///////////////////
// Find the next file
int FindNext(char *filename)
{
	while((entry = readdir(handle)))	// Keep going until we found the next file
	{
		//If file is not self-directory or parent-directory
		if(strcmp(entry->d_name, "."))
		 if(strcmp(entry->d_name, ".."))
		 {
			sprintf(filename,"%s/%s",_dir.c_str(),entry->d_name);

			if(CanReadFile(filename))
				return true;
		}
	}

	closedir(handle); handle = NULL;
	if(!nextsearchpath) return false;
	
	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	char tmp[512]; fix_strncpy(tmp, _dir.c_str());
	int ret = FindFirst(tmp, "*", filename);
	reset_nextsearchpath = true;
	return ret;
}


*/


/*
==========================

	Directory Finding

==========================
*/
/*

// Here if we even need to search files & dirs at the same time
std::string	_dir2;
DIR*	handle2 = NULL;
dirent* entry2 = NULL;

///////////////////
// Find the first dir
int FindFirstDir(const std::string& dir, char *name)
{
	if(reset_nextsearchpath) nextsearchpath = tLXOptions->tSearchPaths;
	GetExactFileName(getNextFullFileName(dir), _dir2);
	if(_dir2.size() == 0)
		return false;

	// TODO: in some cases, handle2 will not get free
	handle2 = opendir(_dir2.c_str());
	_dir2 = dir;

	while(handle2 != 0 && (entry2 = readdir(handle2)) != 0)	// Keep going until we found the next dir
	{
		//If file is not self-directory or parent-directory
		if(strcmp(entry2->d_name, "."))
		 if(strcmp(entry2->d_name, ".."))
		 {
			sprintf(name,"%s/%s",_dir2.c_str(),entry2->d_name);

			// well I know, not the best method, but it works
			// this should test, if it is a directory
			if(!CanReadFile(name))
				return true;
		}

	}

	if(handle2) closedir(handle2);
	handle2 = NULL;
	if(!nextsearchpath) return false;
	
	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	char tmp[512]; fix_strncpy(tmp, _dir2.c_str()); // TODO ...
	int ret = FindFirstDir(tmp, name);
	reset_nextsearchpath = true;
	return ret;
}


///////////////////
// Find the next dir
int FindNextDir(char *name)
{

	while((entry2 = readdir(handle2)))	// Keep going until we found the next dir
	{
		//If file is not self-directory or parent-directory
		if(strcmp(entry2->d_name, "."))
		 if(strcmp(entry2->d_name, ".."))
		 {
			sprintf(name,"%s/%s",_dir2.c_str(),entry2->d_name);

			// this should test, if it is a directory
			if(!CanReadFile(name))
				return true;
		}
	}

	closedir(handle2); handle2 = NULL;
	if(!nextsearchpath) return false;
	
	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	char tmp[512]; fix_strncpy(tmp, _dir2.c_str()); // TODO ...
	int ret = FindFirstDir(tmp, name);
	reset_nextsearchpath = true;
	return ret;
}
*/

// used by unix-GetExactFileName
int GetNextName(const char* fullname, const char** seperators, char* nextname)
{
	int pos;
	int i;

	for(pos = 0; fullname[pos] != '\0'; pos++)
	{
		for(i = 0; seperators[i] != NULL; i++)
			if(strncasecmp(&fullname[pos], seperators[i], strlen(seperators[i])) == 0)
			{
				nextname[pos] = '\0';
				return pos + strlen(seperators[i]);
			}

		nextname[pos] = fullname[pos];
	}

	nextname[pos] = '\0';
	return 0;
}


// used by unix-GetExactFileName
int CaseInsFindFile(const char* dir, const char* searchname, char* filename)
{
	if(strcmp(searchname, "") == 0)
	{
		strcpy(filename, "");
		return true;
	}

	DIR* dirhandle;
	dirhandle = opendir((strcmp(dir, "") == 0) ? "." : dir);
	if(dirhandle == 0) return false;

	dirent* direntry;
	while((direntry = readdir(dirhandle)))
	{
		if(strcasecmp(direntry->d_name, searchname) == 0)
		{
			strcpy(filename, direntry->d_name);
			closedir(dirhandle);
			return true;
		}
	}

	closedir(dirhandle);
	return false;
}


// does case insensitive search for file
bool GetExactFileName(const std::string& abs_searchname, std::string& filename)
{
	if(abs_searchname.size() == 0) {
		filename = "";
		return false;
	}

	std::string sname = abs_searchname;
	ReplaceFileVariables(sname);

	// TODO: ouhhhhh, this has to be redone... !
	const char* seps[] = {"\\", "/", (char*)NULL};
	static char nextname[512]; strcpy(nextname, "");
	static char nextexactname[512]; strcpy(nextexactname, "");
	filename = "";
	int pos = 0;
	int npos = 0;
	do {
		pos += npos;
		if(npos > 0) filename += "/";

		npos = GetNextName(&sname.c_str()[pos], seps, nextname);

		if(!CaseInsFindFile(filename.c_str(), nextname, nextexactname))
		{
			filename += &sname.c_str()[pos];
			return false;
		}

		filename += nextexactname;

	} while(npos > 0);

	return true;
}


#else // WIN32


/*
==========================

	  File Finding

==========================
*/


/*
std::string	_dir;
long	handle = 0;
struct _finddata_t fileinfo;


///////////////////
// Find the first file
int FindFirst(const std::string& dir, char *ext, char *filename)
{
	if(reset_nextsearchpath) nextsearchpath = tLXOptions->tSearchPaths;
	GetExactFileName(getNextFullFileName(dir),_dir);
	if(_dir == "")
		return false;

	static char basepath[512]; // don't need this later, so static is safe

	fix_strncpy(basepath, _dir.c_str());
	fix_strncat(basepath, "/");
	fix_strncat(basepath, ext);

	handle = _findfirst(basepath, &fileinfo);
	_dir = dir;

	// Keep going until we found the first file
	while(handle >= 0 && !_findnext(handle, &fileinfo))
	{
		//If file is not self-directory or parent-directory
		if(strcmp(fileinfo.name, "."))
		 if(strcmp(fileinfo.name, ".."))
		 {
			//If found file is not a directory
			if(!(fileinfo.attrib & _A_SUBDIR))
			{
				sprintf(filename,"%s/%s",_dir.c_str(),fileinfo.name);
				return true;
			}
		}
	}

	if(!nextsearchpath) return false;	
	
	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	static char tmp[512]; fix_strncpy(tmp, _dir.c_str());
	// TODO: this better
	int ret = FindFirst(tmp,ext, filename);
	reset_nextsearchpath = true;
	return ret;
}


///////////////////
// Find the next file
int FindNext(char *filename)
{
	while(!_findnext(handle, &fileinfo)) {
		//If file is not self-directory or parent-directory
		if(strcmp(fileinfo.name, "."))
		 if(strcmp(fileinfo.name, ".."))
		 {
			//If found file is not a directory
			if(!(fileinfo.attrib & _A_SUBDIR))
			{
				sprintf(filename,"%s/%s",_dir.c_str(),fileinfo.name);
				return true;
			}
		}
	};		// Keep going until we found the next file

	if(!nextsearchpath) return false;
	
	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	static char tmp[512]; fix_strncpy(tmp, _dir.c_str());
	// TODO: this better
	int ret = FindFirst(tmp,"*", filename);
	reset_nextsearchpath = true;
	return ret;
}

*/

/*
==========================

	Directory Finding

==========================
*/

/*

// Here if we even need to search files & dirs at the same time
std::string	_dir2;
long	handle2 = 0;
struct _finddata_t fileinfo2;



///////////////////
// Find the first dir
int FindFirstDir(const std::string& dir, char *name)
{
	if(reset_nextsearchpath) nextsearchpath = tLXOptions->tSearchPaths;

	GetExactFileName(getNextFullFileName(dir), _dir2);

	if(_dir2 == "")
		return false;

	static char basepath[512];

	fix_strncpy(basepath, _dir2.c_str());
	fix_strncat(basepath, "/");
	fix_strncat(basepath, "*.*");

	handle2 = _findfirst(basepath, &fileinfo2);
	_dir2 = dir;

	while(handle2 >= 0 && !_findnext(handle2, &fileinfo2)) // Keep going until we found the first dir
	{
		//If file is not self-directory or parent-directory
		if(strcmp(fileinfo2.name, "."))
		 if(strcmp(fileinfo2.name, ".."))
		 {
			//If found file is a directory
			if(fileinfo2.attrib & _A_SUBDIR)
			{
				sprintf(name,"%s/%s",_dir2.c_str(),fileinfo2.name);
				return true;
			}
		}
	}

	if(!nextsearchpath) return false;
	
	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	static char tmp[512]; fix_strncpy(tmp, _dir2.c_str());
	int ret = FindFirstDir(tmp, name);
	reset_nextsearchpath = true;
	return ret;
}



///////////////////
// Find the next dir
int FindNextDir(char *name)
{
	// Keep going until we found the next dir
	while(!_findnext(handle2, &fileinfo2)) {
		//If file is not self-directory or parent-directory
		if(strcmp(fileinfo2.name, "."))
		 if(strcmp(fileinfo2.name, ".."))
		 {
			//If found file is a directory
			if(fileinfo2.attrib & _A_SUBDIR)
			{
				sprintf(name,"%s/%s",_dir2.c_str(),fileinfo2.name);
				return true;
			}
		}

	}

	if(!nextsearchpath) return false;
	
	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	static char tmp[512]; fix_strncpy(tmp, _dir2.c_str());
	int ret = FindFirstDir(tmp, name);
	reset_nextsearchpath = true;
	return ret;
}
*/

#endif // WIN32


searchpathlist	basesearchpaths;
void InitBaseSearchPaths() {
	// TODO: it would be nice to have also Mac OS X conversions
#ifndef WIN32
	AddToFileList(&basesearchpaths, "${HOME}/.OpenLieroX");
	AddToFileList(&basesearchpaths, ".");
	AddToFileList(&basesearchpaths, SYSTEM_DATA_DIR"/OpenLieroX"); // no use of ${SYSTEM_DATA}, because it is uncommon and could cause confusion to the user
#else // Win32
	AddToFileList(&basesearchpaths, "${HOME}/OpenLieroX");
	AddToFileList(&basesearchpaths, ".");
	AddToFileList(&basesearchpaths, "${BIN}");
#endif
}

void CreateRecDir(std::string abs_filename, bool last_is_dir) {
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

std::string GetFullFileName(const std::string& path, std::string* searchpath) {
	static std::string fname;
	static std::string tmp;

	if(searchpath) *searchpath = "";

	if(path.size() == 0)
		return "";

	searchpathlist* spath = NULL;
	bool has_tried_basesearchpaths = false;
	if(tLXOptions != NULL) spath = tLXOptions->tSearchPaths;
	if(spath == NULL) {
		spath = basesearchpaths;
		has_tried_basesearchpaths = true;
	}
	while(spath) { // loop over searchpaths
		tmp = spath->filename + "/" + path;
		if(GetExactFileName(tmp, fname)) {
			// we got here, if the file exists
			if(CanReadFile(fname, true)) {
				if(searchpath) *searchpath = spath->filename;
				return fname;
			}
		}

		spath = spath->next;
		if(spath == NULL && !has_tried_basesearchpaths) {
			has_tried_basesearchpaths = true;
			spath = basesearchpaths;
		}
	} // loop over searchpaths

	// none file in searchpaths found, check now, if it is perhaps an absolute filename
	if(GetExactFileName(path, fname)) {
		// we got here, if the file exists
		if(CanReadFile(fname, true)) {
			// searchpath is already set to ""
			return fname;
		}
	}
	
	return "";
}

std::string GetWriteFullFileName(const std::string& path, bool create_nes_dirs) {
	filelist_t* spath = NULL;
	if(tLXOptions != NULL) spath = tLXOptions->tSearchPaths;
	if(spath == NULL) spath = basesearchpaths;

	static std::string tmp;
	static std::string fname;

	// get the dir, where we should write into
	if(!spath) {
		printf("ERROR: we want to write somewhere, but don't know where => we are writing to your temp-dir now...\n");
		tmp = GetTempDir() + "/" + path;
	} else {
		GetExactFileName(spath->filename, tmp);

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

	std::string fullfn = GetFullFileName(path);

	bool write_mode = strchr(mode, 'w') != 0;
	bool append_mode = strchr(mode, 'a') != 0;
	if(write_mode || append_mode) {
		std::string writefullname = GetWriteFullFileName(path, true);
		if(append_mode && fullfn.size()>0) { // check, if we should copy the file
			if(CanReadFile(fullfn)) { // we can read the file
				fclose(fp);
				// GetWriteFullFileName ensures an exact filename,
				// so no case insensitive check is needed here
				if(fullfn != writefullname) {
					// it is not the file, we would write to, so copy it to the wanted destination
					FileCopy(fullfn, writefullname);
				}
			}
		}
		//printf("opening file for writing (mode %s): %s\n", mode, writefullname);
		return fopen(writefullname.c_str(), mode);
	}

	if(fullfn.size() != 0) {
		//printf("open file for reading (mode %s): %s\n", mode, fullfn);
		return fopen(fullfn.c_str(), mode);
	}

	return NULL;
}


void AddToFileList(searchpathlist* l, const std::string& f) {
	if(!FileListIncludes(l, f)) l->push_back(f);
}

void removeEndingSlashes(std::string& s) {	
	for(
		int i = s.size() - 1;
		i > 0 && (s[i] == '\\' || s[i] == '/');
		i--
	)
		s.erase(i);
}

/////////////////
// Returns true, if the list contains the path
bool FileListIncludes(const searchpathlist* l, const std::string& f) {
	static std::string tmp1;
	static std::string tmp2;
	tmp1 = f;
	removeEndingSlashes(tmp1);
	ReplaceFileVariables(tmp1);
	
	// Go through the list, checking each item
	for(std::list::const_iterator i = l->begin(); i != l->end(); i++) {
		tmp2 = *i;
		removeEndingSlashes(tmp2);
		ReplaceFileVariables(tmp2);
		if(stringcasecmp(tmp1, tmp2) == 0)
			return true;		
	}
	
	return false;
}

std::string GetHomeDir() {
#ifndef WIN32
	passwd* userinfo = getpwuid(getuid());
	if(userinfo) {
		return userinfo->pw_dir;
	} else
		return getenv("HOME"); // last chance (doesnt work in some exotic cases)
#else
	static char tmp[1024];
	if (!SHGetSpecialFolderPath(NULL,tmp,CSIDL_PERSONAL,FALSE))  {
		// TODO: get dynamicaly another possible path
		// the following is only a workaround!
		return "C:\\OpenLieroX";
	}
	return tmp;
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

std::string GetBinaryDir() {
	return binary_dir;
}

std::string GetTempDir() {
#ifndef WIN32
	return "/tmp"; // year, it's so simple :)
#else
	static char buf[256] = "";
	if(buf[0] == '\0') { // only do this once
		GetTempPath(sizeof(buf),buf);
		fix_markend(buf);
	}
	return buf;
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

bool FileCopy(const std::string& src, const std::string& dest) {
	static char tmp[2048];

	printf("FileCopy: %s -> %s\n", src.c_str(), dest.c_str());
	FILE* src_f = fopen(src.c_str(), "r");
	if(!src_f) {
		printf("FileCopy: ERROR: cannot open source\n");
		return false;
	}
	FILE* dest_f = fopen(dest.c_str(), "w");
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
	FILE* fp = fopen(fname.c_str(), "w");
	if(fp) {
		fclose(fp);
		remove(fname.c_str());
		return true;
	}
	return false;
}
