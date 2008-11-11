
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
    SERVICE_STATUS ssStatus; 

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
        DELETE);            // need delete access 
 
    if (schService == NULL)
    { 
        printf("OpenService failed (%d)\n", GetLastError()); 
        CloseServiceHandle(schSCManager);
        return;
    }

    // Delete the service.
 
    if (! DeleteService(schService) ) 
    {
        printf("DeleteService failed (%d)\n", GetLastError()); 
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
