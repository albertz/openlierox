#include "xml-grammar.h"
#include "context.h"
#include "wnd.h"
#include "list.h"
#include "button.h"
#include "edit.h"
#include "group.h"
#include "check.h"
#include <sstream>
#include <iostream>
#include <utility>
#include "../../luaapi/types.h"
#include "../../luaapi/context.h"

using namespace std;

namespace OmfgGUI
{

struct XMLHandler
{
	struct Tag
	{
		Tag(std::string const& label_)
		: label(label_)
		{
		}
		
		std::string label;
		std::map<std::string, std::string> attributes;
	};
	
	struct WndInfo
	{
		WndInfo(Wnd* wnd_)
		: wnd(wnd_)
		{

		}
		
		Wnd* wnd;
	};
	
	XMLHandler(Context& context_, Wnd* dest_)
	: tag(""), context(context_), firstWindow(0)
	{
		windows.push(WndInfo(dest_));
	}
	
	void error(std::string err)
	{
		cerr << err << endl;
	}
	
	void beginTag(std::string const& label)
	{
		tag = Tag(label);
	}
	
	void beginAttributes()
	{
	}
	
	void attribute(std::string const& name, std::string const& value)
	{
		tag.attributes[name] = value;
	}
	
	std::string const& getAttrib(std::string const& name, std::string const& def)
	{
		std::map<std::string, std::string>::iterator i = tag.attributes.find(name);
		if(i == tag.attributes.end())
			return def;
		return i->second;
	}

	void endAttributes()
	{
		Wnd* newWindow = 0;
					
		if(tag.label == "window")
		{
			newWindow = lua_new(Wnd, (windows.top().wnd, tag.attributes), luaIngame);
		}
		else if(tag.label == "list")
		{
			newWindow = lua_new(List, (windows.top().wnd, tag.attributes), luaIngame);
		}
		else if(tag.label == "button")
		{
			newWindow = lua_new(Button, (windows.top().wnd, tag.attributes), luaIngame);
		}
		else if(tag.label == "group")
		{
			newWindow = lua_new(Group, (windows.top().wnd, tag.attributes), luaIngame);
		}
		else if(tag.label == "edit")
		{
			newWindow = lua_new(Edit, (windows.top().wnd, tag.attributes), luaIngame);
		}
		else if(tag.label == "check")
		{
			newWindow = lua_new(Check, (windows.top().wnd, tag.attributes), luaIngame);
		}
		
		if(!windows.top().wnd)
		{
			// Set as root
			context.setRoot(newWindow);
		}
		
		if(newWindow)
		{
			if(!firstWindow)
				firstWindow = newWindow;
			windows.push(WndInfo(newWindow)); // Done last
		}
	}
	
	void endTag(std::string const& label)
	{
		windows.pop();
	}
	
	Tag tag;
	
	std::stack<WndInfo> windows;
	Context& context;
	Wnd* firstWindow;
};

Wnd* Context::buildFromXML(std::istream& s, Wnd* dest)
{
	if(dest && dest->m_context != this)
	{
		return 0; // The destination window belongs to a different context
	}

	XMLHandler handler(*this, dest);
	xmlDocument(s, handler);
	return handler.firstWindow;
}

}
