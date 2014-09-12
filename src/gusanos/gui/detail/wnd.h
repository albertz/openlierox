#ifndef OMFG_GUI_WND_H
#define OMFG_GUI_WND_H

#include <string>
#include "util/rect.h"
#include "util/common.h"
#include "renderer.h"
#include "context.h"
#include "../../luaapi/types.h"

#include <iostream>
#include <list>
#include <map>
using std::cerr;
using std::endl;

namespace OmfgGUI
{

	class Wnd
	{
		public:
			friend class Context;

			static LuaReference metaTable;

			enum Dir
			{
			    Up = 0,
			    Right,
			    Down,
			    Left
		};

			enum LuaCallbacks
			{
			    OnAction = 0,
			    OnKeyDown,
			    OnActivate,
			    LuaCallbacksMax,
		};

			Wnd(Wnd* parent, std::map<std::string, std::string> const& attributes,
			    std::string const& tagLabel = "window")
					: m_focusable(true),
					m_parent(0),
					m_lastChildFocus(0),
					m_context(0),
					m_font(0),
					m_tagLabel(tagLabel),
					m_attributes(attributes),
					m_visible(true),
					m_active(false)
			{
				getAttrib("label", m_text);
				getAttrib("class", m_id);
				getAttrib("id", m_id);
				getAttrib("group", m_group);
				std::string v;
				if(getAttrib("selectable", v))
					m_focusable = (v != "0");

				if(parent)
					parent->addChild(this);
			}

			virtual ~Wnd();

			void* operator new(size_t count);

			void operator delete(void* block)
			{
				// Lua frees the memory
			}

			void operator delete(void* block, void *)
			{
				// Lua frees the memory
			}

			void* operator new(size_t count, void* space)
			{
				return space;
			}

			virtual void setActivation(bool active);

			void doSetActivation(bool active);

			void toggleActivation()
			{
				doSetActivation(!m_active);
			}

			bool doAction();

			virtual void setText(std::string const& aStr);

			std::string const& getText() const
			{
				return m_text;
			}

			Rect const& getRect()
			{
				return m_rect;
			}

			Wnd* getParent()
			{
				return m_parent;
			}

			std::string const& getID()
			{
				return m_id;
			}

			bool getAttrib(std::string const& name, std::string& dest);

			void getCoord(int& dx, int& dy, int x, int y);
			void getCoordX(int& dx, int x);
			void getCoordY(int& dy, int y);

			void updatePlacement();

			void allocateSpace(int& x, int& y, int width, int height);

			Wnd* findClosestChild(Wnd* org, Dir direction);

			void doUpdateGSS();

			virtual int classID();

			void pushReference();

			virtual bool registerCallback(std::string const& name, LuaReference callb);

			void setGroup(std::string newGroup);

			void addChild(Wnd* ch)
			{
				if(!ch->m_parent) {
					ch->m_parent = this;
					m_children.push_back(ch);
					m_namedChildren[ch->m_id] = ch;

					if(!ch->m_context && m_context) {
						ch->setContext_(m_context);
					}

					if(!m_group.empty() && ch->m_group.empty()) {
						ch->setGroup(m_group);
					}
				}
			}

			void removeChild(Wnd* ch)
			{
				for(Wnd* p = this; p && p->m_lastChildFocus == ch; p = p->m_parent) {
					p->m_lastChildFocus = 0;
				}

				m_children.remove(ch);
				m_namedChildren.erase(ch->m_id);
			}

			Wnd* getChildByName(std::string const& name)
			{
				std::map<std::string, Wnd*>::const_iterator i = m_namedChildren.find(name);
				if(i != m_namedChildren.end())
					return i->second;
				return 0;
			}

			void setVisibility(bool v)
			{
				m_visible = v;
			}

			bool isVisibile();

			bool isActive()
			{
				return m_active;
			}

			bool isFocused()
			{
				return m_context && m_context->getFocus() == this;
			}

			bool switchTo();

			void focus()
			{
				if(m_context)
					m_context->setFocus(this);
			}

			void setSubFocus(Wnd* p)
			{
				m_lastChildFocus = p;
			}

			Context* context()
			{
				return m_context;
			}

			bool m_focusable;
			LuaReference luaReference;

		protected:

			void setContext_(Context* context);

			bool readSpriteSet(BaseSpriteSet*& dest, std::string const& str);

			bool readSkin(BaseSpriteSet*& dest, std::string const& str);

			LuaReference m_callbacks[LuaCallbacksMax];

			std::string          m_text;
			Rect                 m_rect;
			std::list<Wnd *>     m_children;
			Wnd                 *m_parent;
			Wnd                 *m_lastChildFocus;
			Context             *m_context;
			BaseFont            *m_font;

			std::string          m_tagLabel;
			std::string          m_className;
			std::string          m_id;
			std::string          m_state;
			std::string          m_group;

			std::map<std::string, std::string> m_attributes;

			std::map<std::string, Wnd*> m_namedChildren;

			bool                 m_visible;
			bool                 m_active;

			Rect                 m_freeRect;
			int                  m_freeNextX;
			int                  m_freeNextY;

			// Formatting
			struct Formatting
			{
				enum Flags
				{
				    HasLeft   = 1 << 0,
				    HasTop    = 1 << 1,
				    HasRight  = 1 << 2,
				    HasBottom = 1 << 3,
				};

				Formatting()
						: width(50), height(50), spacing(5), padding(5), flags(0)
						, rect(10, 10, 0, 0)
				{}

				int         width;
				int         height;

				int         spacing;
				int         padding;
				long        flags;

				Rect        rect;

			}
			m_formatting;
	};

}

#endif //OMFG_GUI_WND_H
