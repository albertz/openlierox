(*
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
*)
(*
 4th of September 2001 - Added Kylix support ( though untested )
                         Added NL_VERSION_STRING
                         Added NLVoid type declaration
                         Dominique Louis <Dominique@SavageSoftware.com.au>

 14th of February 2002 - Added FreePascal Support ( though untested )

 07th of March 2002 - Made sure the header compiles.

  2002/08/25 Sly
  - Updated to version 1.62

 5 September 2002 - Updated to version 1.63, Phil Frisbie, Jr.

 7 October 2002 - Updated to version 1.64, Phil Frisbie, Jr.

*)

unit HawkNL;

{$IFDEF FPC}
{$PACKRECORDS 4}
{$ENDIF FPC}

interface

const
  NL_MAJOR_VERSION = 1;
  NL_MINOR_VERSION = 68;
  NL_VERSION_STRING = 'HawkNL 1.68';

type
  NLbyte    = ShortInt;
  NLubyte   = Byte;
  NLshort   = SmallInt;
  NLushort  = Word;
  NLlong    = LongInt;
  NLulong   = LongWord;
  NLint     = Integer;
  NLuint    = Cardinal;
  NLsizei   = Integer;
  NLenum    = Cardinal;
  NLflags   = Cardinal;
  NLboolean = ByteBool;
  NLfloat   = Single;
  NLclampf  = Single;
  NLdouble  = Double;
  NLclampd  = Double;
  NLcstr    = PChar;
  NLvoid    = Pointer;
  NLsocket  = integer;
(* multithread *)
  NLThreadFunc = function(data: Pointer): Pointer; cdecl;
  NLthreadID = Pointer;
  NLmutex = Pointer;
  NLcond = Pointer;

  PNLaddress = ^NLaddress;
  NLaddress = record
    addr: array[0..31] of NLubyte; (* large enough to hold IPv6 address *)
    driver: NLenum;      (* driver type, not used yet *)
    valid: NLboolean;    (* set to NL_TRUE when address is valid *)
  end;

  TNLtime = record
    seconds: NLlong;     (* seconds since 12:00AM, 1 January, 1970 *)
    mseconds: NLlong;    (* milliseconds added to the seconds *)
    useconds: NLlong;    (* microseconds added to the seconds *)
  end;

const
  NL_MAX_STRING_LENGTH   = 256;
  NL_MAX_GROUPS          = 128;
  NL_MAX_GROUP_SOCKETS   = 8192;
  NL_MAX_PACKET_LENGTH   = 16384;

{ Boolean values }
  NL_INVALID              = -1;
  NL_FALSE                = 0;
  NL_TRUE                 = 1;

{ Network types }
{ Only one can init at a time }
  NL_IP                   = $0003;  { all platforms }
  NL_LOOP_BACK            = $0004;  { all platforms, for single player client/server emulation with no network}
  NL_IPX                  = $0005;  { Windows and Linux? }
  NL_SERIAL               = $0006;  { Windows and Linux only? }
  NL_MODEM                = $0007;  { Windows and Linux only? }
  NL_PARALLEL             = $0008;  { Windows and Linux only? }

{ Connection types }
  NL_RELIABLE             = $0010;  { NL_IP (TCP), NL_IPX (SPX), NL_LOOP_BACK }
  NL_UNRELIABLE           = $0011;  { NL_IP (UDP), NL_IPX, NL_LOOP_BACK }
  NL_RELIABLE_PACKETS     = $0012;  { NL_IP (TCP), NL_IPX (SPX), NL_LOOP_BACK }
  NL_BROADCAST            = $0013;  { NL_IP (UDP), NL_IPX, or NL_LOOP_BACK broadcast packets }
  NL_MULTICAST            = $0014;  { NL_IP (UDP) multicast }
  NL_RAW                  = $0015;  { NL_SERIAL or NL_PARALLEL }
{ TCP/IP specific aliases for connection types }
  NL_TCP                  = NL_RELIABLE;
  NL_TCP_PACKETS          = NL_RELIABLE_PACKETS;
  NL_UDP                  = NL_UNRELIABLE;
  NL_UDP_BROADCAST        = NL_BROADCAST;
  NL_UDP_MULTICAST        = NL_MULTICAST;

{ nlGetString }
  NL_VERSION              = $0020;  { the version string }
  NL_NETWORK_TYPES        = $0021;  { space delimited list of available network types }
  NL_CONNECTION_TYPES     = $0022;  { space delimited list of available connection types }

