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
	static char tmp[256];
	if(nextsearchpath != NULL) {
		strcpy(tmp, nextsearchpath->filename);
		strcat(tmp, "/");
		strcat(tmp, f);
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

char	_dir[256];
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
	strcpy(_dir, dir);
	
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
	static char tmp[256]; strcpy(tmp, _dir);
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
	static char tmp[256]; strcpy(tmp, _dir);
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
char	_dir2[256];
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
	strcpy(_dir2, dir);

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
	static char tmp[256]; strcpy(tmp, _dir2);
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
	static char tmp[256]; strcpy(tmp, _dir2);
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
	if(searchname == NULL) {
		if(filename != NULL)
			filename[0] = '\0';
		return false;
	}

	if (filename == NULL)
		return false;

	const char* seps[] = {"\\", "/", (char*)NULL};
	char nextname[256] = "";
	char nextexactname[256] = "";
	strcpy(filename, "");
	int pos = 0;
	int npos = 0;
	do
	{
		pos += npos;
		if(npos > 0) strcat(filename, "/");

		npos = GetNextName(&searchname[pos], seps, nextname);
		
		if(!CaseInsFindFile(filename, nextname, nextexactname))
		{
			strcat(filename, &searchname[pos]);
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



char	_dir[256];
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
	
	static char basepath[256];

	strcpy(basepath, _dir);
	strcat(basepath, "/");
	strcat(basepath, ext);

	handle = _findfirst(basepath, &fileinfo);
	strcpy(_dir, dir);
	
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
	static char tmp[256]; strcpy(tmp, _dir);
	// TODO: this better
	int ret = FindFirst(tmp,"*", filename);
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
	static char tmp[256]; strcpy(tmp, _dir);
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
char	_dir2[256];
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
	
	static char basepath[256];

	strcpy(basepath, _dir2);
	strcat(basepath, "/");
	strcat(basepath, "*.*");

	handle2 = _findfirst(basepath, &fileinfo2);
	strcpy(_dir2, dir);
	
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
	static char tmp[256]; strcpy(tmp, _dir2);
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
	static char tmp[256]; strcpy(tmp, _dir2);
	int ret = FindFirstDir(tmp, name);
	reset_nextsearchpath = true;	
	return ret;
}

#endif


filelist_t*	basesearchpaths = NULL;
void InitBaseSearchPaths() {
	assert(basesearchpaths == NULL);
	
#ifndef WIN32
	AddToFileList(&basesearchpaths, GetHomeDir());
	AddToFileList(&basesearchpaths, ".");
	AddToFileList(&basesearchpaths, SYSTEM_DATA_DIR"/OpenLieroX");
#else // Win32
	AddToFileList(&basesearchpaths, GetHomeDir());
	AddToFileList(&basesearchpaths, ".");
	// add EXE-file path
	char *slashpos = strrchr(argv0,'\\');
	if (slashpos)  {
		*slashpos = 0;
		AddToFileList(&basesearchpaths, argv0);
	}
#endif
}

void CreateRecDir(char* f) {
	static char tmp[256];	
	for(size_t i = 0; f[i] != '\0'; i++) {
		tmp[i] = f[i];
		if(tmp[i] == '\\' || tmp[i] == '/') {
			tmp[i] = '\0';
			mkdir(tmp, 0777);
			tmp[i] = f[i];
		}
	}
}

char* GetFullFileName(const char* path) {
	static char fname[256] = "";
	static char tmp[256] = "";
	
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
		strcpy(tmp, spath->filename);
		strcat(tmp, "/");
		strcat(tmp, path);
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
				fclose(f);
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

FILE *OpenGameFile(const char *path, const char *mode) {
	static char fname[1024] = "";
	static char tmp[1024] = "";
	
	if(path == NULL || path[0] == '\0')
		return NULL;
	
	if(strchr(mode, 'w')) {
		strcpy(tmp, GetHomeDir());
		strcat(tmp, "/");
		strcat(tmp, path);
		GetExactFileName(tmp, fname);
		CreateRecDir(fname);
		return fopen(fname, mode);
	}		

	char* fullfn = GetFullFileName(path);
	if(fullfn != NULL && fullfn[0] != '\0')  {
		FILE *result = fopen(fullfn, mode);
		if (!result) {
			return fopen(path,mode);
		}
		return result;
	}
	else
		return fopen(path,mode);
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
	strncpy((*fl)->filename, f, sizeof((*fl)->filename));
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
	strcpy(tmp1, f); 
	tmp1[1023] = '\0';
	removeEndingSlashes(tmp1);
	
	// Go through the list, checking each item
	for(const filelist_t* fl = l; fl != NULL; fl = fl->next) {
		if (!fl->filename)
			continue;
		strcpy(tmp2, fl->filename);
		tmp2[1023] = '\0';
		removeEndingSlashes(tmp2);
		if(strcasecmp(tmp1, tmp2) == 0)
			return true;
	}
	
	return false;
}

char* GetHomeDir() {
	static char tmp[1024];
#ifndef WIN32
	strcpy(tmp, getenv("HOME"));
	strcat(tmp, "/.OpenLieroX");
#else
	if (!SHGetSpecialFolderPath(NULL,tmp,CSIDL_PERSONAL,FALSE))  {
		// TODO: get dynamicaly another possible path
		// the following is only a workaround!
		strcpy(tmp, "C:\\OpenLieroX");
//		strcpy(tmp,"./");
		return tmp;
	}
	// Safety
	tmp[1023-strlen("\\OpenLieroX")] = '\0';

	strcat(tmp,"\\OpenLieroX");
#endif
	return tmp;
}

