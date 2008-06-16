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

#ifndef PARALLEL_H
#define PARALLEL_H

#ifdef __cplusplus
extern "C" {
#endif

NLboolean parallel_Init(void);
void parallel_Shutdown(void);
NLboolean parallel_Listen(NLsocket socket);
NLsocket parallel_AcceptConnection(NLsocket socket);
NLsocket parallel_Open(NLushort port, NLenum type);
NLboolean parallel_Connect(NLsocket socket, NLaddress *address);
NLboolean parallel_Close(NLsocket socket);
NLint parallel_Read(NLsocket socket, NLvoid *buffer, NLint nbytes);
NLint parallel_Write(NLsocket socket, NLvoid *buffer, NLint nbytes);
NLbyte *parallel_AddrToString(NLaddress *address, NLbyte *string);
NLboolean parallel_StringToAddr(NLbyte *string, NLaddress *address);
NLboolean parallel_GetLocalAddr(NLsocket socket, NLaddress *address);
NLboolean parallel_SetLocalAddr(NLaddress *address);
NLbyte *parallel_GetNameFromAddr(NLaddress *address, NLbyte *name);
NLboolean parallel_GetNameFromAddrAsync(NLaddress *address, NLbyte *name);
NLboolean parallel_GetAddrFromName(NLbyte *name, NLaddress *address);
NLboolean parallel_GetAddrFromNameAsync(NLbyte *name, NLaddress *address);
NLboolean parallel_AddrCompare(NLaddress *address1, NLaddress *address2);
NLushort parallel_GetPortFromAddr(NLaddress *address);
void parallel_SetAddrPort(NLaddress *address, NLushort port);
NLint parallel_GetSystemError(void);
NLint parallel_PollGroup(NLint group, NLenum name, NLsocket *sockets, NLint number, NLint timeout);
NLboolean parallel_Hint(NLenum name, NLint arg);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* PARALLEL_H */

