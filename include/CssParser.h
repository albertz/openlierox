#ifndef __CSS_PARSER_H__
#define __CSS_PARSER_H__

#include <string>
#include <list>
#include <vector>
#include <stdint.h>
#include "Color.h" // for Color
#include "StringBuf.h"
#include "StyleVar.h"

// Useful macros
#define FOR_EACH_ATTRIBUTE(selector, iter_name) for (std::list<CSSParser::Attribute>::const_iterator iter_name = selector.getAttributes().begin(); iter_name != selector.getAttributes().end(); iter_name++)

// Css parser class
class CSSParser  {
public:
	CSSParser() : iLine(0) {}

public:
	// CSS attribute class
	class Attribute  {
	public:
		Attribute() : iPriority(DEFAULT_PRIORITY) {}
		Attribute(const std::string& name, const std::string& val, uint32_t priority, bool important = false) : 
			sName(name), bImportant(important), iPriority(priority) { setValue(val); }
	public:
		// Attribute value
		class Value  {
		public:
			// Unit of the value (only for float/int types)

			Value() : sValue("") {}
			Value(const std::string& s) : sValue(s) {}
		private:
			std::string sValue;

		public:
			Value& operator=(const Value& v2)  {
				if (&v2 != this)  {
					sValue = v2.sValue;
				}
				return *this;
			}

			void setValue(const std::string& s)	{ sValue = s; }

			std::string getString(const std::string& def = "") const;
			float getFloat(float def = 0) const;
			int getInteger(int def = 0) const;
			bool getBool(bool def = true) const;
			std::string getUnit(const std::string& def = "") const;
			std::string getURL(const std::string& def = "") const;
			Color getColor(const Color& def = Color()) const;
		};
	private:
		std::string sName;
		std::string sValue;
		std::vector<Value> tParsedValue;
		bool bImportant;
		uint32_t iPriority;

	public:
		Attribute& operator=(const Attribute& a2)  {
			if (&a2 != this)  {
				sName = a2.sName;
				sValue = a2.sValue;
				tParsedValue = a2.tParsedValue;
				bImportant = a2.bImportant;
			}
			return *this;
		}

		// Compare operator
		bool operator==(const Attribute& a2) const  {
			return sName == a2.sName;
		}

		bool operator!=(const Attribute& a2) const  {
			return sName != a2.sName;
		}

		void setName(const std::string& name)	{ sName = name; }
		void setValue(const std::string& value);
		void setImportant(bool i)				{ bImportant = i; }
		void setPriority(uint32_t p)				{ iPriority = p; }

		uint32_t getPriority() const		{ return bImportant ? iPriority + 1 : iPriority; }
		bool isImportant() const		{ return bImportant; }
		const std::string& getName() const	{ return sName; }
		const std::string& getUnparsedValue() const	{ return sValue; }
		const std::vector<Value>& getParsedValue() const { return tParsedValue; }
		const Value& getFirstValue() const { return *tParsedValue.begin(); }
	};

	// CSS selector class
	class Selector  {
	public:
		Selector() : iFilePos((size_t)-1) {}
		Selector(const std::string& el, const std::string& id, const std::string& cl, const std::string& pscl, const std::string& base_url) :
		sElement(el), sID(id), sClass(cl), sPseudoClass(pscl), sBaseUrl(base_url), iFilePos((size_t)-1) {}
		
		// Selector context class
		// HINT: the higher in the hierarchy the selector is, the higher in the list, for example:
		// UL UL LI will result in list:
		// [0] UL
		// [1] UL
		// [2] LI
		class Context  {
		public:
			Context() {}
		private:
			std::list<Selector> tContextSelectors;
		public:
			Context& operator=(const Context& c2)  {
				if (this != &c2)  {
					tContextSelectors = c2.tContextSelectors;
				}
				return *this;
			}

