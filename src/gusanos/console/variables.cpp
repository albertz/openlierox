#include "variables.h"
#include "util/text.h"
#include "util/macros.h"
#include "util/stringbuild.h"
#include "console.h"

///////////////////////////////EnumVariable////////////////////////////////

//============================= LIFECYCLE ================================

EnumVariable::EnumVariable()
: m_src(0), m_defaultValue(0)
{

}

EnumVariable::~EnumVariable()
{
	
}

EnumVariable::EnumVariable(std::string name, int* src, int defaultValue, MapType const& mapping, CallbackT const& func)
: Variable(name), m_src(src), m_defaultValue(defaultValue), m_mapping(mapping), m_callback(func)
{
	*m_src = m_defaultValue;

	foreach(i, m_mapping)
	{
		m_reverseMapping[i->second] = i->first;
	}
}

//============================= INTERFACE ================================

std::string EnumVariable::invoke(const std::list<std::string> &args)
{
	if (!args.empty())
	{
		MapType::iterator v = m_mapping.find(*args.begin());
		if(v == m_mapping.end())
		{
			std::string help = "INVALID VALUE, POSSIBLE VALUES: ";
			
			foreach(i, m_mapping)
			{
				help += i->first + ' ';
			}
			
			return help;
		}
		
		int oldValue = *m_src;
		*m_src = v->second;
		if ( m_callback ) m_callback(oldValue);
		
		return std::string();
	}
	else
	{
		ReverseMapType::iterator v = m_reverseMapping.find(*m_src);
		
		if(v == m_reverseMapping.end())
			return S_(m_name) << " HAS ILLEGAL NUMERIC VALUE " << *m_src;
		
		//return m_name + " IS \"" + v->second + '"';
		return v->second;
	}
}

struct ItemGetText
{
	template<class IteratorT>
	std::string const& operator()(IteratorT i) const
	{
		return i->first;
	}
};

std::string EnumVariable::completeArgument(int idx, std::string const& beginning)
{
	if(idx != 0)
		return beginning;
		
	return shellComplete(
		m_mapping,
		beginning.begin(),
		beginning.end(),
		ItemGetText(),
		ConsoleAddLines(*m_owner)
	);
}
