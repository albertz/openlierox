/////////////////////////////////////////
//
//   Auxiliary Software class library
//
/////////////////////////////////////////


// File finding routines
// Created 30/9/01
// By Jason Boettcher


#include "defs.h"

#ifdef WIN32
#	include <io.h>
#else
	// TODO: i already coded it, i will place it here tommorrow or ...
#endif


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
				sprintf(filename,"%s\\%s",_dir,fileinfo.name);

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

	do
	{
		//If file is not self-directory or parent-directory
		if(strcmp(fileinfo.name, "."))
		 if(strcmp(fileinfo.name, ".."))
		 {
			//If found file is not a directory
			if(!(fileinfo.attrib & _A_SUBDIR))
			{
				sprintf(filename,"%s\\%s",_dir,fileinfo.name);

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

	do
	{
		//If file is not self-directory or parent-directory
		if(strcmp(fileinfo2.name, "."))
		 if(strcmp(fileinfo2.name, ".."))
		 {
			//If found file is a directory
			if(fileinfo2.attrib & _A_SUBDIR)
			{
				sprintf(name,"%s\\%s",_dir,fileinfo2.name);

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
				sprintf(name,"%s\\%s",_dir,fileinfo2.name);

				return true;
			}
		}
	} while(!_findnext(handle2, &fileinfo2));		// Keep going until we found the next dir


	return false;
}
