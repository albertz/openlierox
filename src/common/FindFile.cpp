/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// File finding routines
// Created 30/9/01
// By Jason Boettcher


#include "defs.h"
#include "LieroX.h"

#ifdef WIN32
	#include <shlobj.h>
#endif

// TODO: merge windows and linux code in this file

bool reset_nextsearchpath = true;
filelist_t* nextsearchpath = NULL;

// TODO: this does not handle the FindFile and FindDir seperatly
char* getNextFullFileName(const char* f) {
	static char tmp[512];
	if(nextsearchpath != NULL) {
		fix_strncpy(tmp, nextsearchpath->filename);
		fix_strncat(tmp, "/");
		fix_strncat(tmp, f);
		return tmp;	
	} else
		return NULL;
}

bool CanReadFile(const char* f) {
	FILE* h = OpenGameFile(f, "r");
	if(!h) return false;
	fclose(h);
	return true;
}


#ifndef WIN32

/*
==========================

	  File Finding

==========================
*/

char	_dir[512];
DIR*	handle = NULL;
dirent* entry = NULL;

///////////////////
// Find the first file
int FindFirst(char *dir, char *ext, char *filename)
{
	if(reset_nextsearchpath) nextsearchpath = tLXOptions->tSearchPaths;
	GetExactFileName(getNextFullFileName(dir), _dir);
	if(_dir[0] == '\0')
		return false;
	
	if(strcmp(ext, "*") != 0)
		printf("FindFirst: WARNING: I can't handle anything else than * as ext \n");

	handle = opendir(_dir);
	fix_strncpy(_dir, dir);
	
	while(handle != 0 && (entry = readdir(handle)) != 0) // Keep going until we found the first file
	{
		//If file is not self-directory or parent-directory
		if(strcmp(entry->d_name, "."))
		 if(strcmp(entry->d_name, ".."))
		 {
			sprintf(filename,"%s/%s",_dir,entry->d_name);

			if(CanReadFile(filename))
				return true;
		}

	}

	if(handle) closedir(handle);
	handle = NULL;
	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	char tmp[512]; fix_strncpy(tmp, _dir);
	int ret = FindFirst(tmp, "*", filename);
	reset_nextsearchpath = true;	
	return ret;
}


///////////////////
// Find the next file
int FindNext(char *filename)
{
	while(entry = readdir(handle))	// Keep going until we found the next file
	{
		//If file is not self-directory or parent-directory
		if(strcmp(entry->d_name, "."))
		 if(strcmp(entry->d_name, ".."))
		 {
			sprintf(filename,"%s/%s",_dir,entry->d_name);
		
			if(CanReadFile(filename))
				return true;
		}
	}

	closedir(handle); handle = NULL;
	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	char tmp[512]; fix_strncpy(tmp, _dir);
	int ret = FindFirst(tmp, "*", filename);
	reset_nextsearchpath = true;	
	return ret;
}





/*
==========================

	Directory Finding

==========================
*/


// Here if we even need to search files & dirs at the same time
char	_dir2[512];
DIR*	handle2 = NULL;
dirent* entry2 = NULL;

///////////////////
// Find the first dir
int FindFirstDir(char *dir, char *name)
{
	if(reset_nextsearchpath) nextsearchpath = tLXOptions->tSearchPaths;
	GetExactFileName(getNextFullFileName(dir), _dir2);
	if(_dir2[0] == '\0')
		return false;
			
	handle2 = opendir(_dir2);
	fix_strncpy(_dir2, dir);

	while(handle2 != 0 && (entry2 = readdir(handle2)) != 0)	// Keep going until we found the next dir
	{
		//If file is not self-directory or parent-directory
		if(strcmp(entry2->d_name, "."))
		 if(strcmp(entry2->d_name, ".."))
		 {			
			sprintf(name,"%s/%s",_dir2,entry2->d_name);
			
			// well I know, not the best method, but it works
			// this should test, if it is a directory
			if(!CanReadFile(name))
				return true;
		}

	}

	if(handle2) closedir(handle2);
	handle2 = NULL;
	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	char tmp[512]; fix_strncpy(tmp, _dir2);
	int ret = FindFirstDir(tmp, name);
	reset_nextsearchpath = true;	
	return ret;
}


