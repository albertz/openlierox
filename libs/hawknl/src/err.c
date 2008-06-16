/*
  HawkNL cross platform network library
  Copyright (C) 2000-2002 Phil Frisbie, Jr. (phil@hawksoft.com)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA  02111-1307, USA.

  Or go to http://www.gnu.org/copyleft/lgpl.html
*/

#include "nlinternal.h"

#ifdef WINDOWS_APP
/* Windows systems */
#define NL_WIN_THREADS

#ifdef _MSC_VER
#pragma warning (disable:4201)
#pragma warning (disable:4214)
#endif /* _MSC_VER */

#define WIN32_LEAN_AND_MEAN
#include <winsock.h>

#ifdef _MSC_VER
#pragma warning (default:4201)
#pragma warning (default:4214)
#endif /* _MSC_VER */

#endif

#ifdef NL_WIN_THREADS
/* native Windows */
static DWORD key = (DWORD)0xFFFFFFFF;
#else
/* POSIX systems */
#include <pthread.h>

#define KEY_NULL    ((pthread_key_t)-1)
static pthread_key_t key = KEY_NULL;
#endif

void nlSetError(NLenum err)
{

#ifdef NL_WIN_THREADS
    /* check to see if we need to initialize */
    if(key == (DWORD)0xFFFFFFFF)
    {
        key = TlsAlloc();
    }
    if(key != (DWORD)0xFFFFFFFF)
    {
        (void)TlsSetValue(key, (LPVOID)err);
    }
#else
    /* check to see if we need to initialize */
    if(key == KEY_NULL)
    {
        (void)pthread_key_create(&key, NULL);
    }
    if(key != KEY_NULL)
    {
        (void)pthread_setspecific(key, (void *)err);
    }
#endif
}

NL_EXP NLenum NL_APIENTRY nlGetError(void)
{
    NLenum  result;
#ifdef NL_WIN_THREADS
    /* check to see if we need to initialize */
    if(key == (DWORD)0xFFFFFFFF)
    {
        key = TlsAlloc();
        if(key == (DWORD)0xFFFFFFFF)
            return NL_TLS_ERROR;
        else
            return NL_NO_ERROR;
    }
    else
    {
        int     lasterror = WSAGetLastError();

        result = (NLenum)TlsGetValue(key);
        WSASetLastError(lasterror);
        return result;
    }
#else
    /* check to see if we need to initialize */
    if(key == KEY_NULL)
    {
        if(pthread_key_create(&key, NULL) != 0)
            return NL_TLS_ERROR;
        else
            return NL_NO_ERROR;
    }
    else
    {
        result = (NLenum)pthread_getspecific(key);
        return result;
    }
#endif
}


