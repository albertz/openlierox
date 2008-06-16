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

#ifndef SERIAL_H
#define SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

NLboolean serial_Init(void);
void serial_Shutdown(void);
NLboolean serial_Listen(NLsocket socket);
NLsocket serial_AcceptConnection(NLsocket socket);
NLsocket serial_Open(NLushort port, NLenum type);
NLboolean serial_Connect(NLsocket socket, NLaddress *address);
NLboolean serial_Close(NLsocket socket);
NLint serial_Read(NLsocket socket, NLvoid *buffer, NLint nbytes);
NLint serial_Write(NLsocket socket, NLvoid *buffer, NLint nbytes);
NLbyte *serial_AddrToString(NLaddress *address, NLbyte *string);
NLboolean serial_StringToAddr(NLbyte *string, NLaddress *address);
NLboolean serial_GetLocalAddr(NLsocket socket, NLaddress *address);
NLboolean serial_SetLocalAddr(NLaddress *address);
NLbyte *serial_GetNameFromAddr(NLaddress *address, NLbyte *name);
NLboolean serial_GetNameFromAddrAsync(NLaddress *address, NLbyte *name);
NLboolean serial_GetAddrFromName(NLbyte *name, NLaddress *address);
NLboolean serial_GetAddrFromNameAsync(NLbyte *name, NLaddress *address);
NLboolean serial_AddrCompare(NLaddress *address1, NLaddress *address2);
NLushort serial_GetPortFromAddr(NLaddress *address);
void serial_SetAddrPort(NLaddress *address, NLushort port);
NLint serial_GetSystemError(void);
NLint serial_PollGroup(NLint group, NLenum name, NLsocket *sockets, NLint number, NLint timeout);
NLboolean serial_Hint(NLenum name, NLint arg);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SERIAL_H */

