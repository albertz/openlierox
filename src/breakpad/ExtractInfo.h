/*
 *  ExtractInfo.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 18.10.09.
 *  Code under LGPL.
 *
 */

#ifndef __OLX_EXTRACTINFO_H__
#define __OLX_EXTRACTINFO_H__

#ifdef __cplusplus

#include <string>
#include <ostream>

// dumps info on stdout
void MinidumpExtractInfo(const std::string& minidumpfile, std::ostream& out, std::ostream& err);

extern "C" {
#endif

// Checks the parameter, if we should do the extractinfo.
// Returns true if we did that. We want to quit after that.
int DoCrashReport(int argc, char** argv);

#ifdef __cplusplus
}
#endif

#endif
