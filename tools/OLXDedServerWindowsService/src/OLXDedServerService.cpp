
#include <windows.h>
#include <tlhelp32.h>
#include <string.h>
#include <stdio.h>
#include "OLXDedServerServiceMsgs.h"

#pragma warning(disable: 4996)

#define SVCNAME "OLXDedServer"
#define OLX_TIMEOUT 60
#define OLX_EXE "OpenLieroX.exe"

SERVICE_STATUS          gSvcStatus; 
SERVICE_STATUS_HANDLE   gSvcStatusHandle; 
HANDLE                  ghSvcStopEvent = NULL;

VOID WINAPI SvcCtrlHandler( DWORD ); 
VOID WINAPI SvcMain( DWORD, LPTSTR * ); 

VOID ReportSvcStatus( DWORD, DWORD, DWORD );
VOID SvcInit( DWORD, LPTSTR * ); 
VOID SvcReportEvent( LPTSTR );

//
// Purpose: 
//   Entry point for the process
//
// Parameters:
//   None
// 
// Return value:
//   None
//
void main(int argc, char *argv[]) 
{ 
    // If command-line parameter is "install", install the service. 
    // Otherwise, the service is probably being started by the SCM.

    // TO_DO: Add any additional services for the process to this table.
    SERVICE_TABLE_ENTRY DispatchTable[] = 
    { 
        { SVCNAME, (LPSERVICE_MAIN_FUNCTION) SvcMain }, 
        { NULL, NULL } 
    }; 
 
    // This call returns when the service has stopped. 
    // The process should simply terminate when the call returns.

    if (!StartServiceCtrlDispatcher( DispatchTable )) 
    { 
        SvcReportEvent("StartServiceCtrlDispatcher"); 
    } 
} 


//
// Purpose: 
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None.
//
VOID WINAPI SvcMain( DWORD dwArgc, LPTSTR *lpszArgv )
{
    // Register the handler function for the service

    gSvcStatusHandle = RegisterServiceCtrlHandler( 
        SVCNAME, 
        SvcCtrlHandler);

    if( !gSvcStatusHandle )
    { 
        SvcReportEvent(TEXT("RegisterServiceCtrlHandler")); 
        return; 
    } 

    // These SERVICE_STATUS members remain as set here

    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
    gSvcStatus.dwServiceSpecificExitCode = 0;    

    // Report initial status to the SCM

    ReportSvcStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );

    // Perform service-specific initialization and work.

    SvcInit( dwArgc, lpszArgv );
}

//
// Purpose: 
//   The service code
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None
//

BOOL QuitSignalHandler( DWORD fdwCtrlType ) 
{ 
	return TRUE; // Do not pass signal further
}


