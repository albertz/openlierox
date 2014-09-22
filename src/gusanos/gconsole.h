#ifndef VERMES_GCONSOLE_H
#define VERMES_GCONSOLE_H

#include "console/console.h"
#include <string>

class GConsole : public Console
{
public:
	void init();
	int executeConfig(const std::string &filename);
};

extern GConsole console;

#endif  // VERMES_GCONSOLE_H