///////////////////
// Find the next dir
int FindNextDir(char *name)
{
	
	while(entry2 = readdir(handle2))	// Keep going until we found the next dir
	{
		//If file is not self-directory or parent-directory
		if(strcmp(entry2->d_name, "."))
		 if(strcmp(entry2->d_name, ".."))
		 {
			sprintf(name,"%s/%s",_dir2,entry2->d_name);
		
			// this should test, if it is a directory
			if(!CanReadFile(name))
				return true;
		}
	}

	closedir(handle2); handle2 = NULL;
	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	char tmp[512]; fix_strncpy(tmp, _dir2);
	int ret = FindFirstDir(tmp, name);
	reset_nextsearchpath = true;	
	return ret;
}


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
	while(direntry = readdir(dirhandle))
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
bool GetExactFileName(const char* searchname, char* filename)
{
	if (filename == NULL)
		return false;
	
	if(searchname == NULL) {
		filename[0] = '\0';
		return false;
	}

	std::string sname(searchname);
	ReplaceFileVariables(sname);

	const char* seps[] = {"\\", "/", (char*)NULL};
	static char nextname[512]; strcpy(nextname, "");
	static char nextexactname[512]; strcpy(nextexactname, "");
	strcpy(filename, "");
	int pos = 0;
	int npos = 0;
	do
	{
		pos += npos;
		if(npos > 0) strcat(filename, "/");

		npos = GetNextName(&sname.c_str()[pos], seps, nextname);
		
		if(!CaseInsFindFile(filename, nextname, nextexactname))
		{
			strcat(filename, &sname.c_str()[pos]);
			return false;
		}
		
		strcat(filename, nextexactname);
		
	} while(npos > 0);
	
	return true;
}


#else // WIN32


/*
==========================

	  File Finding

==========================
*/



char	_dir[512];
long	handle = 0;
struct _finddata_t fileinfo;


///////////////////
// Find the first file
int FindFirst(char *dir, char *ext, char *filename)
{
	if(reset_nextsearchpath) nextsearchpath = tLXOptions->tSearchPaths;
	GetExactFileName(getNextFullFileName(dir), _dir);
	if(_dir[0] == '\0')
		return false;
	
	static char basepath[512]; // don't need this later, so static is safe

	fix_strncpy(basepath, _dir);
	fix_strncat(basepath, "/");
	fix_strncat(basepath, ext);

	handle = _findfirst(basepath, &fileinfo);
	fix_strncpy(_dir, dir);
	
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
				sprintf(filename,"%s/%s",_dir,fileinfo.name);
				return true;
			}
		}
	}

	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	static char tmp[512]; fix_strncpy(tmp, _dir);
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
				sprintf(filename,"%s/%s",_dir,fileinfo.name);
				return true;
			}
		}
	};		// Keep going until we found the next file

	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	static char tmp[512]; fix_strncpy(tmp, _dir);
	// TODO: this better
	int ret = FindFirst(tmp,"*", filename);
	reset_nextsearchpath = true;	
	return ret;
}



/*
==========================

	Directory Finding

==========================
*/



// Here if we even need to search files & dirs at the same time
char	_dir2[512];
long	handle2 = 0;
struct _finddata_t fileinfo2;



///////////////////
// Find the first dir
int FindFirstDir(char *dir, char *name)
{
	if(reset_nextsearchpath) nextsearchpath = tLXOptions->tSearchPaths;
	GetExactFileName(getNextFullFileName(dir), _dir2);
	if(_dir2[0] == '\0')
		return false;
	
	static char basepath[512];

	fix_strncpy(basepath, _dir2);
	fix_strncat(basepath, "/");
	fix_strncat(basepath, "*.*");

	handle2 = _findfirst(basepath, &fileinfo2);
	fix_strncpy(_dir2, dir);
	
	while(handle2 >= 0 && !_findnext(handle2, &fileinfo2)) // Keep going until we found the first dir
	{
		//If file is not self-directory or parent-directory
		if(strcmp(fileinfo2.name, "."))
		 if(strcmp(fileinfo2.name, ".."))
		 {
			//If found file is a directory
			if(fileinfo2.attrib & _A_SUBDIR)
			{
				sprintf(name,"%s/%s",_dir2,fileinfo2.name);
				return true;
			}
		}
	}

	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	static char tmp[512]; fix_strncpy(tmp, _dir2);
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
				sprintf(name,"%s/%s",_dir2,fileinfo2.name);
				return true;
			}
		}

	}

	nextsearchpath = nextsearchpath->next;
	reset_nextsearchpath = false;
	static char tmp[512]; fix_strncpy(tmp, _dir2);
	int ret = FindFirstDir(tmp, name);
	reset_nextsearchpath = true;	
	return ret;
}

