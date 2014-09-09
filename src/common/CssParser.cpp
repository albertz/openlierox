// OpenLieroX

#include <assert.h>
#include <algorithm>
#include "CssParser.h"

#include "StringUtils.h"
#include "Timer.h"
#include "FindFile.h"

//
// Css parser class
//

/////////////////////////
// Set the attribute value and parse it
void CSSParser::Attribute::setValue(const std::string &value)
{
	// Remove multiple blank characters and turn all blank characters to spaces
	StringBuf val = value;

	// Check if the value is function-like (for example url(some path.png) or rgb(0 , 50, 0)
	if (val.str().find('(') != std::string::npos && val.str().find(')') != std::string::npos)  {
		std::string func_name = val.readUntil('(');
		TrimSpaces(func_name);

		// Check the name
		bool is_valid_name = true;
		for (std::string::iterator it = func_name.begin(); it != func_name.end(); it++)
			if (!isalnum((uchar)*it) && *it != '_' && *it != '-')
				is_valid_name = false;

		if (is_valid_name)  {
			tParsedValue.push_back(Value(value)); // Just put the whole "function" as a first (and only) value
			return;
		}
		// Not a function, proceed further
	}

	// Parse the value
	std::vector<std::string> tokens = val.splitByBlank();

	for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); it++)  {
		
		// String value
		tParsedValue.push_back(Value(*it));
	}

	sValue = val.str();
}

/////////////////////
// Returns the value represented as a string, converts it to lower case
std::string CSSParser::Attribute::Value::getString(const std::string& def) const
{
	if (sValue.size())
		return stringtolower(sValue);
	else
		return def;
}

/////////////////////
// Returns the value represented as float
float CSSParser::Attribute::Value::getFloat(float def) const
{
	bool fail = false;
	float res = from_string<float>(sValue, fail);
	if (fail)
		return def;
	else
		return res;
}

/////////////////////
// Returns the value represented as integer
int CSSParser::Attribute::Value::getInteger(int def) const
{
	bool fail = false;
	int res = from_string<int>(sValue, fail);
	if (fail)
		return def;
	else
		return res;
}

/////////////////////
// Returns the value represented as boolean
bool CSSParser::Attribute::Value::getBool(bool def) const
{
	if (stringcaseequal(sValue, "true") || stringcaseequal(sValue, "yes") || sValue == "1")
		return true;
	else if (stringcaseequal(sValue, "false") || stringcaseequal(sValue, "no") || sValue == "0")
		return false;
	else
		return def;
}

/////////////////////
// Returns the value represented as URL
std::string CSSParser::Attribute::Value::getURL(const std::string& def) const
{
	std::string res = sValue;
	StripQuotes(res); // Get rid of quotes

	// Check if it is url()
	if (res.size() >= 5)  {
		if (stringcaseequal(res.substr(0, 3), "url"))  {
			if (res[3] == '(' && *res.rbegin() == ')')  {
				res.erase(0, 4);
				res.erase(res.size() - 1);
				StripQuotes(res); // Strip the quotes if any
			}
		}
	}

	if (res.size())
		return res;
	else
		return def;
}

/////////////////////
// Returns the value represented as a color
Color CSSParser::Attribute::Value::getColor(const Color& def) const
{
	bool fail = false;
	Color res = StrToCol(sValue, fail);
	if (fail)
		return def;
	else
		return res;
}

/////////////////////
// Returns the value's unit
std::string CSSParser::Attribute::Value::getUnit(const std::string& def) const
{
	// Must be an integer/float value
	bool fail1 = false, fail2 = false;
	from_string<int>(sValue, fail1);
	from_string<float>(sValue, fail2);
	if (fail1 && fail2)
		return def;

	// Get the unit
	std::string res;
	for (std::string::const_reverse_iterator it = sValue.rbegin(); it != sValue.rend(); it++)  {
		if (isdigit((uchar)*it) || *it == '.')
			break;
		res += tolower((uchar)*it);
	}
	std::reverse(res.begin(), res.end());

	if (res.size())
		return res;
	else
		return def;
}