VOID SvcInit( DWORD dwArgc, LPTSTR *lpszArgv)
{
    // TO_DO: Declare and set any required variables.
    //   Be sure to periodically call ReportSvcStatus() with 
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportSvcStatus with SERVICE_STOPPED.

    // Create an event. The control handler function, SvcCtrlHandler,
    // signals this event when it receives the stop control code.

    ghSvcStopEvent = CreateEvent(
                         NULL,    // default security attributes
                         TRUE,    // manual reset event
                         FALSE,   // not signaled
                         NULL);   // no name

    if ( ghSvcStopEvent == NULL)
    {
        ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
        return;
    }

    // Report running status when initialization is complete.

    ReportSvcStatus( SERVICE_RUNNING, NO_ERROR, 0 );
    
    //AllocConsole(); // So we can send Ctrl-Break (dunno if it helps really)
    
   	//SetConsoleCtrlHandler( (PHANDLER_ROUTINE) QuitSignalHandler, TRUE );

    
    char szPath[MAX_PATH] = "";
    char szCurDir[MAX_PATH] = "";
    char szCommand[MAX_PATH*2] = OLX_EXE " -dedicated";

    GetModuleFileName( NULL, szPath, MAX_PATH );
    char * sep = strrchr(szPath, '\\');
    if( sep )
    {
		strcpy(szCurDir, szPath);
		strrchr(szCurDir, '\\')[0] = 0;
		//strcpy( sep+1, OLX_EXE );
    }
    
    //strcpy( szCommand, "\"" );
    //strcat( szCommand, szPath );
    //strcat( szCommand, "\" -dedicated" );
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	memset(&pi, 0, sizeof(pi));

	if( ! CreateProcess( NULL, szCommand, NULL, NULL, false, 
							NORMAL_PRIORITY_CLASS /*| CREATE_NEW_PROCESS_GROUP*/,
							NULL, szCurDir, &si, &pi ))
	{
		char buf[1024];
		char errorBuf[1024] = "Unknown error";
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, errorBuf, sizeof(errorBuf), NULL );
		sprintf(buf, "Cannot start OpenLieroX EXE, cmd '%s', curdir '%s', error: %s", szCommand, szCurDir, errorBuf);
        SvcReportEvent(buf); 
		ReportSvcStatus( SERVICE_STOPPED, ERROR_FILE_NOT_FOUND, 0 );
		return;
	};

    while(1)
    {
        // Check whether to stop the service.
        if( WaitForSingleObject(ghSvcStopEvent, 1000) != WAIT_TIMEOUT )
        {
			PostThreadMessage( pi.dwThreadId, WM_CLOSE, 0, 0 );
				
			HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
			THREADENTRY32 te32; 
			// Take a snapshot of all running threads  
			hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
			if( hThreadSnap != INVALID_HANDLE_VALUE )
			{
				// Fill in the size of the structure before using it. 
				te32.dwSize = sizeof(THREADENTRY32); 

				// Retrieve information about the first thread,
				// and exit if unsuccessful
				if( Thread32First( hThreadSnap, &te32 ) ) 
				{
					// Now walk the thread list of the system,
					// and display information about each thread
					// associated with the specified process
					do 
					{ 
						if( te32.th32OwnerProcessID == pi.dwProcessId )
						{
			  				PostThreadMessage( te32.th32ThreadID, WM_CLOSE, 0, 0 );
						}
					} while( Thread32Next(hThreadSnap, &te32 ) ); 
				}
				CloseHandle( hThreadSnap );			
			}
			
			// Kill it
			Sleep( 2000 );
			TerminateProcess( pi.hProcess, 0 );
			ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
			return;
		};
	    ReportSvcStatus( SERVICE_RUNNING, NO_ERROR, 0 );
    }
}

//
// Purpose: 
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, 
//     in milliseconds
// 
// Return value:
//   None
//
VOID ReportSvcStatus( DWORD dwCurrentState,
                      DWORD dwWin32ExitCode,
                      DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ( (dwCurrentState == SERVICE_RUNNING) ||
           (dwCurrentState == SERVICE_STOPPED) )
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus( gSvcStatusHandle, &gSvcStatus );
}

//
// Purpose: 
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
// 
// Return value:
//   None
//
VOID WINAPI SvcCtrlHandler( DWORD dwCtrl )
{
   // Handle the requested control code. 

   switch(dwCtrl) 
   {  
      case SERVICE_CONTROL_STOP: 
         ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

         // Signal the service to stop.

         SetEvent(ghSvcStopEvent);
         
         return;
 
      case SERVICE_CONTROL_INTERROGATE: 
         // Fall through to send current status.
         break; 
 
      default: 
         break;
   } 

   ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
}

//
// Purpose: 
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
// 
// Return value:
//   None
//
// Remarks:
//   The service must have an entry in the Application event log.
//
VOID SvcReportEvent(LPTSTR szFunction) 
{ 
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    //TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, SVCNAME);

    if( NULL != hEventSource )
    {
        //sprintf(Buffer, "%s failed with %d", szFunction, GetLastError());

        lpszStrings[0] = SVCNAME;
        lpszStrings[1] = szFunction;

        ReportEvent(hEventSource,        // event log handle
                    EVENTLOG_WARNING_TYPE, // event type
                    0,                   // event category
                    SVC_ERROR,			 // Custom event ID from our MC file
                    NULL,                // no security identifier
                    2,                   // size of lpszStrings array
                    0,                   // no binary data
                    lpszStrings,         // array of strings
                    NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}