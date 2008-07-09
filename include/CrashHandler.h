/*
	OpenLieroX CrashHandler

	registers a crash handler in the OS and handles the crashes

	code under LGPL
	created 09-07-2008 by Albert Zeyer
*/

#ifndef __CRASHHANDLER_H__
#define __CRASHHANDLER_H__

class CrashHandler {
public:
	static void init();
	static void uninit();
	static CrashHandler* get();
};

#endif