{ nlGetInteger, nlClear }
  NL_PACKETS_SENT         = $0030;  { total packets sent since last nlClear }
  NL_BYTES_SENT           = $0031;  { total bytes sent since last nlClear }
  NL_AVE_BYTES_SENT       = $0032;  { average bytes sent per second for the last 8 seconds }
  NL_HIGH_BYTES_SENT      = $0033;  { highest bytes per second ever sent }
  NL_PACKETS_RECEIVED     = $0034;  { total packets received since last nlClear }
  NL_BYTES_RECEIVED       = $0035;  { total bytes received since last nlClear }
  NL_AVE_BYTES_RECEIVED   = $0036;  { average bytes received per second for the last 8 seconds }
  NL_HIGH_BYTES_RECEIVED  = $0037;  { highest bytes per second ever received }
  NL_ALL_STATS            = $0038;  { nlClear only, clears out all counters }
  NL_OPEN_SOCKETS         = $0039;  { number of open sockets }

{ nlEnable, nlDisable }
  NL_BLOCKING_IO          = $0040;  { set IO to blocking, default is NL_FALSE for non-blocking IO }
  NL_SOCKET_STATS         = $0041;  { enable collection of socket read/write statistics, default disabled }
  NL_BIG_ENDIAN_DATA      = $0042;  { enable big endian data for nlSwap* and read/write macros, default enabled }
  NL_LITTLE_ENDIAN_DATA   = $0043;  { enable little endian data for nlSwap* and read/write macros, default disabled }

{ nlPollGroup }
  NL_READ_STATUS          = $0050;  { poll the read status for all sockets in the group }
  NL_WRITE_STATUS         = $0051;  { poll the write status for all sockets in the group }

{ nlHint, advanced network settings for experienced developers}
  NL_LISTEN_BACKLOG       = $0060;  { TCP, SPX: the backlog of connections for listen }
  NL_MULTICAST_TTL        = $0061;  { UDP : The multicast TTL value. Default : 1 }
  NL_REUSE_ADDRESS        = $0062;  { TCP, UDP : Allow IP address to be reused. Default : NL_FALSE }
  NL_TCP_NO_DELAY         = $0063;  { disable Nagle algorithm, only do this for a TCP/IP socket sending small packets }

{ errors }
  NL_NO_NETWORK           = $0100;  { no network was found on init }
  NL_OUT_OF_MEMORY        = $0101;  { a malloc failed }
  NL_INVALID_ENUM         = $0102;  { function called with an invalid NLenum }
  NL_INVALID_SOCKET       = $0103;  { socket is not valid, or has been terminated }
  NL_INVALID_PORT         = $0104;  { the port could not be opened }
  NL_INVALID_TYPE         = $0105;  { the network type is not available }
  NL_SYSTEM_ERROR         = $0106;  { a system error occurred, call nlGetSystemError }
  NL_SOCK_DISCONNECT      = $0107;  { the socket should be closed because of a connection loss or error }
  NL_NOT_LISTEN           = $0108;  { the socket has not been set to listen }
  NL_CON_REFUSED          = $010a;  { connection refused }
  NL_NO_PENDING           = $010c;  { there are no pending connections to accept }
  NL_BAD_ADDR             = $010d;  { the address or port are not valid }
  NL_MESSAGE_END          = $010f;  { the end of a reliable stream (TCP) message has been reached }
  NL_NULL_POINTER         = $0110;  { a NULL pointer was passed to a function }
  NL_INVALID_GROUP        = $0111;  { the group is not valid, or has been destroyed }
  NL_OUT_OF_GROUPS        = $0112;  { out of internal group objects }
  NL_OUT_OF_GROUP_SOCKETS = $0113;  { the group has no more room for sockets }
  NL_BUFFER_SIZE          = $0114;  { the buffer was too small to store the data, retry with a larger buffer }
  NL_PACKET_SIZE          = $0115;  { the size of the packet exceeds NL_MAX_PACKET_LENGTH or the protocol max }
  NL_WRONG_TYPE           = $0116;  { the function does not support the socket type }
  NL_CON_PENDING          = $0117;  { a non-blocking connection is still pending }
  NL_SELECT_NET_ERROR     = $0118;  { a network is already selected, call nlShutDown and nlInit first }
  NL_PACKET_SYNC          = $011a;  { the NL_RELIABLE_PACKET stream is out of sync }
  NL_TLS_ERROR            = $011b;  { thread local storage could not be created }
  NL_TIMED_OUT            = $011c;  { the function timed out }
  NL_SOCKET_NOT_FOUND     = $011d;  { the socket was not found in the group }
  NL_STRING_OVER_RUN      = $011e;  { the string is not null terminated, or is longer than NL_MAX_STRING_LENGTH }
  NL_MUTEX_RECURSION      = $011f;  { the mutex was recursivly locked or unlocked }
{ for backwards compatability }
  NL_SOCKET_ERROR         = NL_SYSTEM_ERROR;
  NL_CON_TERM             = NL_SOCK_DISCONNECT;

