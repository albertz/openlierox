//
//  Result.h
//  OpenLieroX
//
//  Created by Albert Zeyer on 02.01.12.
//  Copyright (c) 2012 Albert Zeyer. All rights reserved.
//

#ifndef OpenLieroX_Result_h
#define OpenLieroX_Result_h

#include <string>

struct Result {
	bool success;
	std::string humanErrorMsg;
	Result(bool _s) : success(_s) {}
	Result(const std::string& _err) : success(false), humanErrorMsg(_err) {}
	Result(const char* _err) : success(false), humanErrorMsg(_err) {}
	operator bool() const { return success; }
};

// Negative result is just a simple wrapper to evalute to true if there is an error.
// E.g. you can use it like if(NegResult r = doSomething()) { ... }
struct NegResult {
	Result res;
	NegResult(const Result& _res) : res(_res) {}
	operator bool() const { return !res.success; }	
};

#endif