#endif


filelist_t*	basesearchpaths = NULL;
void InitBaseSearchPaths() {
	assert(basesearchpaths == NULL);
	
	// TODO: it would be nice to have also Mac OS X konversions
#ifndef WIN32
	AddToFileList(&basesearchpaths, "${HOME}/.OpenLieroX");
	AddToFileList(&basesearchpaths, ".");
	AddToFileList(&basesearchpaths, SYSTEM_DATA_DIR"/OpenLieroX"); // no use of ${SYSTEM_DATA}, because it is uncommon
#else // Win32
	AddToFileList(&basesearchpaths, "${HOME}/OpenLieroX");
	AddToFileList(&basesearchpaths, ".");
	AddToFileList(&basesearchpaths, "${BIN}");
#endif
}

void CreateRecDir(char* f, bool last_is_dir) {
	static char tmp[512];
	size_t i = 0;
	for(; f[i] != '\0'; i++) {
		tmp[i] = f[i];
		if(tmp[i] == '\\' || tmp[i] == '/') {
			tmp[i] = '\0';
			mkdir(tmp, 0777);
			tmp[i] = f[i];
		}
	}
	if(last_is_dir) {
		tmp[i] = '\0';
		mkdir(tmp, 0777);
	}
}

char* GetFullFileName(const char* path, char** searchpath) {
	static char fname[1024]; strcpy(fname, "");
	static char tmp[1024]; strcpy(tmp, "");
	
	if(path == NULL || path[0] == '\0')
		return NULL;

	filelist_t* spath = NULL;
	bool has_tried_basesearchpaths = false;
	if(tLXOptions != NULL) spath = tLXOptions->tSearchPaths;
	if(spath == NULL) {
		spath = basesearchpaths;
		has_tried_basesearchpaths = true;
	}
	while(spath) { // loop over searchpaths		
		fix_strncpy(tmp, spath->filename);
		fix_strncat(tmp, "/");
		fix_strncat(tmp, path);
		if(GetExactFileName(tmp, fname)) {
			// we got here, if the file exists
#ifndef WIN32
			// if it is a directory, return NULL
			DIR* h = opendir(fname);
			if(h) {
				closedir(h);
				return NULL;
			}
#else
			// fopen will return NULL on Windows, if fname is a dir
#endif
			FILE* f = fopen(fname, "r");
			if(f) {
				// we can read the file and it is not a directory
				fclose(f);
				if(searchpath)
					*searchpath = spath->filename;
				return fname;
			}
			
			return NULL;
		}
	
		spath = spath->next;
		if(spath == NULL && !has_tried_basesearchpaths) {
			has_tried_basesearchpaths = true;
			spath = basesearchpaths;
		}
	} // loop over searchpaths

	return NULL;
}

char* GetWriteFullFileName(const char* path, bool create_nes_dirs) {
	filelist_t* spath = NULL;
	if(tLXOptions != NULL) spath = tLXOptions->tSearchPaths;
	if(spath == NULL) spath = basesearchpaths;
	
	static char tmp[1024];
	static char fname[1024];
	
	// get the dir, where we should write into
	if(!spath) {
		printf("ERROR: we want to write somewhere, but don't know where => we are writing to your temp-dir now...\n");
		fix_strncpy(tmp, GetTempDir());
		fix_strncat(tmp, "/");
		fix_strncat(tmp, path);
		
	} else {
		GetExactFileName(spath->filename, tmp);
	
		CreateRecDir(tmp);
		if(!CanWriteToDir(tmp)) {
			printf("ERROR: we cannot write to %s => we are writing to your temp-dir now...\n", tmp);
			fix_strncpy(tmp, GetTempDir());
		}
		
		fix_strncat(tmp, "/");
		fix_strncat(tmp, path);
	}
	
	GetExactFileName(tmp, fname);	
	if(create_nes_dirs) CreateRecDir(fname, false);
	return tmp;
}

