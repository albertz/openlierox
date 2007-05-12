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

#ifndef LOOPBACK_H
#define LOOPBACK_H

#ifdef __cplusplus
extern "C" {
#endif

NLboolean loopback_Init(void);
void loopback_Shutdown(void);
NLboolean loopback_Listen(NLsocket socket);
NLsocket loopback_AcceptConnection(NLsocket socket);
NLsocket loopback_Open(NLushort port, NLenum type);
NLboolean loopback_Connect(NLsocket socket, const NLaddress *address);
void loopback_Close(NLsocket socket);
NLint loopback_Read(NLsocket socket, /*@out@*/ NLvoid *buffer, NLint nbytes);
NLint loopback_Write(NLsocket socket, const NLvoid *buffer, NLint nbytes);
NLchar *loopback_AddrToString(const NLaddress *address, /*@returned@*/ /*@out@*/ NLchar *string);
NLboolean loopback_StringToAddr(const NLchar *string, /*@out@*/ NLaddress *address);
NLboolean loopback_GetLocalAddr(/*@unused@*/ NLsocket socket, /*@out@*/ NLaddress *address);
NLaddress *loopback_GetAllLocalAddr(/*@out@*/ NLint *count);
NLboolean loopback_SetLocalAddr(/*@unused@*/ const NLaddress *address);
NLchar *loopback_GetNameFromAddr(const NLaddress *address, /*@returned@*/ /*@out@*/ NLchar *name);
NLboolean loopback_GetNameFromAddrAsync(const NLaddress *address, /*@out@*/ NLchar *name);
NLboolean loopback_GetAddrFromName(const NLchar *name, /*@out@*/ NLaddress *address);
NLboolean loopback_GetAddrFromNameAsync(const NLchar *name, /*@out@*/ NLaddress *address);
NLboolean loopback_AddrCompare(const NLaddress *address1, const NLaddress *address2);
NLushort loopback_GetPortFromAddr(const NLaddress *address);
void loopback_SetAddrPort(NLaddress *address, NLushort port);
NLint loopback_GetSystemError(void);
NLint loopback_PollGroup(NLint group, NLenum name, /*@out@*/ NLsocket *sockets, NLint number, NLint timeout);
NLboolean loopback_Hint(NLenum name, NLint arg);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* LOOPBACK_H */

