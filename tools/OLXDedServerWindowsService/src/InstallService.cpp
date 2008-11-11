#include <windows.h>
//#include <tchar.h>
//#include <strsafe.h>
#include <string.h>
#include <stdio.h>

#define SVCNAME "OLXDedServer"
#define SVCDISPLAYNAME "OpenLieroX dedicated server"
#define SVCEXENAME "OLXDedServerService.exe"

//
// Purpose: 
//   Installs a service in the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID SvcInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    char szPath[MAX_PATH];
    char szCommand[MAX_PATH*2];

    if( !GetModuleFileName( NULL, szPath, MAX_PATH ) )
    {
        printf("Cannot install service (%d)\n", GetLastError());
        return;
    }
    char * sep = strrchr(szPath, '\\');
    if( sep )
    {
		strcpy( sep+1, SVCEXENAME );
    }
    
    strcpy( szCommand, "\"" );
    strcat( szCommand, szPath );
    strcat( szCommand, "\"" );

    // Get a handle to the SCM database. 
 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Create the service

    schService = CreateService( 
        schSCManager,              // SCM database 
        SVCNAME,                   // name of service 
        SVCDISPLAYNAME,            // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_AUTO_START,        // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        szCommand,                 // path to service's binary 
        "extended base",           // After net services
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 
 
    if (schService == NULL) 
    {
        printf("CreateService failed (%d)\n", GetLastError()); 
        CloseServiceHandle(schSCManager);
        return;
    }
    else printf("Service installed successfully, command = %s\n", szCommand); 

    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
    
    HKEY hKey;
    DWORD dwDisp = 0;
    const char * keyAddr = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" SVCNAME;

    if(RegCreateKeyEx( HKEY_LOCAL_MACHINE, keyAddr, 0L, NULL, REG_OPTION_NON_VOLATILE, 
				KEY_ALL_ACCESS, NULL, &hKey, &dwDisp) != ERROR_SUCCESS)
    {
        printf("RegCreateKeyEx failed (%d)\n", GetLastError()); 
        return;
    }
    
    DWORD MsgTypes = 0x00000007; // The magic value from MSDN
    RegSetValueEx(hKey, "EventMessageFile", 0, REG_SZ, (const BYTE *)szPath, strlen(szPath) + 1);  
    RegSetValueEx(hKey, "TypesSupported", 0, REG_DWORD, (const BYTE *)&MsgTypes, sizeof(MsgTypes));  

    RegCloseKey(hKey);
    
}

void main(int argc, char *argv[]) 
{ 
    SvcInstall();
}
