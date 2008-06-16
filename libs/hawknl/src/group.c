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

#include <string.h>

#define FD_SETSIZE              8192

#if defined WIN32 || defined WIN64 || defined (_WIN32_WCE)
/* Windows systems */
#ifdef _MSC_VER
#pragma warning (disable:4201)
#pragma warning (disable:4214)
#pragma warning (disable:4127)
#endif /* _MSC_VER */

#define WIN32_LEAN_AND_MEAN
#include <winsock.h>

#ifdef _MSC_VER
#pragma warning (default:4201)
#pragma warning (default:4214)
#endif /* _MSC_VER */

#else
/* Unix-style systems or macs with posix support */

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#define INVALID_SOCKET -1
#define SOCKET int
#endif

#include "nlinternal.h"

static NLmutex  grouplock;

typedef struct
{
    NLsocket    *sockets;   /* the list of sockets in this group */
    NLint       maxsockets; /* the number of sockets allocated */
    NLint       numsockets; /* the number of sockets stored */
    fd_set      *fdset;     /* for nlPollGroup */
    SOCKET      highest;    /* for nlPollGroup */
} nl_group_t;

typedef /*@only@*/ nl_group_t *pnl_group_t;
static /*@only@*/ pnl_group_t *groups;
static NLint nlnextgroup = 0;
static NLint nlnumgroups = 0;


/* Internal functions */

void nlGroupLock(void)
{
    (void)nlMutexLock(&grouplock);
}

void nlGroupUnlock(void)
{
    (void)nlMutexUnlock(&grouplock);
}

NLboolean nlGroupInit(void)
{
    if(groups == NULL)
    {
        groups = (nl_group_t **)malloc(NL_MAX_GROUPS * sizeof(nl_group_t *));
    }
    if(groups == NULL)
    {
        nlSetError(NL_OUT_OF_MEMORY);
        return NL_FALSE;
    }
    memset(groups, 0, NL_MAX_GROUPS * sizeof(nl_group_t *));
    if(nlMutexInit(&grouplock) == NL_FALSE)
    {
        /* error code is already set */
        return NL_FALSE;
    }
    return NL_TRUE;
}

void nlGroupShutdown(void)
{
    if(groups != NULL)
    {
        NLint i;

        for(i=0;i<NL_MAX_GROUPS;i++)
        {
            if(groups[i] != NULL)
            {
                (void)nlGroupDestroy(i + NL_FIRST_GROUP);
            }
        }
        free(groups);
        groups = NULL;
    }
    (void)nlMutexDestroy(&grouplock);
}

SOCKET nlGroupGetFdset(NLint group, fd_set *fd)
{
    NLint       realgroup = group - NL_FIRST_GROUP;
    nl_group_t  *pgroup = NULL;

    if(groups == NULL)
    {
        nlSetError(NL_NO_NETWORK);
        return INVALID_SOCKET;
    }
    if(realgroup < 0)
    {
        nlSetError(NL_INVALID_GROUP);
        return INVALID_SOCKET;
    }
    pgroup = groups[realgroup];
    if(pgroup == NULL)
    {
        nlSetError(NL_INVALID_GROUP);
        return INVALID_SOCKET;
    }
    /* if fdset is NULL, then create it */
    if(pgroup->fdset == NULL)
    {
        int     i;
        SOCKET  realsock;
        /* create the fd_set */
        pgroup->fdset = (fd_set *)malloc(sizeof(fd_set));
        if(pgroup->fdset == NULL)
        {
            (void)nlMutexUnlock(&grouplock);
            nlSetError(NL_OUT_OF_MEMORY);
            return INVALID_SOCKET;
        }
        FD_ZERO(pgroup->fdset);
        pgroup->highest = 0;
        for(i=0;i<pgroup->numsockets;i++)
        {
            realsock = (SOCKET)nlSockets[pgroup->sockets[i]]->realsocket;
            FD_SET(realsock, pgroup->fdset);
            if(pgroup->highest < realsock + 1)
            {
                pgroup->highest = realsock + 1;
            }
        }
    }
    memcpy(fd, pgroup->fdset, sizeof(fd_set));

    return pgroup->highest;
}

/* Group management API */