////////////////////
// Compare operator for Context
bool CSSParser::Selector::Context::operator ==(const CSSParser::Selector::Context &c2) const
{
	// Must have same size
	if (getSize() != c2.getSize())
		return false;

	// Must have same context selectors
	std::list<CSSParser::Selector>::const_iterator it1 = tContextSelectors.begin();
	std::list<CSSParser::Selector>::const_iterator it2 = c2.tContextSelectors.begin();
	for (; it1 != tContextSelectors.end(); it1++, it2++)
		if (!(*it1 == *it2))
			return false;

	return true;
}

////////////////////
// Returns true if this is part of context c2
bool CSSParser::Selector::Context::isPartOf(const CSSParser::Selector::Context& c2) const
{
	// c2 has bo be wider (or at least equal) to contain us
	if (getSize() > c2.getSize())
		return false;

	// Must have same context selectors
	std::list<CSSParser::Selector>::const_reverse_iterator it1 = tContextSelectors.rbegin();
	std::list<CSSParser::Selector>::const_reverse_iterator it2 = c2.tContextSelectors.rbegin();
	for (; it2 != c2.tContextSelectors.rend(); it1++, it2++)
		if (!(*it1 == *it2))
			return false;

	return true;
}

/////////////////////
// Compare operator for two selectors
bool CSSParser::Selector::operator ==(const CSSParser::Selector &s2) const
{
	// If there's some ID, ignore everything else
	if (sID.size())
		return sID == s2.sID && sPseudoClass == s2.sPseudoClass;
		
	// If there's element and class, compare everything
	return sElement == s2.sElement && sClass == s2.sClass && sPseudoClass == s2.sPseudoClass && tContext == s2.tContext;
}

////////////////////////////
// Returns true if the Selector s2 should inherit our attributes
bool CSSParser::Selector::isParentOf(const CSSParser::Selector& s2) const
{
	/*bool psClassOK = (sPseudoClass.size() == 0) || (sPseudoClass == s2.sPseudoClass);
	if (sID.size())
		return psClassOK && sID == s2.sID;
	bool classOK = (sClass.size() == 0) || (sClass == s2.sClass);
	
	if (s2.sID.size())  {
		return ((s2.sElement == sElement) || s2.sElement.size() == 0) && classOK && psClassOK;
	}

	return ((sElement == s2.sElement) || sElement.size() == 0) && classOK && psClassOK;*/
	// The priority is the following:
	// Pseudoclass
	// ID
	// Class
	// Element

	if (sPseudoClass.size() != 0 && s2.sPseudoClass.size() == 0)
		return false;
	if (sPseudoClass != s2.sPseudoClass && sPseudoClass.size() != 0)
		return false;

	if (sID.size() != 0 && s2.sID.size() == 0)
		return false;
	if (sID != s2.sID && sID.size() != 0)
		return false;

	if (sClass.size() != 0 && s2.sClass.size() == 0)
		return false;
	if (sClass != s2.sClass && sClass.size() != 0)
		return false;

	if (sElement.size() != 0 && s2.sElement.size() == 0)
		return false;
	if (sElement != s2.sElement && sElement.size() != 0)
		return false;

	return true;
}

/////////////////////////
// Finds an attribute based on its name
CSSParser::Attribute *CSSParser::Selector::findAttribute(const std::string& name)
{
	for (std::list<Attribute>::iterator it = tAttributes.begin(); it != tAttributes.end(); it++)
		if (it->getName() == name)
			return &(*it);

	return NULL;
}

/////////////////////////
// Merges this selector with another one (adds attributes that are missing in this selector)
void CSSParser::Selector::inheritFrom(const CSSParser::Selector &s2)
{
	for (std::list<Attribute>::const_iterator it = s2.tAttributes.begin(); it != s2.tAttributes.end(); it++)  {
		CSSParser::Attribute *a = findAttribute(it->getName());
		if (a == NULL) // The attribute does not exist, add it
			addAttribute(*it);
	}
}

