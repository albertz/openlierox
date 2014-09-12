#ifndef OMFG_GUI_LIST_H
#define OMFG_GUI_LIST_H

#include "wnd.h"
#include "llist.h"

#include <string>
#include <boost/lexical_cast.hpp>

namespace OmfgGUI
{

	using boost::lexical_cast;

	class List;


	struct ListNode : public LNodeImp<ListNode>
	{
		static LuaReference metaTable;

		typedef LList<ListNode> list_t;
		typedef list_t::iterator node_iter_t;
		typedef list_t::reference node_ref_t;

		friend class List;

		ListNode(std::string const& text)
				: selected(false), expanded(true)
				, parent(0), level(0)
				, list(0)
		{
			columns.push_back(text);
		}

		virtual ~ListNode()
		{}

		node_iter_t push_back(ListNode* node)
		{
			children.insert(node);

			node->level = level + 1;
			node->parent = this;

			return node_iter_t(node);
		}

		void resizeColumns(size_t s);

		static node_iter_t getPrevVisible(node_iter_t i);
		static node_iter_t getNextVisible(node_iter_t i);
		static int findOffsetTo(node_iter_t i, node_iter_t to);
		static node_iter_t findRelative(node_iter_t i, int aIdx);

		void* operator new(size_t count);

		void operator delete(void* block)
		{
			// Lua frees the memory
		}

		void* operator new(size_t count, void* space)
		{
			return space;
		}

		void operator delete(void*, void*)
		{
			// Lua frees the memory
		}

		void setText(unsigned int column, std::string const& text)
		{
			if(column < columns.size())
				columns[column] = text;
		}

		std::string getText(unsigned int column)
		{
			if(column < columns.size())
				return columns[column];
			else
				return "";
		}

		std::vector<std::string> const& getFields()
		{
			return columns;
		}

		void clearSelections();

		std::vector<std::string> columns;
		bool        selected;
		bool        expanded;
		ListNode*   parent;
		long        level;
		List*       list;
		list_t      children;
		LuaReference luaReference;
		LuaReference luaData;
	};

	class List : public Wnd
	{
		public:
			static LuaReference metaTable;

			typedef LList<ListNode> list_t;
			typedef list_t::iterator node_iter_t;
			typedef list_t::reference node_ref_t;
			static const long rowHeight = 12;

			struct ColumnHeader
			{
				ColumnHeader(std::string const& name_, double widthFactor_)
						: name(name_), widthFactor(widthFactor_)
				{}

				std::string name;
				double      widthFactor;
			};

			friend struct ListNode;

			List(Wnd* parent, std::map<std::string, std::string> const& attributes)
					: 
					Wnd(parent, attributes, "list"),
					m_RootNode("root")
			{
				assert(!m_RootNode.parent);
				m_RootNode.list = this;
				m_RootNode.setNext(0);
				m_RootNode.setPrev(0);
			}

			void addColumn(ColumnHeader const& column);

			node_iter_t verify(node_iter_t i);

			node_iter_t push_back(ListNode* node)
			{
				node->list = this;
				node->columns.resize(m_columnHeaders.size());
				m_RootNode.children.insert(node);
				node_iter_t i(node);
				i->parent = 0;

				return i;
			}

			node_iter_t push_back(ListNode* node, ListNode* parent)
			{
				node->list = this;
				node->columns.resize(m_columnHeaders.size());
				node_iter_t i = parent->push_back(node);

				return i;
			}

			void clear()
			{
				m_RootNode.children.clear();
			}

			struct NumericLT
			{
				NumericLT(unsigned int column_)
						: column(column_)
				{}

				bool operator()(ListNode* a, ListNode* b)
				{
					return lexical_cast<int>(a->getText(column))
					       > lexical_cast<int>(b->getText(column));
				}

				unsigned int column;
			};

			void sortNumerically(int byColumn)
			{
				NumericLT criteria(byColumn);
				m_RootNode.children.sort(criteria);
			}

			struct LuaLT
			{
				LuaLT(LuaContext& context_, LuaReference comparer)
						: context(context_), comp(comparer)
				{}

				bool operator()(ListNode* a, ListNode* b);

				LuaContext& context;
				LuaReference comp;
			};

			void sortLua(LuaReference comparer);

			node_iter_t getFirstNode()
			{
				return m_RootNode.children.begin();
			}

			virtual int classID();

		private:
			bool verify_(node_iter_t i, node_iter_t n);

			ListNode         m_RootNode;
			std::vector<ColumnHeader> m_columnHeaders;
	};

}

#endif //OMFG_GUI_LIST_H