FILE *OpenGameFile(const char *path, const char *mode) {	
	if(path == NULL || path[0] == '\0')
		return NULL;
	
	char* fullfn = GetFullFileName(path);
	
	bool write_mode = strchr(mode, 'w') != NULL;
	bool append_mode = strchr(mode, 'a') != NULL;
	if(write_mode || append_mode) {
		char* writefullname = GetWriteFullFileName(path, true);
		if(append_mode && fullfn) { // check, if we should copy the file
			FILE* fp = fopen(fullfn, "r");
			if(fp) { // we can read the file
				fclose(fp);
				if(strcmp(fullfn, writefullname)) {
					// it is not the file, we would write to, so copy it to the wanted destination
					FileCopy(fullfn, writefullname);
				}
			}
		}
		//printf("opening file for writing (mode %s): %s\n", mode, writefullname);
		return fopen(writefullname, mode);
	}		

	if(fullfn != NULL && fullfn[0] != '\0') {
		//printf("open file for reading (mode %s): %s\n", mode, fullfn);
		return fopen(fullfn, mode);
	}
	
	return NULL;
}


void AddToFileList(filelist_t** l, const char* f) {
	if (!l || !f)
		return;
	filelist_t** fl;
	for(fl = l; *fl != NULL; fl = &(*fl)->next) {}
	*fl = new filelist_t;
	if (!(*fl))
		return;
	(*fl)->next = NULL;
	fix_strncpy((*fl)->filename, f);
}

void removeEndingSlashes(char* s) {
	for(
		int i = strlen(s) - 1;
		i > 0 && (s[i] == '\\' || s[i] == '/');
		i--
	)
		s[i] = '\0';
}

/////////////////
// Returns true, if the list contains the path
bool FileListIncludes(const filelist_t* l, const char* f) {
	// Check
	if (!f || !l)
		return false;

	static char tmp1[1024] = "";
	static char tmp2[1024] = "";
	fix_strncpy(tmp1, f); 
	removeEndingSlashes(tmp1);
	
	// Go through the list, checking each item
	for(const filelist_t* fl = l; fl != NULL; fl = fl->next) {
		if (!fl->filename)
			continue;
		fix_strncpy(tmp2, fl->filename);
		removeEndingSlashes(tmp2);
		if(strcasecmp(tmp1, tmp2) == 0)
			return true;
	}
	
	return false;
}

char* GetHomeDir() {
	static char tmp[1024];
#ifndef WIN32
	fix_strncpy(tmp, getenv("HOME"));
#else
	if (!SHGetSpecialFolderPath(NULL,tmp,CSIDL_PERSONAL,FALSE))  {
		// TODO: get dynamicaly another possible path
		// the following is only a workaround!
		fix_strncpy(tmp, "C:\\OpenLieroX");
	}
#endif
	return tmp;
}


char* GetSystemDataDir() {
#ifndef WIN32	
	return SYSTEM_DATA_DIR;
#else
	// windows don't have such dir, don't it?
	// or should we return windows/system32 (which is not exactly intended here)?
	return "";
#endif
}

char* GetBinaryDir() {
	return binary_dir;
}

char* GetTempDir() {
#ifndef WIN32
	return "/tmp"; // year, it's so simple :)
#else
	// TODO !!
	static char buf[256];
	GetTempPath(sizeof(buf),buf);
	fix_markend(buf);
	return buf;
#endif
}

void ReplaceFileVariables(std::string& filename) {
	replace(filename, "${HOME}", GetHomeDir());
	replace(filename, "${SYSTEM_DATA}", GetSystemDataDir());
	replace(filename, "${BIN}", GetBinaryDir());	
}

bool FileCopy(const std::string src, const std::string dest) {
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

bool CanWriteToDir(const std::string dir) {
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
