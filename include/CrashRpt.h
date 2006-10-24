///////////////////////////////////////////////////////////////////////////////
//
//  Module: CrashRpt.h
//
//    Desc: Defines the interface for the CrashRpt.DLL.
//
// Copyright (c) 2003 Michael Carruth
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _CRASHRPT_H_
#define _CRASHRPT_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <windows.h>

// CrashRpt.h
#ifdef CRASHRPTAPI

// CRASHRPTAPI should be defined in all of the DLL's source files as
// #define CRASHRPTAPI extern "C" __declspec(dllexport)

#else

// This header file is included by an EXE - export
#define CRASHRPTAPI extern "C" __declspec(dllimport)

#endif

// Client crash callback
typedef BOOL (CALLBACK *LPGETLOGFILE) (LPVOID lpvState);

//-----------------------------------------------------------------------------
// Install
//    Initializes the library and optionally set the client crash callback and
//    sets up the email details.
//
// Parameters
//    pfn         Client crash callback
//    lpTo        Email address to send crash report
//    lpSubject   Subject line to be used with email
//
// Return Values
//    If the function succeeds, the return value is a pointer to the underlying
//    crash object created.  This state information is required as the first
//    parameter to all other crash report functions.
//
// Remarks
//    Passing NULL for lpTo will disable the email feature and cause the crash 
//    report to be saved to disk.
//
CRASHRPTAPI 
LPVOID 
Install(
   IN LPGETLOGFILE pfn OPTIONAL,                // client crash callback
   IN LPCTSTR lpTo OPTIONAL,                    // Email:to
   IN LPCTSTR lpSubject OPTIONAL                // Email:subject
   );

//-----------------------------------------------------------------------------
// Uninstall
//    Uninstalls the unhandled exception filter set up in Install().
//
// Parameters
//    lpState     State information returned from Install()
//
// Return Values
//    void
//
// Remarks
//    This call is optional.  The crash report library will automatically 
//    deinitialize when the library is unloaded.  Call this function to
//    unhook the exception filter manually.
//
CRASHRPTAPI 
void 
Uninstall(
   IN LPVOID lpState                            // State from Install()
   );

//-----------------------------------------------------------------------------
// AddFile
//    Adds a file to the crash report.
//
// Parameters
//    lpState     State information returned from Install()
//    lpFile      Fully qualified file name
//    lpDesc      Description of file, used by details dialog
//
// Return Values
//    void
//
// Remarks
//    This function can be called anytime after Install() to add one or more
//    files to the generated crash report.
//
CRASHRPTAPI 
void 
AddFile(
   IN LPVOID lpState,                           // State from Install()
   IN LPCTSTR lpFile,                           // File name
   IN LPCTSTR lpDesc                            // File desc
   );

//-----------------------------------------------------------------------------
// GenerateErrorReport
//    Generates the crash report.
//
// Parameters
//    lpState     State information returned from Install()
//    pExInfo     Pointer to an EXCEPTION_POINTERS structure
//
// Return Values
//    void
//
// Remarks
//    Call this function to manually generate a crash report.
//
CRASHRPTAPI 
void 
GenerateErrorReport(
   IN LPVOID lpState,
   IN PEXCEPTION_POINTERS pExInfo OPTIONAL
   );

#endif