////////////////////////////
// Add an attribute to the selector
void CSSParser::Selector::addAttribute(const CSSParser::Attribute& a)
{
	Attribute *attr = findAttribute(a.getName());
	if (attr)
		*attr = a; // Overwrite it
	else
		tAttributes.push_back(a);
}

////////////////////////
// Get the error message nicely formatted
std::string CSSParser::Error::getFullMessage() const
{
	return "Parse error on line " + to_string<size_t>(iLine) + ": " + sMessage;
}

/////////////////////////
// Throws a parse error, if fatal is true, the parsing will stop
void CSSParser::throwError(const std::string &msg, bool fatal)
{
	tParseErrors.push_back(Error(iLine, msg, fatal));
	if (fatal)
		throw Error(iLine, msg, fatal);
}


//////////////////////////
// Read a selector
void CSSParser::readSelector()
{
	// Selector ends with an {
	StringBuf s = tCss.readUntil('{');
	if (tCss.atEnd())
		throwError("Unexpected end of file found", true);

	// Trim any spaces
	s.trimBlank();

	// Unnamed selector?
	if (s.empty())  {
		throwError("Unnamed selector found, \"default\" assumed");
		s = "default";
	}

	// There can be more selectors delimited with commas, for example:
	// TEXTBOX, IMAGE, MINIMAP { border: 1px solid black; }
	std::list<Selector> selectors;

	std::vector<std::string> tokens = s.splitBy(',');
	for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); it++)  {
		// Initialize variables
		StringBuf selector_str = *it;
		selector_str.trimBlank();

		// Empty one?
		if (selector_str.empty())  {
			throwError("Unnamed selector found, \"default\" assumed");
			selectors.push_back(Selector("default", "", "", "", sCSSPath));
			continue;
		}

		// The selector can have so called context delimited by spaces, for example:
		// DIALOG LISTVIEW SCROLLBAR
		std::vector<std::string> context = selector_str.splitByBlank();
		size_t i = 0;

		Selector current_sel;
		current_sel.setFilePos(tSelectors.size() + selectors.size());
		for (std::vector<std::string>::iterator c_it = context.begin(); c_it != context.end(); c_it++, i++)  {

			StringBuf cont = *c_it;
			

			// If the selector starts with a sharp (#), it means an ID
			if (cont.getC() == '#')  {
				cont.incPos(); // Skip the sharp

				// Also split by ':' in case of a pseudo class (#selector:pseudoclass)
				if (i == context.size() - 1)  { // The last element is the real selector
					current_sel.setID(cont.readUntil(':'));
					current_sel.setPseudoClass(stringtolower(cont.getRestStr()));
				} else
					current_sel.addContextSelector(Selector("", cont.readUntil(':'), "", stringtolower(cont.getRestStr()), sCSSPath));

				continue;
			}
			cont.resetPos();

			// If the selector starts with a dot (.), it means a pure class
			if (cont.getC()  == '.')  {
				cont.incPos(); // Skip the dot

				// Also split by ':' in case of a pseudo class (.selector:pseudoclass)
				if (i == context.size() - 1)  { // The last element is the real selector
					current_sel.setClass(cont.readUntil(':'));
					current_sel.setPseudoClass(stringtolower(cont.getRestStr()));
				} else
					current_sel.addContextSelector(Selector("", "", cont.readUntil(':'), stringtolower(cont.getRestStr()), sCSSPath));

				continue;
			}
			cont.resetPos();

			// Normal selector
			if (i == context.size() - 1)  { // The last element is the real selector
				// Can have this format:
				// SELECTOR.CLASS:PSEUDOCLASS
				if (cont.str().find('.') != std::string::npos)  {
					current_sel.setElement(stringtolower(cont.readUntil('.')));
					current_sel.setClass(cont.readUntil(':'));
				} else
					current_sel.setElement(stringtolower(cont.readUntil(':')));
				current_sel.setPseudoClass(stringtolower(cont.getRestStr()));
			} else
				current_sel.addContextSelector(Selector(stringtolower(cont.readUntil('.')), "", cont.readUntil(':'), stringtolower(cont.getRestStr()), sCSSPath));
		}

		// Add the selector
		selectors.push_back(current_sel);
	}

	// Read the attributes and add them to all the selectors we've read
	StringBuf attributes = tCss.readUntil('}');
	while (!attributes.atEnd())  {
		const Attribute& a = readAttribute(attributes);
		for (std::list<Selector>::iterator s_it = selectors.begin(); s_it != selectors.end(); s_it++)
			s_it->addAttribute(a);

		// Skip any blank space after the attribute
		// This avoids getting a blank attribute if there are some blank characters after the last
		// attribute (i.e. the attribute before } )
		attributes.skipBlank();
	}

	// Add the selectors to the global selector list
	for (std::list<Selector>::iterator i = selectors.begin(); i != selectors.end(); i++)
		addSelector(*i);

	// Skip any blank spaces after the selector
	tCss.skipBlank();
}