{ standard multicast TTL settings as recommended by the }
{ white paper at http://www.ipmulticast.com/community/whitepapers/howipmcworks.html }
  NL_TTL_LOCAL            = 1;       { local LAN only }
  NL_TTL_SITE             = 15;      { this site }
  NL_TTL_REGION           = 63;      { this region }
  NL_TTL_WORLD            = 127;     { the world }

{
  Low level API, a thin layer over Sockets or other network provider
}

function nlListen(socket: NLsocket):NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlAcceptConnection(socket: NLsocket): NLsocket; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlOpen(port: NLushort; tipo: NLenum): NLsocket; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlConnect(socket: NLsocket; var address: NLaddress): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlClose(socket: NLsocket): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlRead(socket: NLsocket; var buffer; nbytes: NLint): NLint; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlWrite(socket: NLsocket; const buffer; nbytes: NLint): NLint; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetSocketStat(socket: NLsocket; name: NLenum): NLlong; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlClearSocketStat(socket: NLsocket; name: NLenum): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlPollGroup(group: NLint; name: NLenum; var sockets: NLsocket; number: NLint; timeout: NLint):NLint; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlHint(name: NLenum; arg: NLint): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};

{
  Address management API
}

function nlAddrToString(var address: NLaddress;straddr: PChar): PChar; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlStringToAddr(straddr: PChar; var address: NLaddress): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetRemoteAddr(socket: NLsocket; var address: NLaddress): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlSetRemoteAddr(socket: NLsocket; var address: NLaddress): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetLocalAddr(socket: NLsocket; var address: NLaddress): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetAllLocalAddr(var count: NLint): PNLaddress; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlSetLocalAddr(const address: NLaddress): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetNameFromAddr(var address: NLaddress; name: PChar): PChar; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetNameFromAddrAsync(var address: NLaddress; name: PChar): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetAddrFromName(name: PChar; var address: NLaddress): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetAddrFromNameAsync(name: PChar; var address: NLaddress): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlAddrCompare(var address1, address2: NLaddress): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetPortFromAddr(var address: NLaddress): NLushort; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlSetAddrPort(var address: NLaddress; port: NLushort): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};


{
  Group management API
}

function nlGroupCreate: NLint; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGroupDestroy(group: NLint): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGroupAddSocket(group: NLint; socket: NLsocket): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGroupGetSockets(group: NLint; var sockets: NLsocket; var number: NLint): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGroupDeleteSocket(group: NLint; socket: NLsocket): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};

{
  Multithreading API
}

function nlThreadCreate(func: NLThreadFunc; data: Pointer; joinable: NLboolean): NLthreadID; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
procedure nlThreadYield; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlThreadJoin(threadID: NLthreadID; status: PPointer): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlMutexInit(var mutex: NLmutex): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlMutexLock(var mutex: NLmutex): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlMutexUnlock(var mutex: NLmutex): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlMutexDestroy(var mutex: NLmutex): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlCondInit(var cond: NLcond): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlCondWait(var cond: NLcond; timeout: NLint): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlCondSignal(var cond: NLcond): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlCondBroadcast(var cond: NLcond): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlCondDestroy(var cond: NLcond): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};

{
  Time API
}

function nlTime(var ts: TNLtime): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};

{
  Misc. API
}

