#ifndef LIERO_CONSOLE_HPP
#define LIERO_CONSOLE_HPP

#include <string>

namespace Console
{
void init();
void test();
void waitForAnyKey();
void setAttributes(int attr);
int  getAttributes();
void write(std::string const& str);
void writeLine(std::string const& str);
void writeTextBar(std::string const& str, int barFormat);
void clear();

void writeWarning(std::string const& str);

struct LocalAttributes
{
	LocalAttributes(int attr)
	: oldAttributes(getAttributes())
	{
		setAttributes(attr);
	}
	
	~LocalAttributes()
	{
		setAttributes(oldAttributes);
	}
	
	int oldAttributes;
};

}

#endif // LIERO_CONSOLE_HPP
