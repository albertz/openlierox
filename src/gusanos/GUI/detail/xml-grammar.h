#ifndef OMFG_GUI_XML_GRAMMAR_H
#define OMFG_GUI_XML_GRAMMAR_H

#include <iostream>
#include <string>
#include <vector>

template<class InputT>
int xmlScan(InputT& in)
{
	int c;
	do
	{
		c = in.get();
	} while(c == ' ' || c == '\r' || c == '\n' || c == '\t');
	
	return c;
}

template<class InputT>
void xmlUnScan(InputT& in, int c)
{
	in.putback(c);
}

template<class InputT>
int xmlScanLexeme(InputT& in)
{
	int c = in.get();

	return c;
}

inline bool xmlIsStringChar(int b)
{
	return ((b >= 'a' && b <= 'z')
	     || (b >= 'A' && b <= 'Z')
	     ||  b == '-');
}

template<class InputT>
bool xmlString(InputT& in, std::string& str)
{
	//[a-zA-Z\-]
	int b = xmlScan(in);
	if(xmlIsStringChar(b))
	{
		str.clear();
		do
		{
			str += b;
			b = xmlScanLexeme(in);
		} while(xmlIsStringChar(b));
		
		xmlUnScan(in, b);
		
		return true;
	}
	
	xmlUnScan(in, b);
	
	return false;
}

template<class InputT>
bool xmlQuotedString(InputT& in, std::string& str)
{
	int b = xmlScan(in);
	if(b == '"')
	{
		//[^\"]*\"
		str.clear();
		while(true)
		{
			b = xmlScanLexeme(in);
			if(b == InputT::traits_type::eof())
				return false;
			if(b == '"')
				break;
			str += b;
		}
		
		return true;
	}
	else if(b == '\'')
	{
		//[^\']*\'
		str.clear();
		while(true)
		{
			b = xmlScanLexeme(in);
			if(b == InputT::traits_type::eof())
				return false;
			if(b == '\'')
				break;
			str += b;
		}
		
		return true;
	}
	
	xmlUnScan(in, b);
	
	return false;
}

template<class InputT>
bool xmlAnyString(InputT& in, std::string& str)
{
	if(xmlString(in, str) || xmlQuotedString(in, str))
		return true;
	return false;
}

template<class InputT, class DestT>
bool xmlAttributes(InputT& in, DestT& dest)
{
	//(string '=' quotedString)*
	
	dest.beginAttributes();
	
	std::string label;
	while(xmlString(in, label))
	{
		if(xmlScan(in) != '=')
		{
			dest.error("Expected '='");
			return false;
		}
		std::string value;
		if(!xmlAnyString(in, value))
			return false;
		
		// TODO: Do things with (label, value)
		//std::cout << label << "=\"" << value << "\" ";
		dest.attribute(label, value);
	}
	
	dest.endAttributes();
	
	return true;
}

template<class InputT, class DestT>
bool xmlDocument(InputT& in, DestT& dest)
{
	//('<' string attributes ('>' | '/>') [ document  '</' string '>' ] )*
	int b;
	
	while((b = xmlScan(in)) == '<')
	{
		bool noEnd = false;
		std::string str;
		
		if(!xmlString(in, str))
		{
			if(xmlScan(in) == '/')
			{
				return true;
			}
			
			dest.error("Expected tag label");
			return false;
		}

		dest.beginTag(str);

		if(!xmlAttributes(in, dest))
			return false;

		b = xmlScan(in);
		if(b == '/')
		{
			noEnd = true;
			b = xmlScan(in);
		}
		if(b != '>')
		{
			dest.error("Expected '>'");
			return false;
		}
		
		if(!noEnd)
		{
			if(!xmlDocument(in, dest))
				return false;
			
			// The '</' is parsed by xmlDocument
				
			std::string strEnd;
		
			if(!xmlString(in, strEnd))
			{
				dest.error("Excepted end tag label");
				return false;
			}
				
			if(str != strEnd)
			{
				//Non-matching end tag
				dest.error("Tag label mismatch");
				return false;
			}
			
			if(xmlScan(in) != '>')
			{
				dest.error("Expected '>'");
				return false;
			}
		}
		
		dest.endTag(str);
	}
	
	
	if(b == InputT::traits_type::eof())
	{
		return true;
	}
	
	dest.error("Expected EOF or tag");
	return false;
}

// GSS parser

struct Selector
{
	std::string tagLabel;
	std::string className;
	std::string id;
	std::string state;
};

inline bool gssIsStringChar(int b)
{
	return ((b >= 'a' && b <= 'z')
	     || (b >= 'A' && b <= 'Z')
	     || (b >= '0' && b <= '9')
	     ||  b == '-'  || b == '.'
	     ||  b == '#'  || b == '_'
	     ||  b == '\\' || b == '-'
	     ||  b == '%'  || b == '('
	     ||  b == ')'  || b == '/'
	     );
}


