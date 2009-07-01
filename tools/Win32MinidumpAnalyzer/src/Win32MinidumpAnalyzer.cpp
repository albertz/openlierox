///////////////////////////////////////////////////////////////////////////////
//
// ReadUserStream.cpp 
//
// This example shows how to read user data streams from the minidump 
// 
// Author: Oleg Starodumov (www.debuginfo.com)
//
//


///////////////////////////////////////////////////////////////////////////////
// Include files 
//

#include <windows.h>
#include <tchar.h>
#include <dbghelp.h>
#include <crtdbg.h>

#include <stdio.h>


///////////////////////////////////////////////////////////////////////////////
// Directives 
//

#pragma comment( lib, "dbghelp.lib" )


///////////////////////////////////////////////////////////////////////////////
// Function declarations 
//

void ShowUserStreams( PVOID pMiniDump, int streamNum ); 


///////////////////////////////////////////////////////////////////////////////
// Stream identifiers 
//

// Stream identifiers 
// (LastReservedStream constant is defined in MINIDUMP_STREAM_TYPE 
// enumeration in DbgHelp.h; all user data stream identifiers 
// must be larger than LastReservedStream) 
const ULONG32 cFirstStreamID = LastReservedStream + 1; 


///////////////////////////////////////////////////////////////////////////////
// main() function 
//

int _tmain(int argc, TCHAR* argv[])
{
	// Check parameters 

	if( argc < 2 ) 
	{
		_tprintf( _T("Usage: %s <MiniDumpFile> [StreamIndex]\n"), argv[0] ); 
		return 0; 
	}

	const TCHAR* pFileName = argv[1]; 

	//_tprintf( _T("Minidump: %s \n"), pFileName ); 


	// Read the user data streams and display their contents 

	HANDLE hFile = NULL; 

	HANDLE hMapFile = NULL; 

	PVOID pViewOfFile = NULL; 

	do 
	{
		// Map the minidump into memory 

		hFile = CreateFile( pFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL ); 

		if( ( hFile == NULL ) || ( hFile == INVALID_HANDLE_VALUE ) ) 
		{
			_tprintf( _T("Error: CreateFile failed. Error: %u \n"), GetLastError() ); 
			break;     
		}

		hMapFile = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 0, NULL ); 

		if( hMapFile == NULL ) 
		{
			_tprintf( _T("Error: CreateFileMapping failed. Error: %u \n"), GetLastError() ); 
			break;     
		}

		pViewOfFile = MapViewOfFile( hMapFile, FILE_MAP_READ, 0, 0, 0 ); 

		if( pViewOfFile == NULL ) 
		{
			_tprintf( _T("Error: MapViewOfFile failed. Error: %u \n"), GetLastError() ); 
			break;     
		}


		// Show the contents of user data streams 
		int streamNum = 0;
		if( argc > 2  )
			streamNum = atoi(argv[2]);
		ShowUserStreams( pViewOfFile, streamNum ); 

	}
	while( 0 ); 


	// Cleanup 

	if( hMapFile != NULL ) 
		CloseHandle( hMapFile ); 

	if( hFile != NULL ) 
		CloseHandle( hFile ); 


	// Complete 

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// ShowUserStreams function 
//

void ShowUserStreams( PVOID pMiniDump, int streamNum ) 
{
	// Check parameters 

	if( pMiniDump == 0 ) 
	{
		_ASSERTE( !_T("Invalid parameter.") ); 
		return; 
	}


	// Show the contents of user data streams 

		// First stream 

	ULONG StreamID = cFirstStreamID + streamNum; 

	PMINIDUMP_DIRECTORY  pMiniDumpDir  = 0; 
	PVOID                pStream       = 0; 
	ULONG                StreamSize    = 0; 

	if( !MiniDumpReadDumpStream( pMiniDump, StreamID, &pMiniDumpDir, 
	                             &pStream, &StreamSize ) ) 
	{
		DWORD ErrCode = GetLastError(); 
		if( ErrCode != 0 ) // 0 -> no such stream in the dump 
			_tprintf( _T("Error: MiniDumpReadDumpStream failed. Error: %u \n"), ErrCode ); 
		else 
			_tprintf( _T("Stream (id %u) not found in the minidump.\n"), StreamID ); 
	}
	else 
	{
		// Show the contents 

		if( ( pStream == 0 ) || ( StreamSize == 0 ) ) 
		{
			_tprintf( _T("Invalid stream (id %u).\n"), StreamID ); 
		}
		else if( IsBadStringPtrA( (LPCSTR)pStream, StreamSize ) ) 
		{
			_tprintf( _T("Invalid stream data (id %u).\n"), StreamID ); 
		}
		else 
		{
			_tprintf( _T("%s\n"), pStream ); 
		}
	}

}