NL_EXP NLint NL_APIENTRY nlGroupCreate(void)
{
    NLint       newgroup = NL_INVALID;
    nl_group_t  *pgroup = NULL;

    if(groups == NULL)
    {
        nlSetError(NL_NO_NETWORK);
        return NL_INVALID;
    }
    if(nlMutexLock(&grouplock) == NL_FALSE)
    {
        return NL_INVALID;
    }
    if(nlnumgroups == NL_MAX_GROUPS)
    {
        (void)nlMutexUnlock(&grouplock);
        nlSetError(NL_OUT_OF_GROUPS);
        return NL_INVALID;
    }
    /* get a group number */
    if(nlnumgroups == nlnextgroup)
    {
        /* do not increment nlnextgroup here, wait in case of malloc failure */
        newgroup = nlnextgroup + 1;
    }
    else
    /* there is an open group slot somewhere below nlnextgroup */
    {
        NLint   i;

        for(i=0;i<nlnextgroup;i++)
        {
            if(groups[i] == NULL)
            {
                /* found an open group slot */
                newgroup = i;
            }
        }
        /* let's check just to make sure we did find a group */
        if(newgroup == NL_INVALID)
        {
            (void)nlMutexUnlock(&grouplock);
            nlSetError(NL_OUT_OF_MEMORY);
            return NL_INVALID;
        }
    }
    /* allocate the memory */
    pgroup = (nl_group_t *)malloc((size_t)(sizeof(nl_group_t)));
    if(pgroup == NULL)
    {
        (void)nlMutexUnlock(&grouplock);
        nlSetError(NL_OUT_OF_MEMORY);
        return NL_INVALID;
    }
    else
    {
        NLint   i;

        pgroup->sockets = (NLsocket *)malloc(NL_MIN_SOCKETS * sizeof(NLsocket *));
        if(pgroup->sockets == NULL)
        {
            free(pgroup);
            (void)nlMutexUnlock(&grouplock);
            nlSetError(NL_OUT_OF_MEMORY);
            return NL_INVALID;
        }
        pgroup->maxsockets = NL_MIN_SOCKETS;
        /* fill with -1, since 0 is a valid socket number */
        for(i=0;i<pgroup->maxsockets;i++)
        {
            pgroup->sockets[i] =  -1;
        }
        pgroup->numsockets = 0;
        pgroup->fdset = NULL;
        pgroup->highest = 0;
        groups[newgroup] = pgroup;
    }

    nlnumgroups++;
    if(nlnumgroups == newgroup)
    {
        nlnextgroup = nlnumgroups;
    }
    if(nlMutexUnlock(&grouplock) == NL_FALSE)
    {
        return NL_INVALID;
    }
    /* adjust the group number */
    return (newgroup + NL_FIRST_GROUP);
}

NL_EXP NLboolean NL_APIENTRY nlGroupDestroy(NLint group)
{
    NLint   realgroup = group - NL_FIRST_GROUP;

    if(groups == NULL)
    {
        nlSetError(NL_NO_NETWORK);
        return NL_FALSE;
    }
    if(realgroup < 0)
    {
        nlSetError(NL_INVALID_GROUP);
        return NL_FALSE;
    }
    if(nlMutexLock(&grouplock) == NL_FALSE)
    {
        return NL_FALSE;
    }
    if(groups[realgroup] != NULL)
    {
        if(groups[realgroup]->fdset != NULL)
        {
            free(groups[realgroup]->fdset);
        }
        if(groups[realgroup]->sockets != NULL)
        {
            free(groups[realgroup]->sockets);
        }
        free(groups[realgroup]);
        groups[realgroup] = NULL;
        nlnumgroups--;
    }
    if(nlMutexUnlock(&grouplock) == NL_FALSE)
    {
        return NL_FALSE;
    }
    return NL_TRUE;
}

NL_EXP NLboolean NL_APIENTRY nlGroupAddSocket(NLint group, NLsocket socket)
{
    NLint       realgroup = group - NL_FIRST_GROUP;
    NLint       i;
    nl_group_t  *pgroup = NULL;

    if(groups == NULL)
    {
        nlSetError(NL_NO_NETWORK);
        return NL_FALSE;
    }
    if(realgroup < 0)
    {
        nlSetError(NL_INVALID_GROUP);
        return NL_FALSE;
    }

    /* add the socket to the group */
    if(nlMutexLock(&grouplock) == NL_FALSE)
    {
        return NL_FALSE;
    }
    pgroup = groups[realgroup];
    /* allocate more sockets as needed */
    if(pgroup->numsockets == pgroup->maxsockets)
    {
        NLint       oldmax = pgroup->maxsockets;
        NLint       j;
        NLsocket    *newsockets;

        if(oldmax == NL_MAX_GROUP_SOCKETS)
        {
            (void)nlMutexUnlock(&grouplock);
            nlSetError(NL_OUT_OF_GROUP_SOCKETS);
            return NL_FALSE;
        }
        pgroup->maxsockets *= 2;
        if(pgroup->maxsockets > NL_MAX_GROUP_SOCKETS)
        {
            pgroup->maxsockets = NL_MAX_GROUP_SOCKETS;
        }
        if((newsockets = (NLsocket *)realloc(pgroup->sockets, pgroup->maxsockets * sizeof(NLsocket *))) == NULL)
        {
            pgroup->maxsockets = oldmax;
            (void)nlMutexUnlock(&grouplock);
            nlSetError(NL_OUT_OF_MEMORY);
            return NL_FALSE;
        }
        /* set the new sockets to -1 */
        for(j=oldmax;j<pgroup->maxsockets;j++)
        {
            newsockets[j] = -1;
        }
        pgroup->sockets = newsockets;
    }

    for(i=0;i<pgroup->maxsockets;i++)
    {
        if(pgroup->sockets[i] == -1)
        {
            pgroup->sockets[i] = socket;
            if(pgroup->fdset != NULL)
            {
                SOCKET realsock;

                /* make sure the socket is valid */
                if(nlIsValidSocket(socket) == NL_FALSE)
                {
                    (void)nlMutexUnlock(&grouplock);
                    nlSetError(NL_INVALID_SOCKET);
                    return NL_FALSE;
                }
                realsock = (SOCKET)nlSockets[socket]->realsocket;
                FD_SET(realsock, pgroup->fdset);
                if(pgroup->highest < realsock + 1)
                {
                    pgroup->highest = realsock + 1;
                }
            }
            break;
        }
    }
    if(i == pgroup->maxsockets)
    {
        (void)nlMutexUnlock(&grouplock);
        nlSetError(NL_OUT_OF_GROUP_SOCKETS);
        return NL_FALSE;
    }
    pgroup->numsockets++;
    if(nlMutexUnlock(&grouplock) == NL_FALSE)
    {
        return NL_FALSE;
    }
    return NL_TRUE;
}