inline bool gssIsSelectorStringChar(int b)
{
	return ((b >= 'a' && b <= 'z')
	     || (b >= 'A' && b <= 'Z')
	     || (b >= '0' && b <= '9')
	     || (b == '-')
	     );
}

template<class InputT>
bool gssString(InputT& in, std::string& str)
{
	//[a-zA-Z0-9.#_\\-%()/]
	int b = xmlScan(in);
	if(gssIsStringChar(b))
	{

		str.clear();
		do
		{
			str += b;
			b = xmlScanLexeme(in);

		} while(gssIsStringChar(b));
		
		xmlUnScan(in, b);
		
		return true;
	}
	
	xmlUnScan(in, b);
	
	return false;
}


template<class InputT>
bool gssSelectorString(InputT& in, std::string& str)
{
	//[a-zA-Z0-9\-]
	int b = xmlScan(in);
	if(gssIsSelectorStringChar(b))
	{

		str.clear();
		do
		{
			str += b;
			b = xmlScanLexeme(in);

		} while(gssIsSelectorStringChar(b));
		
		xmlUnScan(in, b);
		
		return true;
	}
	
	xmlUnScan(in, b);
	
	return false;
}

template<class InputT>
bool gssQuotedString(InputT& in, std::string& str)
{
	int b = xmlScan(in);
	if(b == '"')
	{
		//[^\"]*\"
		str.clear();
		while(true)
		{
			b = xmlScanLexeme(in);
			if(b == InputT::traits_type::eof())
				return false;
			if(b == '"')
				break;
			str += b;
		}
		
		return true;
	}
	else if(b == '\'')
	{
		//[^\']*\'
		str.clear();
		while(true)
		{
			b = xmlScanLexeme(in);
			if(b == InputT::traits_type::eof())
				return false;
			if(b == '\'')
				break;
			str += b;
		}
		
		return true;
	}
	
	xmlUnScan(in, b);
	
	return false;
}

template<class InputT>
bool gssAnyString(InputT& in, std::string& str)
{
	if(gssString(in, str) || gssQuotedString(in, str))
		return true;
	return false;
}

template<class InputT, class DestT>
bool gssDeclaration(InputT& in, DestT& dest, std::vector<Selector> const& selectors)
{
	std::string property;

	if(!gssString(in, property))
		return true;
		
	if(xmlScan(in) != ':')
	{
		dest.error("Expected ':'");
		return false;
	}
	
	std::vector<std::string> values;
	
	std::string value;
	if(!gssAnyString(in, value))
	{
		dest.error("Expected value");
		return false;
	}

	do
	{
		values.push_back(value);
	} while(gssAnyString(in, value));
	
	for(std::vector<Selector>::const_iterator i = selectors.begin();
	    i != selectors.end();
	    ++i)
	{
		dest.selector(i->tagLabel, i->className, i->id, i->state, property, values);
	}
	
	return true;
}



template<class InputT>
bool gssCharacter(InputT& in, char wanted)
{
	char b;
	if((b = xmlScan(in) == wanted))
		return true;
	in.putback(b);
	return false;
}

template<class InputT, class DestT>
bool gssSelector(InputT& in, DestT& dest)
{
	bool succ = gssSelectorString(in, dest.tagLabel);
	// First, get tag label
	char b = xmlScan(in);
	if(b == '.')
	{
		gssSelectorString(in, dest.className);
		b = xmlScan(in);
		succ = true;
	}
	if(b == '#')
	{
		gssSelectorString(in, dest.id);
		b = xmlScan(in);
		succ = true;
	}
	if(b == ':')
	{
		gssSelectorString(in, dest.state);
		succ = true;
	}
	else
		in.putback(b);
		
	return succ;
}

template<class InputT, class DestT>
bool gssSheet(InputT& in, DestT& dest)
{
	Selector selector;
	while(true)
	{
		selector = Selector();
		if(!gssSelector(in, selector))
			break;
			
		std::vector<Selector> selectors;
		selectors.push_back(selector);
		
		//Parse selector
		
		
		char b;
		while((b = xmlScan(in)) == ',')
		{
			if(!gssSelector(in, selector))
			{
				dest.error("Expected selector");
				return false;
			}
			selectors.push_back(selector);
		}
		
		if(b != '{')
		{
			dest.error("Expected '{'");
			return false;
		}
	
		if(!gssDeclaration(in, dest, selectors))
		{
			dest.error("Expected property");
			return false;
		}
		
		while((b = xmlScan(in)) == ';')
		{
			if(!gssDeclaration(in, dest, selectors))
			{
				return false;
			}
		}
		
		if(b != '}')
		{
			dest.error("Expected '}'");
			return false;
		}
	}
	
	if(xmlScan(in) == InputT::traits_type::eof())
	{
		return true;
	}

	dest.error("Expected EOF or selector");
	return false;
}



#endif //OMFG_GUI_XML_GRAMMAR_H
