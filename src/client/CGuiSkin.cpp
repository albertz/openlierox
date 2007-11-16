/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#include "CGuiSkin.h"

#include "LieroX.h"
#include "AuxLib.h"
#include "Menu.h"
#include "StringUtils.h"
#include "CBox.h"
#include "CImage.h"
#include "CButton.h"
#include "CCheckbox.h"
#include "CLabel.h"
#include "CSlider.h"
#include "CTextbox.h"
#include "Options.h"

#include <sstream>
// XML parsing library
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>


CGuiSkin * CGuiSkin::m_instance = NULL;

std::string CGuiSkin::StripClassName( const std::string & c )
{
	std::string ret(c);
	if( ret.find(".") != std::string::npos )	// Leave only last part of name
		ret = ret.substr( ret.find(".") + 1 );
	if( ret.find("->") != std::string::npos )
		ret = ret.substr( ret.find("->") + 2 );
	ret = ret.substr( ret.find_first_not_of(" \t") );	// Strip spaces
	ret = ret.substr( 0, ret.find_last_not_of(" \t") + 1 );
	return ret;
};

std::string CGuiSkin::DumpVars()
{
	Init();
	std::ostringstream ret;
	for( std::map< std::string, SkinVarPtr_t > :: iterator i = m_instance->m_vars.begin();
			i != m_instance->m_vars.end(); i++ )
	{
		ret << i->first + ": ";
		switch( i->second.type )
		{
			case SVT_BOOL: ret << "bool: " << *i->second.b; break;
			case SVT_INT: ret << "int: " << *i->second.i; break;
			case SVT_FLOAT: ret << "float: " << *i->second.f; break;
			case SVT_STRING: ret << "string: \"" << *i->second.s << "\""; break;
			case SVT_CALLBACK: ret << "callback: " << *i->second.c; break;
		};
		ret << "\n";
	};
	return ret.str();
};

void CGuiSkin::ClearLayouts()
{
	Init();
	m_instance->m_guis.clear();
};

static int xmlGetInt(xmlNodePtr Node, const std::string& Name);
static float xmlGetFloat(xmlNodePtr Node, const std::string& Name);
static Uint32 xmlGetColor(xmlNodePtr Node, const std::string& Name);
static std::string xmlGetString(xmlNodePtr Node, const std::string& Name);
#define		CMP(str1,str2)  (!xmlStrcmp((const xmlChar *)str1,(const xmlChar *)str2))


CGuiSkinnedLayout * CGuiSkin::GetLayout( const std::string & filename )
{
	Init();
	std::string filepath;
	std::string skinpath( tLXOptions->sSkinPath );
	if( skinpath == "" ) skinpath = "default";
	skinpath += tLXOptions->sResolution;
	if( ! GetExactFileName( "data/frontend/skins/" + skinpath + "/" + filename + ".xml", filepath ) )
		return NULL;

	if( m_instance->m_guis.find( filepath ) != m_instance->m_guis.end() )
		return m_instance->m_guis[ filepath ];

	xmlDocPtr	Doc;
	xmlNodePtr	Node;
	CGuiSkinnedLayout * gui = new CGuiSkinnedLayout();

	Doc = xmlParseFile(filepath.c_str());
	if (Doc == NULL)  
	{
		printf("Cannot parse GUI skin file %s\n", filepath.c_str() );
		return NULL;
	};
	
	Node = xmlDocGetRootElement(Doc);
	if (Node == NULL)
	{
		printf("GUI skin file %s is empty\n", filepath.c_str() );
		return NULL;
	};
	
	if ( ! CMP( Node->name, "dialog" ) )  
	{
		printf("GUI skin file %s is invalid: root item should be \"dialog\"\n", filepath.c_str() );
		return NULL;
	};

	Node = Node->xmlChildrenNode;
	while (Node != NULL)  
	{
		int left   = xmlGetInt(Node,"left");
		int top    = xmlGetInt(Node,"top");
		int width  = xmlGetInt(Node,"width");
		int height = xmlGetInt(Node,"height");

		if (CMP(Node->name,"box"))  
		{
			int round  = xmlGetInt(Node,"round");
			int border = xmlGetInt(Node,"border");
			Uint32 light_color = xmlGetColor(Node,"lightcolor");
			Uint32 dark_color = xmlGetColor(Node,"darkcolor");
			Uint32 bgcolor = xmlGetColor(Node,"bgcolor");

			gui->Add(new CBox(round,border,light_color,dark_color,bgcolor),-1,left,top,width,height);
		} 
		else if (CMP(Node->name,"label"))  
		{
			std::string name = xmlGetString(Node,"name");
			std::string text = xmlGetString(Node,"text");
			Uint32 color = xmlGetColor(Node,"color");
			//generic_events_t Events;
			//CGuiLayout_ReadEvents(this,tCurrentNode,&Events);

			// Add the label
			CLabel *Label = new CLabel(text,color);
			gui->Add(Label,gui->GetIdByName(name.c_str()),left,top,width,height);
			//Label->SetupEvents(&Events);
		} 
		else if (CMP(Node->name,"text"))	// Some extra newline - skip it
		{
			//printf("XML text inside \"%s\": \"%s\"\n", Node->parent->name, Node->content );
		}
		else
		{
			printf("GUI skin file %s is invalid: invalid item \"%s\"\n", filepath.c_str(), Node->name );
		};

		Node = Node->next;
	};

	xmlFree(Doc);
	m_instance->m_guis[ filepath ] = gui;
	printf("GUI skin file %s loaded\n", filepath.c_str() );
	return gui;
};




///////////////////
// Get an integer from the specified property
int xmlGetInt(xmlNodePtr Node, const std::string& Name)
{
	xmlChar *sValue;
	sValue = xmlGetProp(Node,(const xmlChar *)Name.c_str());
	if(!sValue)
		return 0;
	int result = atoi((const char *)sValue);
	xmlFree(sValue);
	return result;
}

///////////////////
// Get a float from the specified property
float xmlGetFloat(xmlNodePtr Node, const std::string& Name)
{
	xmlChar *sValue = xmlGetProp(Node,(const xmlChar *)Name.c_str());
	if (!sValue)
		return 0;
	float result = (float)atof((const char *)sValue);
	xmlFree(sValue);
	return result;
}

///////////////////
// Get a colour from the specified property
Uint32 xmlGetColor(xmlNodePtr Node, const std::string& Name)
{
	xmlChar *sValue;

	// Get the value
	sValue = xmlGetProp(Node,(const xmlChar *)Name.c_str());

	Uint32 result = StrToCol((char*)sValue);

	xmlFree(sValue);
	return result;
}

std::string xmlGetString(xmlNodePtr Node, const std::string& Name)
{
	xmlChar *sValue = xmlGetProp(Node,(const xmlChar *)Name.c_str());
	if (!sValue)
		return "";
	std::string ret = (const char *)sValue;
	xmlFree(sValue);
	return ret;
}
