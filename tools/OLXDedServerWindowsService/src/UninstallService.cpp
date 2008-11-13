
#include <windows.h>
//#include <tchar.h>
//#include <strsafe.h>
#include <string.h>
#include <stdio.h>

#define SVCNAME "OLXDedServer"

VOID DoDeleteSvc()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

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

    // Get a handle to the service.

    schService = OpenService( 
        schSCManager,       // SCM database 
        SVCNAME,          // name of service 
        DELETE | SERVICE_STOP | SERVICE_QUERY_STATUS );            // need delete access 
 
    if (schService == NULL)
    { 
        printf("OpenService failed (%d)\n", GetLastError()); 
        CloseServiceHandle(schSCManager);
        return;
    }
	
	//Stop service
	SERVICE_STATUS ssp;
	ControlService( schService, SERVICE_CONTROL_STOP, &ssp );
	Sleep(300);
 
    // Delete the service.
    if (! DeleteService(schService) ) 
    {
		char errorBuf[1024] = "Unknown error";
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, errorBuf, sizeof(errorBuf), NULL );
        printf("DeleteService failed - %s - reboot and try again\n", errorBuf); 
    }
    else printf("Service deleted successfully\n"); 
 
    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
    
    HKEY hKey;
    DWORD dwDisp = 0;
    const char * keyAddr = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application";
    
    if(RegCreateKeyEx( HKEY_LOCAL_MACHINE, keyAddr, 0L, NULL, REG_OPTION_NON_VOLATILE, 
				KEY_ALL_ACCESS, NULL, &hKey, &dwDisp) != ERROR_SUCCESS)
    {
        printf("RegCreateKeyEx failed (%d)\n", GetLastError()); 
        return;
    }
    
    RegDeleteKey( hKey, SVCNAME );
    
    RegCloseKey(hKey);
}

void main(int argc, char *argv[]) 
{ 
    DoDeleteSvc();
}
