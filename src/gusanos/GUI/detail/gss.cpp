#include "context.h"
#include <istream>
#include <string>
#include "util/log.h"

namespace OmfgGUI
{
	

}

#include "gss-grammar.h"

namespace OmfgGUI
{
	
struct GSSImpl : public TGrammar<GSSImpl>
{
	GSSImpl(Context::GSSselectors& selectors, std::istream& str_, std::string const& fileName_)
	: selectors(selectors), str(str_), fileName(fileName_)
	{
		this->next();
	}
	
	size_t read(char* p, size_t s)
	{
		str.read(p, s);
		return str.gcount();
	}
	
	void reportError(std::string const& error, Location loc)
	{
		loc.print(error);
	}
	
	Location getLoc()
	{
		return Location(fileName, line);
	}
	
	Context::GSSselector& addSelector()
	{
		selectors.push_back(Context::GSSselector());
		return selectors.back();
	}
	
	Context::GSSselectors& selectors;
	std::istream& str;
	std::string fileName;
};

void Context::loadGSS(std::istream& s, std::string const& fileName)
{
	GSSImpl handler(m_gss, s, fileName);
	
	//std::cout << "Loading GSS " << fileName << std::endl;
	handler.rule_document();
	//std::cout << std::endl << "Done" << fileName << std::endl;
	//return handler.full();
}

}

	