NLboolean nlGroupGetSocketsINT(NLint group, NLsocket *socket, NLint *number)
{
    NLint       realgroup = group - NL_FIRST_GROUP;
    NLint       len, i;
    nl_group_t  *pgroup = NULL;

    if(socket == NULL || number == NULL)
    {
        nlSetError(NL_NULL_POINTER);
        return NL_FALSE;
    }
    if(groups == NULL)
    {
        nlSetError(NL_NO_NETWORK);
        return NL_FALSE;
    }
    if(realgroup < 0)
    {
        nlSetError(NL_INVALID_GROUP);
        *number = 0;
        return NL_FALSE;
    }
    pgroup = groups[realgroup];
	len = *number;
    if(len > pgroup->numsockets)
    {
        len = pgroup->numsockets;
    }
    for(i=0;i<len;i++)
    {
        socket[i] = pgroup->sockets[i];
    }
    *number = len;
    return NL_TRUE;
}

NL_EXP NLboolean NL_APIENTRY nlGroupGetSockets(NLint group, NLsocket *socket, NLint *number)
{
    NLboolean result;

    if(nlMutexLock(&grouplock) == NL_FALSE)
    {
        return NL_FALSE;
    }
    result = nlGroupGetSocketsINT(group, socket, number);
    if(nlMutexUnlock(&grouplock) == NL_FALSE)
    {
        return NL_FALSE;
    }
    return result;
}

NL_EXP NLboolean NL_APIENTRY nlGroupDeleteSocket(NLint group, NLsocket socket)
{
    NLint       realgroup = group - NL_FIRST_GROUP;
    NLint       i;
    nl_group_t  *pgroup = NULL;

    if(groups == NULL)
    {
        nlSetError(NL_NO_NETWORK);
        return NL_FALSE;
    }
    if(realgroup < 0)
    {
        nlSetError(NL_INVALID_GROUP);
        return NL_FALSE;
    }

    /* delete the socket from the group */
    if(nlMutexLock(&grouplock) == NL_FALSE)
    {
        return NL_FALSE;
    }
    pgroup = groups[realgroup];
    for(i=0;i<pgroup->numsockets;i++)
    {
        /* check for match */
        if(pgroup->sockets[i] == socket)
            break;
    }
    if(i == pgroup->numsockets)
    {
        /* did not find the socket */
        (void)nlMutexUnlock(&grouplock);
        nlSetError(NL_SOCKET_NOT_FOUND);
        return NL_FALSE;
    }
    /* now pgroup[i] points to the socket to delete */
    /* shift all other sockets down to close the gap */
    i++;
    for(;i<pgroup->maxsockets;i++)
    {
        pgroup->sockets[i - 1] = pgroup->sockets[i];
        /* check for end of list */
        if(pgroup->sockets[i] == -1)
            break;
    }
    pgroup->numsockets--;
    if(pgroup->fdset != NULL)
    {
        /* make sure the socket is valid */
        if(nlIsValidSocket(socket) == NL_TRUE)
        {
            SOCKET realsock;

            realsock = (SOCKET)nlSockets[socket]->realsocket;
            FD_CLR(realsock, pgroup->fdset);
        }
        else
        {
            /* the socket was already closed */
            /* free the fdset so that it can be rebuilt */
            free(pgroup->fdset);
            pgroup->fdset = NULL;
            (void)nlMutexUnlock(&grouplock);
            nlSetError(NL_INVALID_SOCKET);
            return NL_FALSE;
        }
    }
    if(nlMutexUnlock(&grouplock) == NL_FALSE)
    {
        return NL_FALSE;
    }
    return NL_TRUE;
}
