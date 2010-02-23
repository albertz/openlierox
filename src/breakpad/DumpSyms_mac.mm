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

// dump_syms_tool.m: Command line tool that uses the DumpSymbols class.
// TODO(waylonis): accept stdin

#include <unistd.h>
#include <mach-o/arch.h>

#include "common/mac/dump_syms.h"
#include "common/mac/macho_utilities.h"

bool DumpSyms(const std::string& bin, const std::string& symfile) {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSString *module_str = [[NSFileManager defaultManager]
							stringWithFileSystemRepresentation:bin.c_str()
							length:bin.length()];
	NSString* nssymfile = [NSString stringWithUTF8String:symfile.c_str()];
	DumpSymbols *dump = [[DumpSymbols alloc] initWithContentsOfFile:module_str];

	const NXArchInfo *localArchInfo = NXGetLocalArchInfo();
	if (localArchInfo) {
		NSString* arch;
		if (localArchInfo->cputype & CPU_ARCH_ABI64)
			arch = (localArchInfo->cputype == CPU_TYPE_POWERPC64) ? @"ppc64":
			@"x86_64";
		else
			arch = (localArchInfo->cputype == CPU_TYPE_POWERPC) ? @"ppc" :
			@"x86";
		
		if (![dump setArchitecture:arch]) {
			[dump release];
			return false;			
		}
	}
	
	if(![dump writeSymbolFile:nssymfile]) {
		[dump release];
		return false;
	}
	
    [dump release];
	[pool release];
	return true;
}