			// Methods
			bool operator==(const Context& c2) const;
			bool operator!=(const Context& c2) const  { return !(*this == c2); }
			size_t getSize() const	{ return tContextSelectors.size(); }
			bool isEmpty()		{ return tContextSelectors.empty(); }
			void appendSelector(const Selector& s)  { tContextSelectors.push_back(s); }
			void addFrontSelector(const Selector& s) { tContextSelectors.push_front(s); } // For reverse adding
			bool isPartOf(const Context& c2) const;
		};

	private:
		std::string sElement;
		std::string sID;
		std::string sClass;
		std::string sPseudoClass;
		std::string	sBaseUrl;
		Context tContext;
		std::list<Attribute> tAttributes;
		size_t iFilePos;
	public:
		// Copy operator
		Selector& operator=(const Selector& s2)  {
			if (&s2 != this)  {
				sElement = s2.sElement;
				sID = s2.sID;
				sClass = s2.sClass;
				sPseudoClass = s2.sPseudoClass;
				sBaseUrl = s2.sBaseUrl;
				tContext = s2.tContext;
				tAttributes = s2.tAttributes;
			}
			return *this;
		}

		// Compare operator
		bool operator==(const Selector& s2) const;
		bool operator!=(const Selector& s2) const  { return !(*this == s2); }

		Attribute *findAttribute(const std::string& name);
		void inheritFrom(const Selector& s2);
		bool isParentOf(const Selector& s2) const;

		void setElement(const std::string& e)  { sElement = e; }
		void setID(const std::string& i)  { sID = i; }
		void setClass(const std::string& c)  { sClass = c; }
		void setPseudoClass(const std::string& p)  { sPseudoClass = p; }
		void setBaseURL(const std::string& url)		{ sBaseUrl = url; }
		void setFilePos(size_t pos)	{ iFilePos = pos; }
		void addContextSelector(const Selector& s) { tContext.appendSelector(s); }
		void setContext(const Context& c)	{ tContext = c; }
		void addAttribute(const Attribute& a);

		const std::string& getElement() const  { return sElement; }
		const std::string& getID() const		{ return sID; }
		const std::string& getClass() const 	{ return sClass; }
		const std::string& getPseudoClass() const { return sPseudoClass; }
		const std::string& getBaseURL() const	{ return sBaseUrl; }
		const Context& getContext() const { return tContext; }
		size_t getFilePos() const { return iFilePos; }
		const std::list<Attribute>& getAttributes() const { return tAttributes; }
	};

	// Error class
	class Error  {
	public:
		Error() : iLine(0), bFatal(false) {}
		Error(size_t line, const std::string msg, bool fatal) : iLine(line), sMessage(msg), bFatal(fatal) {}
	private:
		size_t iLine;
		std::string sMessage;
		bool bFatal;
	public:
		Error& operator=(const Error& e2)  {
			if (&e2 != this)  {
				iLine = e2.iLine;
				sMessage = e2.sMessage;
				bFatal = e2.bFatal;
			}

			return *this;
		}

		std::string getFullMessage() const;
		const std::string& getMessage()  { return sMessage; }
		size_t getLine()  { return iLine; }
		bool isFatal()  { return bFatal; }
	};

private:
	std::list<Selector> tSelectors;
	std::string sCSSPath;

	size_t iLine;
	StringBuf tCss;

	std::list<Error> tParseErrors;

private:
	void throwError(const std::string& msg, bool fatal = false);

	void readSelector();
	Attribute readAttribute(StringBuf& buf);
	void removeComments();

public:
	Selector *findSelector(const Selector& another);
	Selector getStyleForElement(const std::string& element, const std::string& id,
		const std::string& cl, const std::string& pscl, const Selector::Context& context) const;
	void addSelector(Selector& s);
	bool parse(const std::string& css, const std::string& path);
	bool parseInSelector(Selector& sel, const std::string& css, uint32_t priority);  // Used for parsing <tag style="css here">
	void clear();
	const std::list<Selector>& getSelectors()		{ return tSelectors; }
	
	static void test_css();
};

#endif
