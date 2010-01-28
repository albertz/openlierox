#ifndef OMFG_GUI_CONTEXT_H
#define OMFG_GUI_CONTEXT_H

#include "util/rect.h"
#include "util/common.h"
#include "renderer.h"
//#include "formatting.h"
#include <iostream>
#include <map>
#include <list>
#include <string>
#include <vector>
#include <stack>

class LuaContext;

namespace OmfgGUI
{

class Wnd;

typedef int KeyType;

class Context
{
public:
	friend struct GSSHandler;
	friend class Wnd; //TEMP
	
	enum
	{
		Unknown = 0,
		Button,
		Edit,
		List,
		Group,
		WndCount
	};
	
	struct MouseKey
	{
		enum type
		{
			Left = 0,
			Right,
			Middle
		};
	};

	//typedef std::vector<Formatting::Change> GSSselector;
	//typedef std::vector<Formatting::BaseChange*> GSSselector;
	/*struct GSSselector
	{
		typedef std::map<std::string, std::vector<std::string> > PropertyMap;
		PropertyMap main;
		std::map<std::string, PropertyMap> states;
	};*/

	//typedef std::map<std::string, std::vector<std::string> > GSSpropertyMap;
	typedef std::list<std::pair<std::string, std::list<std::string> > > GSSpropertyMap;
	
/*
	static GSSpropertyMap GSSpropertyMapStandard;

	struct GSSstate
	{
		GSSpropertyMap& operator() ()
		{
			return m_anyState;
		}
		
		GSSpropertyMap& operator() (std::string const& state_)
		{
			if(state_.size() == 0)
				return m_anyState;
			std::map<std::string, GSSpropertyMap>::iterator i = m_state.find(state_);
			if(i != m_state.end())
				return i->second;
			return GSSpropertyMapStandard;
		}
		
		GSSpropertyMap& insert(std::string const& state_)
		{
			if(state_.size() == 0)
				return m_anyState;
			return m_state[state_];
		}
		
		GSSpropertyMap const& operator() () const
		{
			return m_anyState;
		}
		
		GSSpropertyMap const& operator() (std::string const& state_) const
		{
			if(state_.size() == 0)
				return m_anyState;
			std::map<std::string, GSSpropertyMap>::const_iterator i = m_state.find(state_);
			if(i != m_state.end())
				return i->second;
			return GSSpropertyMapStandard;
		}
		
		static GSSstate standard;
		
	private:
		GSSpropertyMap m_anyState;
		std::map<std::string, GSSpropertyMap> m_state;
	};
	
	struct GSSid
	{
		GSSstate& operator() ()
		{
			return m_anyID;
		}
		
		GSSstate& operator() (std::string const& id_)
		{
			if(id_.size() == 0)
				return m_anyID;
			std::map<std::string, GSSstate>::iterator i = m_id.find(id_);
			if(i != m_id.end())
				return i->second;
			return GSSstate::standard;
		}
		
		GSSstate& insert(std::string const& id_)
		{
			if(id_.size() == 0)
				return m_anyID;
			return m_id[id_];
		}
		
		GSSstate const& operator() () const
		{
			return m_anyID;
		}
		
		GSSstate const& operator() (std::string const& id_) const
		{
			if(id_.size() == 0)
				return m_anyID;
			std::map<std::string, GSSstate>::const_iterator i = m_id.find(id_);
			if(i != m_id.end())
				return i->second;
			return GSSstate::standard;
		}
		
		static GSSid standard;
		
	private:
		GSSstate m_anyID;
		std::map<std::string, GSSstate> m_id;
	};
	
	struct GSSclass
	{
		GSSid& operator() ()
		{
			return m_anyClass;
		}
		
		GSSid& operator() (std::string const& class_)
		{
			if(class_.size() == 0)
				return m_anyClass;
			std::map<std::string, GSSid>::iterator i = m_class.find(class_);
			if(i != m_class.end())
				return i->second;
			return GSSid::standard;
		}
		
		GSSid& insert(std::string const& class_)
		{
			if(class_.size() == 0)
				return m_anyClass;
			return m_class[class_];
		}
		
		GSSid const& operator() () const
		{
			return m_anyClass;
		}
		
		GSSid const& operator() (std::string const& class_) const
		{
			if(class_.size() == 0)
				return m_anyClass;
			std::map<std::string, GSSid>::const_iterator i = m_class.find(class_);
			if(i != m_class.end())
				return i->second;
			return GSSid::standard;
		}
		
		static GSSclass standard;

	private:
		GSSid m_anyClass;
		std::map<std::string, GSSid> m_class;
	};
	
	struct GSSselectorMap
	{
		GSSclass& operator() ()
		{
			return m_anyLabel;
		}
		
		GSSclass& operator() (std::string const& label_)
		{
			if(label_.size() == 0)
				return m_anyLabel;
			std::map<std::string, GSSclass>::iterator i = m_label.find(label_);
			if(i != m_label.end())
				return i->second;
			return GSSclass::standard;
		}

		GSSclass& insert(std::string const& label_)
		{
			if(label_.size() == 0)
				return m_anyLabel;
			return m_label[label_];
		}
		
		GSSclass const& operator() () const
		{
			return m_anyLabel;
		}
		
		GSSclass const& operator() (std::string const& label_) const
		{
			if(label_.size() == 0)
				return m_anyLabel;
			std::map<std::string, GSSclass>::const_iterator i = m_label.find(label_);
			if(i != m_label.end())
				return i->second;
			return GSSclass::standard;
		}
		
	private:
		GSSclass m_anyLabel;
		std::map<std::string, GSSclass> m_label;
	};
*/
	struct GSSselector
	{
		struct Condition
		{
			enum Type
			{
				Tag = 0,
				Class,
				ID,
				State,
				Group
			};
			