function nlInit: NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
procedure nlShutdown; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlSelectNetwork(network: NLenum): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetString(name: NLenum): PChar; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetInteger(name: NLenum): NLlong; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetBoolean(name: NLenum): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlClear(name: NLenum): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetError: NLenum; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetErrorStr(err: NLenum): PChar; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetSystemError: NLint; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetSystemErrorStr(err: NLint): PChar; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlEnable(name: NLenum): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlDisable(name: NLenum): NLboolean; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetCRC16(var data: PChar; len: NLint): NLushort; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlGetCRC32(var data: PChar; len: NLint): NLulong; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlSwaps(x: NLushort): NLushort; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlSwapl(x: NLulong): NLulong; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlSwapf(f: NLfloat): NLfloat; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};
function nlSwapd(d: NLdouble): NLdouble; {$IFDEF WIN32} stdcall {$ELSE} cdecl {$ENDIF};

implementation

const
{$IFDEF WIN32}
  HAWKNL_DLL = 'HawkNL.dll';
{$ENDIF}
{$IFDEF LINUX}
  HAWKNL_DLL = 'libNL.so';
{$ENDIF}

function nlListen; external HAWKNL_DLL;
function nlAcceptConnection; external HAWKNL_DLL;
function nlOpen; external HAWKNL_DLL;
function nlConnect; external HAWKNL_DLL;
function nlClose; external HAWKNL_DLL;
function nlRead; external HAWKNL_DLL;
function nlWrite; external HAWKNL_DLL;
function nlGetSocketStat; external HAWKNL_DLL;
function nlClearSocketStat; external HAWKNL_DLL;
function nlPollGroup; external HAWKNL_DLL;
function nlHint; external HAWKNL_DLL;

{
  Address management API
}

function nlAddrToString; external HAWKNL_DLL;
function nlStringToAddr; external HAWKNL_DLL;
function nlGetRemoteAddr; external HAWKNL_DLL;
function nlSetRemoteAddr; external HAWKNL_DLL;
function nlGetLocalAddr; external HAWKNL_DLL;
function nlGetAllLocalAddr; external HAWKNL_DLL;
function nlSetLocalAddr; external HAWKNL_DLL;
function nlGetNameFromAddr; external HAWKNL_DLL;
function nlGetNameFromAddrAsync; external HAWKNL_DLL;
function nlGetAddrFromName; external HAWKNL_DLL;
function nlGetAddrFromNameAsync; external HAWKNL_DLL;
function nlAddrCompare; external HAWKNL_DLL;
function nlGetPortFromAddr; external HAWKNL_DLL;
function nlSetAddrPort; external HAWKNL_DLL;

{
  Group management API
}

function nlGroupCreate; external HAWKNL_DLL;
function nlGroupDestroy; external HAWKNL_DLL;
function nlGroupAddSocket; external HAWKNL_DLL;
function nlGroupGetSockets; external HAWKNL_DLL;
function nlGroupDeleteSocket; external HAWKNL_DLL;

{
  Multithreading API
}

function nlThreadCreate; external HAWKNL_DLL;
procedure nlThreadYield; external HAWKNL_DLL;
function nlThreadJoin; external HAWKNL_DLL;
function nlMutexInit; external HAWKNL_DLL;
function nlMutexLock; external HAWKNL_DLL;
function nlMutexUnlock; external HAWKNL_DLL;
function nlMutexDestroy; external HAWKNL_DLL;
function nlCondInit; external HAWKNL_DLL;
function nlCondWait; external HAWKNL_DLL;
function nlCondSignal; external HAWKNL_DLL;
function nlCondBroadcast; external HAWKNL_DLL;
function nlCondDestroy; external HAWKNL_DLL;

{
  Time API
}

function nlTime; external HAWKNL_DLL;

{
  Misc. API
}

function nlInit; external HAWKNL_DLL;
procedure nlShutdown; external HAWKNL_DLL;
function nlSelectNetwork; external HAWKNL_DLL;
function nlGetString; external HAWKNL_DLL;
function nlGetInteger; external HAWKNL_DLL;
function nlGetBoolean; external HAWKNL_DLL;
function nlClear; external HAWKNL_DLL;
function nlGetError; external HAWKNL_DLL;
function nlGetErrorStr; external HAWKNL_DLL;
function nlGetSystemError; external HAWKNL_DLL;
function nlGetSystemErrorStr; external HAWKNL_DLL;
function nlEnable; external HAWKNL_DLL;
function nlDisable; external HAWKNL_DLL;
function nlGetCRC16; external HAWKNL_DLL;
function nlGetCRC32; external HAWKNL_DLL;
function nlSwaps; external HAWKNL_DLL;
function nlSwapl; external HAWKNL_DLL;
function nlSwapf; external HAWKNL_DLL;
function nlSwapd; external HAWKNL_DLL;

end.
