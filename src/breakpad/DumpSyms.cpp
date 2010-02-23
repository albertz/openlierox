/*
 *  DumpSyms.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 18.10.09.
 *  Code under LGPL.
 *
 */

#include "DumpSyms.h"

#if defined(WIN32)

#include <stdio.h>
#include <string>

#include "BreakPad.h"
#include "FindFile.h"
#ifndef NBREAKPAD
#include "common/windows/pdb_source_line_writer.h"

using google_breakpad::PDBSourceLineWriter;
#endif

using std::wstring;

bool DumpSyms(const std::string& bin, const std::string& symfile) {
#ifndef NBREAKPAD
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
#endif
	return true;
}

// not Windows -> all other cases (Unix/Linux), except Mac (which has its own implementation in DumpSyms_mac.mm)
#elif !defined(__APPLE__)

#include <string>
#include <cstdio>

#include "common/linux/dump_symbols.h"

using namespace google_breakpad;

bool DumpSyms(const std::string& bin, const std::string& symfile) {
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