			Condition(Type type, std::string v)
			: type(type), v(v)
			{
			}
			
			Type type;
			std::string v;
		};
		
		void addTag(std::string const& name)
		{
			cond.push_back(Condition(Condition::Tag, name)); 
		}

		void addClass(std::string const& name)
		{
			cond.push_back(Condition(Condition::Class, name)); 
		}
		
		void addID(std::string const& name)
		{
			cond.push_back(Condition(Condition::ID, name)); 
		}
		
		void addState(std::string const& name)
		{
			cond.push_back(Condition(Condition::State, name)); 
		}
		
		void addGroup(std::string const& name)
		{
			cond.push_back(Condition(Condition::Group, name)); 
		}
		
		int matchesWindow(Wnd*) const;
		
		std::list<std::string>& addProperty(std::string const& name)
		{
			props.push_back(std::pair<std::string, std::list<std::string> >(name, std::list<std::string>()));
			return props.back().second;
		}
		
		std::list<Condition> cond;
		GSSpropertyMap props;
	};
	
	GSSselector& addSelector()
	{
		m_gss.push_back(GSSselector());
		return m_gss.back();
	}
	
	typedef std::list<GSSselector> GSSselectors;
	
	//typedef std::map<std::string, GSSpropertyMap> GSSselectorMap;
	
	Context(Renderer* renderer)
	: m_mouseCaptureWnd(0), m_rootWnd(0), m_keyboardFocusWnd(0)
	, m_mouseFocusWnd(0), m_renderer(renderer)
	{

	}
	
	virtual ~Context()
	{
	}
	
	void updateGSS();
	
	//Sends a cursor relocation event
	void mouseMove(ulong newX, ulong newY);
	
	//Sends a mouse button down event
	void mouseDown(ulong newX, ulong newY, MouseKey::type button);
	
	//Sends a mouse button up event
	void mouseUp(ulong newX, ulong newY, MouseKey::type button);
	
	//Sends a printable character
	void charPressed(char c);
	
	//Sends a keydown event
	void keyDown(KeyType k, bool aShift, bool aAlt, bool aCtrl);
	
	//Sends a keyup event
	void keyUp(KeyType k, bool aShift, bool aAlt, bool aCtrl);
	
	void captureMouse(Wnd* aWnd)
	{
		m_mouseCaptureWnd = aWnd;
	}
	
	Wnd* getMouseCapture()
	{
		return m_mouseCaptureWnd;
	}
	
	virtual void setFocus(Wnd* aWnd);
	
	// Called by a window if it has focus and
	// is being hidden.
	virtual void hiddenFocus() {}
	
	// Called by a window if it has focus and
	// is turned visible again.
	virtual void shownFocus() {}
	
/*
	virtual LuaContext& luaContext() = 0;
	*/
	virtual bool keyState(int key) = 0;
	
	Wnd* getFocus()
	{
		return m_keyboardFocusWnd;
	}
	
	void setActive(Wnd* wnd);
	
	void process();
	
	void render();
	
	void mouseDown(int x, int y, MouseKey::type button);
	void mouseUp(int x, int y, MouseKey::type button);
	void mouseMove(int x, int y);
	void mouseScroll(int x, int y, int offs);
	
	virtual void loadGSSFile(std::string const&, bool passive) = 0;
	virtual Wnd* loadXMLFile(std::string const&, Wnd* loadTo) = 0;
	
	// This is defined in gss.cpp
	void loadGSS(std::istream& s, std::string const& fileName);
	
	// This is defined in xml.cpp
	Wnd* buildFromXML(std::istream& s, Wnd* dest);
	
	//void testParseXML();
	
	template<class WndT>
	WndT* setRoot(WndT* wnd)
	{
		setRoot_(wnd);
			
		return wnd;
	}

	Wnd* getRoot()
	{
		return m_rootWnd;
	}

	void registerNamedWindow(std::string const& id, Wnd* wnd);
	
	/*
	void deregisterNamedWindow(std::string const& id)
	{
		if(id.size() == 0)
			return;
		m_namedWindows.erase(id);
	}*/

	void registerWindow(Wnd* wnd);
	
	void deregisterWindow(Wnd* wnd);

	Wnd* findNamedWindow(std::string const& id)
	{
		std::map<std::string, Wnd*>::iterator i = m_namedWindows.find(id);
		if(i == m_namedWindows.end())
			return 0;
		return i->second;
	}

	void destroy();
	
	Renderer* renderer()
	{ return m_renderer; }
	
	virtual BaseFont* loadFont(std::string const& name) = 0;
	virtual BaseSpriteSet* loadSpriteSet(std::string const& name) = 0;
	
	
//	BaseFont const& m_defaultFont; //TEMP
	
protected:
	void setRoot_(Wnd* wnd);
	
	Wnd* m_mouseCaptureWnd;
	Wnd* m_rootWnd;
	Wnd* m_keyboardFocusWnd;
	Wnd* m_activeWnd;
	Wnd* m_mouseFocusWnd;
	Renderer* m_renderer;

	//GSSselectorMap m_gss_;
	std::map<std::string, Wnd*> m_namedWindows;
	
	GSSselectors m_gss;

	long m_cursorX;
	long m_cursorY;
};

} //namespace OmfgGUI


#endif //OMFG_GUI_CONTEXT_H
