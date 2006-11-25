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

#ifndef WIN32

/*
==========================

	  File Finding

==========================
*/

char	_dir[256];
DIR*	handle = NULL;
dirent* entry = NULL;
struct stat	entrystat;

///////////////////
// Find the first file
int FindFirst(char *dir, char *ext, char *filename)
{
	GetExactFileName(dir, _dir);
	
	if(strcmp(ext, "*") != 0)
		printf("FindFirst: ERROR: I can't handle anything else than * \n");

	if((handle = opendir(_dir)) == 0)
		return false;

	while(entry = readdir(handle)) // Keep going until we found the first file
	{
		//If file is not self-directory or parent-directory
		if(strcmp(entry->d_name, "."))
		 if(strcmp(entry->d_name, ".."))
		 {
			sprintf(filename,"%s/%s",_dir,entry->d_name);
					
			//If found file is a file
			if(stat(filename, &entrystat) == 0 && entrystat.st_mode & S_IFREG)
				return true;
		}

	}

	closedir(handle);
	return false;
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
		
			//If found file is a file
			if(stat(filename, &entrystat) == 0 && entrystat.st_mode & S_IFREG)
				return true;
		}
	}

	closedir(handle);
	return false;
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
struct stat	entry2stat;

///////////////////
// Find the first dir
int FindFirstDir(char *dir, char *name)
{
	GetExactFileName(dir, _dir2);
	
	if((handle2 = opendir(_dir2)) == 0)
		return false;

	while(entry2 = readdir(handle2))	// Keep going until we found the next dir
	{
		//If file is not self-directory or parent-directory
		if(strcmp(entry2->d_name, "."))
		 if(strcmp(entry2->d_name, ".."))
		 {			
			sprintf(name,"%s/%s",_dir2,entry2->d_name);
			
			//If found file is a directory
			if(stat(name, &entry2stat) == 0 && entry2stat.st_mode & S_IFDIR)
				return true;
		}

	}

	closedir(handle2);
	return false;
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
		
			//If found file is a directory
			if(stat(name, &entry2stat) == 0 && entry2stat.st_mode & S_IFDIR)
				return true;
		}
	}

	closedir(handle2);
	return false;
}



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
	char basepath[256];

	strcpy(_dir,dir);

	strcpy(basepath, dir);
	strcat(basepath, "/");
	strcat(basepath, ext);

	if((handle = _findfirst(basepath, &fileinfo)) < 0)
		return false;

	do
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
	} while(!_findnext(handle, &fileinfo));		// Keep going until we found the first file

	return false;
}


///////////////////
// Find the next file
int FindNext(char *filename)
{
	if(_findnext(handle, &fileinfo))
		return false;

	do	{
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
	} while(!_findnext(handle, &fileinfo));		// Keep going until we found the next file

	return false;
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
	char basepath[256];

	strcpy(_dir,dir);
	strcpy(basepath, dir);
	strcat(basepath, "/");
	strcat(basepath, "*.*");

	if((handle2 = _findfirst(basepath, &fileinfo2)) < 0)
		return false;

	do {
		//If file is not self-directory or parent-directory
		if(strcmp(fileinfo2.name, "."))
		 if(strcmp(fileinfo2.name, ".."))
		 {
			//If found file is a directory
			if(fileinfo2.attrib & _A_SUBDIR)
			{
				sprintf(name,"%s/%s",_dir,fileinfo2.name);
				return true;
			}
		}
	} while(!_findnext(handle2, &fileinfo2));		// Keep going until we found the first dir

	return false;
}



///////////////////
// Find the next dir
int FindNextDir(char *name)
{
	if(_findnext(handle2, &fileinfo2))
		return false;

	do
	{
		//If file is not self-directory or parent-directory
		if(strcmp(fileinfo2.name, "."))
		 if(strcmp(fileinfo2.name, ".."))
		 {
			//If found file is a directory
			if(fileinfo2.attrib & _A_SUBDIR)
			{
				sprintf(name,"%s/%s",_dir,fileinfo2.name);
				return true;
			}
		}

	} while(!_findnext(handle2, &fileinfo2));		// Keep going until we found the next dir

	return false;
}

#endif


filelist_t*	basesearchpaths = NULL;
void InitBaseSearchPaths() {
	assert(basesearchpaths == NULL);
	
#ifndef WIN32
	AddToFileList(&basesearchpaths, ".");
	AddToFileList(&basesearchpaths, "~");
	AddToFileList(&basesearchpaths, "/usr/share/OpenLieroX");
#else // Win32
	AddToFileList(&basesearchpaths, ".");
	AddToFileList(&basesearchpaths, "~");
	// add EXE-file path
	char *slashpos = strrchr(argv0,'\\');
	*slashpos = 0;
	AddToFileList(&basesearchpaths, argv0);
#endif
}

FILE *OpenGameFile(const char *path, const char *mode) {
	static char fname[256] = "";
	static char tmp[256] = "";
	
	filelist_t* spath = tLXOptions->tSearchPaths;
	if(spath == NULL) spath = basesearchpaths;
	assert(spath != NULL);
	for(; spath != NULL; spath = spath->next) {
		strcpy(tmp, spath->filename);
		strcat(tmp, "/");
		strcat(tmp, path);
		if(GetExactFileName(tmp, fname)) {
			// we got here, if the file exists
			printf("fopen -> %s\n", fname);
			return fopen(fname, mode);			
		}
	}
	
	return NULL;	
}


void AddToFileList(filelist_t** l, const char* f) {
	filelist_t** fl;
	for(fl = l; *fl != NULL; fl = &(*fl)->next) {}
	*fl = new filelist_t;
	(*fl)->next = NULL;
	strcpy((*fl)->filename, f);
}

void RemoveEndingSlashes(char* s) {
	for(
		int i = strlen(s) - 1;
		i > 0 && (s[i] == '\\' || s[i] == '/');
		i--
	)
		s[i] = '\0';
}

bool FileListIncludes(const filelist_t* l, const char* f) {
	static char tmp1[64] = ""; // TODO: enough?
	static char tmp2[64] = ""; // TODO: enough?
	strcpy(tmp1, f); RemoveEndingSlashes(tmp1);
	
	for(const filelist_t* fl = l; fl != NULL; fl = fl->next) {
		strcpy(tmp2, fl->filename); RemoveEndingSlashes(tmp2);
		if(strcasecmp(tmp1, tmp2) == 0)
			return true;
	}
	
	return false;
}