/////////////////////
// Read an attribute
CSSParser::Attribute CSSParser::readAttribute(StringBuf& buf)
{
	// An attribute can ends with a semicolon
	StringBuf attr = buf.readUntil(';');

	// Trim
	attr.trimBlank();

	// Check for !important
	bool important = false;
	for(size_t i = 0; !attr.atEnd(); ++i, attr.incPos())  {
		if (attr.getC() == '!')  {
			size_t len = 1;
			len += attr.skipBlank(); // Skip any blank characters between ! and "important"
			std::string im = attr.read(9); // Read the "important" string (9 = strlen(important))
			len += 9;
			if (stringcaseequal(im, "important"))  { // Found?
				important = true;
				attr.erase(i, len);
				break;
			}
		}
	}
	attr.resetPos();

	// Split to name and value (delimited by ':')
	StringBuf name = attr.readUntil(':');
	StringBuf value = attr.getRestStr();

	// Adjust
	name.trimBlank();
	name.toLower();
	value.trimBlank();

	// Fill in the attribute
	return Attribute(name.str(), value.str(), CSS_PRIORITY, important);
}

///////////////////////
// Find a selector by a name
CSSParser::Selector *CSSParser::findSelector(const Selector &another)
{
	for (std::list<Selector>::iterator it = tSelectors.begin(); it != tSelectors.end(); it++)
		if (*it == another)
			return &(*it);

	return NULL;
}

///////////////////////
// Add a selector to the list
void CSSParser::addSelector(CSSParser::Selector &s)
{
	// Add it
	tSelectors.push_back(s);
}

////////////////////
// Sort predicate to get the correct inheritance order
bool parent_sort_pred(CSSParser::Selector s1, CSSParser::Selector s2)
{
	if (s1 == s2)
		return s1.getFilePos() < s2.getFilePos();
	return s2.isParentOf(s1);
}

//////////////////////////
// Returns a style for the specified element
CSSParser::Selector CSSParser::getStyleForElement(const std::string& element, const std::string& id,
		const std::string& cl, const std::string& pscl, const Selector::Context& context) const
{
	// We have to know what element we are looking for
	assert(element.size() != 0);

	// Create the resulting selector
	Selector result(element, id, cl, pscl, sCSSPath);
	result.setContext(context);

	// Go through the selectors and check, if we can inherit attributes from it
	std::vector<Selector> parents;
	for (std::list<Selector>::const_iterator it = tSelectors.begin(); it != tSelectors.end(); it++)  {
		if (it->isParentOf(result))  {
			Selector tmp;
			tmp = *it;
			tmp.setElement(element);
			parents.push_back(tmp);
		}
	}

	// Sort the parents by their specialization
	//std::sort(parents.begin(), parents.end(), parent_sort_pred);
	bool sorted = false;
	while (!sorted)  {
		sorted = true;
		for (int i=0; i < (int)parents.size() - 1; i++)  {
			if (parents[i + 1].isParentOf(parents[i]))  {
				Selector tmp = parents[i];
				parents[i] = parents[i+1];
				parents[i+1] = tmp;
				sorted = false;
			}
		}
	}

	// Inherit the values
	for (std::vector<Selector>::reverse_iterator it = parents.rbegin(); it != parents.rend(); it++)
		result.inheritFrom(*it);

	return result;
}

