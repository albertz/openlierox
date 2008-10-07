/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Useful functions for XML/HTML parsing
// Created 24/7/08
// Karel Petranek

#include "XMLutils.h"
#include "debug.h"
#include "StringUtils.h"

///////////////////
// Get an integer from the specified node
int xmlGetInt(xmlNodePtr node, const std::string& name, int def)
{
	// Read
	xmlChar *sValue = xmlGetProp(node, (const xmlChar *)name.c_str());
	if(!sValue)
		return def;

	// Convert
	bool fail = false;
	int result = from_string<int>((const char *)sValue, fail);
	if (fail)
		result = def;

	// Cleanup
	xmlFree(sValue);
	return result;
}

///////////////////
// Get a float from the specified node
float xmlGetFloat(xmlNodePtr node, const std::string& name, float def)
{
	// Read
	xmlChar *sValue = xmlGetProp(node, (const xmlChar *)name.c_str());
	if (!sValue)
		return def;

	// Convert
	bool fail = false;
	float result = from_string<float>((const char *)sValue, fail);
	if (fail)
		result = def;

	// Cleanup
	xmlFree(sValue);
	return result;
}

///////////////////
// Get a colour from the specified node
Color xmlGetColour(xmlNodePtr node, const std::string& name, const Color& def)
{
	// Get the value
	xmlChar *sValue = xmlGetProp(node, (const xmlChar *)name.c_str());
	if (!sValue)
		return def;

	// Convert
	bool fail = false;
	Color result = StrToCol((const char *)sValue, fail);
	if (fail)
		return def;

	//Cleanup
	xmlFree(sValue);
	return result;
}

///////////////////
// Get a string from the specified node
std::string xmlGetString(xmlNodePtr node, const std::string& name, const std::string& def)
{
	// Get the value
	xmlChar *sValue = xmlGetProp(node, (const xmlChar *)name.c_str());
	if (!sValue)
		return def;

	// Convert
	std::string result((const char *)sValue);

	//Cleanup
	xmlFree(sValue);
	return result;
}

///////////////////
// Get a boolean from the specified node
bool xmlGetBool(xmlNodePtr node, const std::string& name, bool def)
{
	// Get the value
	xmlChar *sValue = xmlGetProp(node, (const xmlChar *)name.c_str());
	if (!sValue)
		return def;

	// Convert
	bool result = def;
	if (!xmlStrcasecmp(sValue, (const xmlChar *)"true"))
		result = true;
	else if (!xmlStrcasecmp(sValue, (const xmlChar *)"yes"))
		result = true;
	else if (!xmlStrcasecmp(sValue, (const xmlChar *)"1"))
		result = true;
	else if (!xmlStrcasecmp(sValue, (const xmlChar *)"on"))
		result = true;
	else if (!xmlStrcasecmp(sValue, (const xmlChar *)"false"))
		result = false;
	else if (!xmlStrcasecmp(sValue, (const xmlChar *)"no"))
		result = false;
	else if (!xmlStrcasecmp(sValue, (const xmlChar *)"0"))
		result = false;
	else if (!xmlStrcasecmp(sValue, (const xmlChar *)"off"))
		result = false;

	//Cleanup
	xmlFree(sValue);
	return result;
}

////////////////
// Get a text from the node
std::string xmlNodeText(xmlNodePtr node, const std::string& def)
{
	if (node->doc && node->children)  {
		const char *str = (const char *)xmlNodeListGetString(node->doc, node->children, 1);
		if (str)  {
			std::string res = str;
			xmlFree((xmlChar *)str);
			return res;
		} else {
			return def;
		}
	} else
		return def;
}

//////////////////
// Get the base URL for the given node
std::string xmlGetBaseURL(xmlNodePtr node)
{
	if (node)
		if (node->doc)
			if (node->doc->URL)  {
				std::string res((const char *)node->doc->URL);
				return res;
			}
	return "";
}

/////////////////
// Replaces all the escape characters with html entities
void xmlEntityText(std::string& text)
{
	replace(text,"\"","&quot;",text);  // "
	replace(text,"'", "&apos;",text);  // '
	replace(text,"&", "&amp;", text);  // &
	replace(text,"<", "&lt;",  text);  // <
	replace(text,">", "&gt;",  text);  // >
}

std::string xmlEntities(const std::string& text)
{
	std::string res = text;
	xmlEntityText(res);
	return res;
}
