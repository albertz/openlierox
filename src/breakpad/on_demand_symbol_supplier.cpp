// Copyright (c) 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <sys/stat.h>
#include <map>
#include <string>
#include <iostream>
#include <fstream>

#include "google_breakpad/processor/basic_source_line_resolver.h"
#include "google_breakpad/processor/minidump.h"
#include "google_breakpad/processor/system_info.h"
#include "processor/pathname_stripper.h"

#include "on_demand_symbol_supplier.h"
#include "DumpSyms.h"

using std::map;
using std::string;

using google_breakpad::OnDemandSymbolSupplier;
using google_breakpad::PathnameStripper;
using google_breakpad::SymbolSupplier;
using google_breakpad::SystemInfo;

OnDemandSymbolSupplier::OnDemandSymbolSupplier(const string &search_dir, 
                                               const string &symbol_search_dir)
: search_dir_(search_dir) {
	/* NSFileManager *mgr = [NSFileManager defaultManager];
	 int length = symbol_search_dir.length();
	 if (length) {
	 // Load all sym files in symbol_search_dir into our module_file_map
	 // A symbol file always starts with a line like this:
	 // MODULE mac x86 BBF0A8F9BEADDD2048E6464001CA193F0 GoogleDesktopDaemon
	 // or
	 // MODULE mac ppc BBF0A8F9BEADDD2048E6464001CA193F0 GoogleDesktopDaemon
	 const char *symbolSearchStr = symbol_search_dir.c_str();
	 NSString *symbolSearchPath = 
	 [mgr stringWithFileSystemRepresentation:symbolSearchStr 
	 length:strlen(symbolSearchStr)];
	 NSDirectoryEnumerator *dirEnum = [mgr enumeratorAtPath:symbolSearchPath];
	 NSString *fileName;
	 NSCharacterSet *hexSet = 
	 [NSCharacterSet characterSetWithCharactersInString:@"0123456789ABCDEF"];
	 NSCharacterSet *newlineSet = 
	 [NSCharacterSet characterSetWithCharactersInString:@"\r\n"];
	 while ((fileName = [dirEnum nextObject])) {
	 // Check to see what type of file we have
	 NSDictionary *attrib = [dirEnum fileAttributes];
	 NSString *fileType = [attrib objectForKey:NSFileType];
	 if ([fileType isEqualToString:NSFileTypeDirectory]) {
	 // Skip subdirectories
	 [dirEnum skipDescendents];
	 } else {
	 NSString *filePath = [symbolSearchPath stringByAppendingPathComponent:fileName];
	 NSString *dataStr = [[[NSString alloc] initWithContentsOfFile:filePath] autorelease];
	 if (dataStr) {
	 // Check file to see if it is of appropriate type, and grab module
	 // name.
	 NSScanner *scanner = [NSScanner scannerWithString:dataStr];
	 BOOL goodScan = [scanner scanString:@"MODULE mac " intoString:nil];
	 if (goodScan) {
	 goodScan = ([scanner scanString:@"x86 " intoString:nil] ||
	 [scanner scanString:@"ppc " intoString:nil]);
	 if (goodScan) {
	 NSString *moduleID;
	 goodScan = [scanner scanCharactersFromSet:hexSet 
	 intoString:&moduleID];
	 if (goodScan) {
	 // Module IDs are always 33 chars long
	 goodScan = [moduleID length] == 33;
	 if (goodScan) {
	 NSString *moduleName;
	 goodScan = [scanner scanUpToCharactersFromSet:newlineSet 
	 intoString:&moduleName];
	 if (goodScan) {
	 goodScan = [moduleName length] > 0;
	 if (goodScan) {
	 const char *moduleNameStr = [moduleName UTF8String];
	 const char *filePathStr = [filePath fileSystemRepresentation];
	 // Map our file
	 module_file_map_[moduleNameStr] = filePathStr;
	 }
	 }
	 }
	 }
	 }
	 }
	 }
	 }
	 }
	 }*/
}

SymbolSupplier::SymbolResult
OnDemandSymbolSupplier::GetSymbolFile(const CodeModule *module,
                                      const SystemInfo *system_info,
                                      string *symbol_file) {
	string path(GetModuleSymbolFile(module));
	
	if (path.empty()) {
		if (!GenerateSymbolFile(module, system_info))
			return NOT_FOUND;
		
		path = GetModuleSymbolFile(module);
	}
	
	if (path.empty())
		return NOT_FOUND;
	
	*symbol_file = path;
	return FOUND;
}

SymbolSupplier::SymbolResult
OnDemandSymbolSupplier::GetSymbolFile(const CodeModule *module,
                                      const SystemInfo *system_info,
                                      string *symbol_file,
                                      string *symbol_data) {
	SymbolSupplier::SymbolResult s = GetSymbolFile(module,
												   system_info,
												   symbol_file);
	
	
	if (s == FOUND) {
		std::ifstream in(symbol_file->c_str());
		getline(in, *symbol_data, std::string::traits_type::to_char_type(
																		 std::string::traits_type::eof()));
		in.close();
	}
	
	return s;
}

string OnDemandSymbolSupplier::GetLocalModulePath(const CodeModule *module) {
	return module->code_file();
}

string OnDemandSymbolSupplier::GetModulePath(const CodeModule *module) {
	return module->code_file();
}

string OnDemandSymbolSupplier::GetNameForModule(const CodeModule *module) {
	return PathnameStripper::File(module->code_file());
}

string OnDemandSymbolSupplier::GetModuleSymbolFile(const CodeModule *module) {
	string name(GetNameForModule(module));
	map<string, string>::iterator result = module_file_map_.find(name);
	
	return (result == module_file_map_.end()) ? "" : (*result).second;
}

bool OnDemandSymbolSupplier::GenerateSymbolFile(const CodeModule *module,
                                                const SystemInfo *system_info) {
	bool result = true;
	string name = GetNameForModule(module);
	string module_path = GetLocalModulePath(module);
	string symbol_path = "/tmp/" + name + "." + system_info->cpu + ".sym";
	
	if (module_path.empty())
		return false;
	
	result = DumpSyms(module_path, symbol_path);
	
	// Add the mapping
	if (result)
		module_file_map_[name] = symbol_path;
	
	return result;
}
