/*
 *  TeeStdoutHandler.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 01.12.09.
 *  code under LGPL
 *
 */

#ifndef __TEESTDOUTHANDLER_H__
#define __TEESTDOUTHANDLER_H__

#include <string>

void teeStdoutInit();
void teeStdoutFile(const std::string& absfilename);
void teeStdoutQuit(bool wait = true);

const char* GetLogFilename();

#endif