/////////////////////
// Removes CSS comments from the string being parsed
void CSSParser::removeComments()
{
	while (true)  {
		// Erase everything between /* and */
		size_t comment_start = tCss.str().find("/*");
		if (comment_start == std::string::npos)
			break;
		size_t comment_end = tCss.str().find("*/", comment_start);
		tCss.erase(comment_start, comment_end - comment_start + 2);
	}
}

/////////////////
// Clear the parser
void CSSParser::clear()
{
	tCss = "";
	tParseErrors.clear();
	tSelectors.clear();
	sCSSPath = "";
	iLine = 0;
}

///////////////////
// Main parsing function
// HINT: nothing is cleared here, the caller is responsible for clearing the parser if necessary
bool CSSParser::parse(const std::string &css, const std::string& path)
{

	// Init the variables
	tCss = css;
	sCSSPath = path;


	// Remove comments from the file
	removeComments();

	// Read all the selectors
	try {
		while (!tCss.atEnd())
			readSelector();
	} catch (...) {
		tCss = "";
		return false;
	}
	tCss = "";

	return true; // Successfully parsed
}

/////////////////////
// Parses the part inside the given selector, for example:
// <tag style="the css here will be parsed with this function">
bool CSSParser::parseInSelector(CSSParser::Selector &sel, const std::string &css, uint32_t priority)
{
	// Read the attributes and add them to all the selectors we've read
	StringBuf attributes = css;
	try  {
		while (!attributes.atEnd())  {
			Attribute a = readAttribute(attributes);
			a.setPriority(priority);
			sel.addAttribute(a);

			// Skip any blank space after the attribute
			// This avoids getting a blank attribute if there are some blank characters after the last
			// attribute (i.e. the attribute before } )
			attributes.skipBlank();
		}
	} catch (...)  {
		return false;
	}

	return true;
}

void CSSParser::test_css()
{
	return;
	AbsTime start = GetTime();

	CSSParser c;
	c.parse(GetFileContents("default.css"), ".");

	printf("==== CSS TEST ====\n");
	printf("Selectors in the file: \n");
	/*for (std::list<CSSParser::Selector>::const_iterator it = c.getSelectors().begin();
		it != c.getSelectors().end(); it++)  {
		// Element info
		printf("  ");
		if (it->getElement().size())
			printf("Element: " + it->getElement() + "  ");
		if (it->getClass().size())
			printf("Class: " + it->getClass() + "  ");
		if (it->getID().size())
			printf("ID: " + it->getID() + "  ");
		if (it->getPseudoClass().size())
			printf("Pseudo class: " + it->getPseudoClass() + "  ");
		printf("\n");

		// Attributes
		for (std::list<CSSParser::Attribute>::const_iterator at = it->getAttributes().begin();
			at != it->getAttributes().end(); at++)  {
			printf("    ");
			printf(at->getName());
			printf("\n");
		}
	}*/
	CSSParser::Selector s = c.getStyleForElement("p", "", "title", "active", CSSParser::Selector::Context());
	c.parseInSelector(s, "some_added_attribute: 20px;some_added_attribute2:15.6px", TAG_CSS_PRIORITY);
	for (std::list<CSSParser::Attribute>::const_iterator at = s.getAttributes().begin();
		at != s.getAttributes().end(); at++)  {
			printf( "%s\n", (at->getName() +  ": " + at->getUnparsedValue()).c_str() );
	}
	printf("Parsing took %f sec\n", (float)(GetTime() - start).seconds());
	printf("==== CSS TEST END ====\n");
}
