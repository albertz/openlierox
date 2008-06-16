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

#ifndef IPX_H
#define IPX_H

#ifdef __cplusplus
extern "C" {
#endif

NLboolean ipx_Init(void);
void ipx_Shutdown(void);
NLboolean ipx_Listen(NLsocket socket);
NLsocket ipx_AcceptConnection(NLsocket socket);
NLsocket ipx_Open(NLushort port, NLenum type);
NLboolean ipx_Connect(NLsocket socket, const NLaddress *address);
void ipx_Close(NLsocket socket);
NLint ipx_Read(NLsocket socket, /*@out@*/ NLvoid *buffer, NLint nbytes);
NLint ipx_Write(NLsocket socket, const NLvoid *buffer, NLint nbytes);
NLchar *ipx_AddrToString(const NLaddress *address, /*@returned@*/ /*@out@*/ NLchar *string);
NLboolean ipx_StringToAddr(const NLchar *string, /*@out@*/ NLaddress *address);
NLboolean ipx_GetLocalAddr(NLsocket socket, /*@out@*/ NLaddress *address);
NLaddress *ipx_GetAllLocalAddr(NLint *count);
NLboolean ipx_SetLocalAddr(const NLaddress *address);
NLchar *ipx_GetNameFromAddr(const NLaddress *address, /*@returned@*/ /*@out@*/ NLchar *name);
NLboolean ipx_GetNameFromAddrAsync(const NLaddress *address, /*@out@*/ NLchar *name);
NLboolean ipx_GetAddrFromName(const NLchar *name, /*@out@*/ NLaddress *address);
NLboolean ipx_GetAddrFromNameAsync(const NLchar *name, /*@out@*/ NLaddress *address);
NLboolean ipx_AddrCompare(const NLaddress *address1, const NLaddress *address2);
NLushort ipx_GetPortFromAddr(const NLaddress *address);
void ipx_SetAddrPort(NLaddress *address, NLushort port);
NLint ipx_GetSystemError(void);
NLint ipx_PollGroup(NLint group, NLenum name, /*@out@*/ NLsocket *sockets, NLint number, NLint timeout);
NLboolean ipx_Hint(NLenum name, NLint arg);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* IPX_H */

