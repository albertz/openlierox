/*
  HawkNL time module
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
#ifdef _MSC_VER
#pragma warning (disable:4201)
#pragma warning (disable:4214)
#endif /* _MSC_VER */

#include <windows.h>
#include <winbase.h>
#include <limits.h>

#ifdef _MSC_VER
#pragma warning (default:4201)
#pragma warning (default:4214)
#endif /* _MSC_VER */

struct mytimeb {
    time_t time;
    unsigned short millitm;
};

static void myftime(struct mytimeb *tb)
{
    static int      needinit = 1;
    static time_t   currentseconds;
    static DWORD    currentmseconds;
    static DWORD    lastmseconds;

    if(needinit == 1)
    {
        time_t t;

        currentseconds = time(&t);
        lastmseconds = GetTickCount();
        currentmseconds = lastmseconds % 1000;
        needinit = 0;
    }
    else
    {
        DWORD mseconds = GetTickCount();

        /* check for roll over */
        if(mseconds < lastmseconds)
        {
            currentmseconds += (UINT_MAX - lastmseconds) + mseconds + 1;
        }
        else
        {
            currentmseconds += mseconds - lastmseconds;
        }
        lastmseconds = mseconds;
        while(currentmseconds > 1000)
        {
            currentseconds++;
            currentmseconds -= 1000;
        }

    }
    tb->time = currentseconds;
    tb->millitm = (unsigned short)currentmseconds;
}
#else /* !WINDOWS_APP */

#include <sys/time.h>

#endif /* !WINDOWS_APP */

NL_EXP NLboolean NL_APIENTRY nlTime(NLtime *t)
{
#ifdef WINDOWS_APP
    static NLboolean        needinit = NL_TRUE;
    static NLboolean        haspcounter = NL_FALSE;
    static LARGE_INTEGER    freq;
    static LARGE_INTEGER    lastcount;
    static NLtime           currenttime;

    if(t == NULL)
    {
		nlSetError(NL_NULL_POINTER);
		return NL_FALSE;
    }
    if(needinit == NL_TRUE)
    {
        if(QueryPerformanceFrequency(&freq) != 0)
        {
            if(QueryPerformanceCounter(&lastcount) != 0)
            {
                /* get the current time */
                struct mytimeb tb;

                myftime(&tb);
                currenttime.seconds = (NLlong)(tb.time);
                currenttime.useconds = (NLlong)(tb.millitm * 1000);
                haspcounter = NL_TRUE;
            }
        }
        needinit = NL_FALSE;
    }
    if(haspcounter == NL_TRUE)
    {
        LARGE_INTEGER   currentcount;
        LARGE_INTEGER   diffcount;

        (void)QueryPerformanceCounter(&currentcount);
        diffcount.QuadPart = currentcount.QuadPart - lastcount.QuadPart;
        lastcount.QuadPart = currentcount.QuadPart;
        while(diffcount.QuadPart >= freq.QuadPart)
        {
            diffcount.QuadPart -= freq.QuadPart;
            currenttime.seconds++;
        }
        currenttime.useconds += (NLlong)(diffcount.QuadPart * 1000000 / freq.QuadPart);
        if(currenttime.useconds >= 1000000)
        {
            currenttime.useconds -= 1000000;
            currenttime.seconds++;
        }
        t->seconds = currenttime.seconds;
        t->mseconds = currenttime.useconds / 1000;
        t->useconds = currenttime.useconds;
    }
    else
    {
        /* fall back to myftime */
        struct mytimeb tb;

        myftime(&tb);
        t->seconds = (NLlong)(tb.time);
        t->mseconds = (NLlong)(tb.millitm);
        t->useconds = (NLlong)(tb.millitm * 1000);
    }
#else /* !WINDOWS_APP */
    struct timeval tv;

    if(t == NULL)
    {
		nlSetError(NL_NULL_POINTER);
		return NL_FALSE;
    }
    gettimeofday(&tv, NULL);
    t->seconds = (NLlong)(tv.tv_sec);
    t->mseconds = (NLlong)(tv.tv_usec / 1000);
    t->useconds = (NLlong)(tv.tv_usec);
#endif /* !WINDOWS_APP */
    return NL_TRUE;
}

/* Windows CE does not have time.h functions */
#if defined (_WIN32_WCE)

time_t time(time_t *timer)
{
    NLtime t;

    nlTime(&t);
    *timer = t.seconds;

    return *timer;
}

#endif

