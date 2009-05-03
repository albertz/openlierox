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
#include "luaapi/types.h"
#include "luaapi/context.h"

using namespace std;

namespace OmfgGUI
{
	
/*
struct GSSHandler
{
	GSSHandler(Context::GSSselectorMap& style_)
	: style(style_)
	{
		
	}
	
	void error(std::string err)
	{
		cout << err << endl;
	}
	
	void selector(std::string const& tagLabel, std::string const& className, std::string const& id, std::string const& state
	, std::string const& property, std::vector<std::string> const& value)
	{
		Context::GSSpropertyMap& dest = style.insert(tagLabel).insert(className).insert(id).insert(state);

		dest.push_back(std::make_pair(property, value));
	}
	
	Context::GSSselectorMap& style;
};

void Context::loadGSS(std::istream& s)
{
	GSSHandler handler(m_gss);
	gssSheet(s, handler);
}*/

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
		/*
		std::string className = getAttrib("class", "");
		std::string label = getAttrib("label", "");
		std::string id = getAttrib("id", "");
		bool focusable = getAttrib("selectable", "1") != "0";
		*/
		Wnd* newWindow = 0;
					
		if(tag.label == "window")
		{
			newWindow = lua_new(Wnd, (windows.top().wnd, tag.attributes), lua);
		}
		else if(tag.label == "list")
		{
			newWindow = lua_new(List, (windows.top().wnd, /*tag.label, className, id,*/ tag.attributes/*, label*/), lua);
		}
		else if(tag.label == "button")
		{
			newWindow = lua_new(Button, (windows.top().wnd, /*tag.label, className, id,*/ tag.attributes/*, label*/), lua);
		}
		else if(tag.label == "group")
		{
			newWindow = lua_new(Group, (windows.top().wnd, /*tag.label, className, id,*/ tag.attributes/*, label*/), lua);
		}
		else if(tag.label == "edit")
		{
			newWindow = lua_new(Edit, (windows.top().wnd, /*tag.label, className, id,*/ tag.attributes/*, label*/), lua);
		}
		else if(tag.label == "check")
		{
			newWindow = lua_new(Check, (windows.top().wnd, /*tag.label, className, id,*/ tag.attributes/*, label*/), lua);
		}
		//newWindow->m_focusable = focusable;
		
		if(!windows.top().wnd)
		{
			// Set as root
			context.setRoot(newWindow);
		}
		
		if(newWindow)
		{
			if(!firstWindow)
				firstWindow = newWindow;
			/*
			newWindow->applyGSS(style);
			newWindow->updatePlacement();
*/
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

/*
void Context::testParseXML()
{
	istringstream rootGSS(
		"#root { background: #000080 ; left: 0 ; top: 0 ; bottom : -1 ; right: -1; padding: 29; spacing: 20 }");
		
	istringstream rootXML("<window id=\"root\" />");
	
	loadGSS(rootGSS, "root");
	buildFromXML(rootXML, 0);
	
	// ========================== test XML and GSS ====================
	
	istringstream gss(
		"button { background: #00AF00 ; "
		" border: #AFFFAF; border-right: #00A000; border-bottom: #00A000 }"
		"button:focused { background: #AF0000 ; border-right: #A00000; border-bottom: #A00000; }"
		"#options { width: 100 ; height: 150 ; bottom: -10 ; right: -10 }");
		
	
	istringstream xml(
		"<button id=\"f\" label=\"Fullscreen\" command=\"vid_fullscreen 1\" />"
		"<button id=\"w\" label=\"Windowed\" command=\"vid_fullscreen 0\" />"
		"<button id=\"o\" label=\"Options\" command=\"gui_loadgss OPTIONS.GSS passive ; gui_loadxml options.xml ; gui_focus options\" />");
	
	// ================================================================
	
	loadGSS(gss, "default");
	
	buildFromXML(xml, getRoot());
	
	setFocus(findNamedWindow("f"));
}
*/
}
