/*
 *  DumpSyms.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 18.10.09.
 *  Code under LGPL.
 *
 */

#include "DumpSyms.h"

#ifdef NBREAKPAD
#include "Debug.h"
bool DumpSyms(const std::string& bin, const std::string& symfile) {
	errors << "DumpSyms: breakpad support not available in this build" << endl;
	return false;
}
#else // Breakpad support

#if defined(WIN32)

#include <stdio.h>
#include <string>

#include "BreakPad.h"
#include "FindFile.h"
#include "common/windows/pdb_source_line_writer.h"

using google_breakpad::PDBSourceLineWriter;

using std::wstring;

bool DumpSyms(const std::string& bin, const std::string& symfile, const std::string& arch) {
	FILE* out = fopen(Utf8ToSystemNative(symfile).c_str(), "wb");
	if(!out) return false;
	
	PDBSourceLineWriter writer;
	if (!writer.Open(Utf8ToUtf16(bin), PDBSourceLineWriter::ANY_FILE)) {
		fclose(out);
		return false;
	}
	
	if (!writer.WriteMap(out)) {
		fclose(out);
		return false;
	}
	
	writer.Close();
	fclose(out);
	return true;
}

// not Windows -> all other cases (Unix/Linux), except Mac (which has its own implementation in DumpSyms_mac.mm)
#elif !defined(__APPLE__)

#include <string>
#include <cstdio>

#include "common/linux/dump_symbols.h"

using namespace google_breakpad;

bool DumpSyms(const std::string& bin, const std::string& symfile, const std::string& arch) {
	FILE* out = fopen(symfile.c_str(), "wb");
	if(!out) return false;
	
	DumpSymbols dumper;
	bool res = dumper.WriteSymbolFile(bin, out);

	if(!res) {
		fseek(out, 0, SEEK_SET);
		// Some systems split the debug data. This is common for Gentoo.
		res = dumper.WriteSymbolFile("/usr/lib/debug/" + bin + ".debug", out);
	}

	fclose(out);
	return res;
}

#endif // Win, Linux/Unix
#endif // Breakpad
