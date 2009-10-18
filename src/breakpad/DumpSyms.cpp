/*
 *  DumpSyms.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 18.10.09.
 *  Code under LGPL.
 *
 */

#include "DumpSyms.h"

#ifdef WIN32

#include <stdio.h>
#include <string>

#include "FindFile.h"
#include "common/windows/pdb_source_line_writer.h"

using std::wstring;
using google_breakpad::PDBSourceLineWriter;

bool DumpSyms(const std::string& bin, const std::string& symfile) {
	FILE* out = fopen(Utf8ToSystemNative(symfile).c_str(), "wb");
	if(!out) return false;
	
	PDBSourceLineWriter writer;
	if (!writer.Open(Utf8ToSystemNative(bin), PDBSourceLineWriter::ANY_FILE)) {
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

#else

#include <string>
#include <cstdio>

#include "common/linux/dump_symbols.h"

using namespace google_breakpad;

bool DumpSyms(const std::string& bin, const std::string& symfile) {
	FILE* out = fopen(symfile.c_str(), "wb");
	if(!out) return false;
	
	DumpSymbols dumper;
	bool res = dumper.WriteSymbolFile(binary, out));
	
	fclose(out);
	return res;
}

#endif
