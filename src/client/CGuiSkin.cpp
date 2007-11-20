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
#include "CGuiSkinnedLayout.h"
#include "CWidget.h"
#include "CWidgetList.h"

#include "LieroX.h"
#include "AuxLib.h"
#include "Menu.h"
#include "StringUtils.h"
#include "Cursor.h"

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

CGuiSkin::SkinVarPtr_t CGuiSkin::GetVar( const std::string & name, CGuiSkin::SkinVarType_t type )
{
	for( std::map< std::string, SkinVarPtr_t > :: iterator it = CGuiSkin::Vars().begin();
			it != CGuiSkin::Vars().end(); it++ )
	{
		if( !stringcasecmp( it->first, name ) && it->second.type == type )
		{
			return it->second;
		};
	};
	return CGuiSkin::SkinVarPtr_t( (bool *) NULL );
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
			case SVT_CALLBACK: ret << "callback: "; break;
		};
		ret << "\n";
	};
	return ret.str();
};

std::string CGuiSkin::DumpWidgets()
{
	Init();
	std::ostringstream ret;
	for( std::map< std::string, std::pair< paramListVector_t, WidgetCreator_t > > :: iterator it = 
			m_instance->m_widgets.begin();	it != m_instance->m_widgets.end(); it++ )
	{
		ret << it->first + "( ";
		for( unsigned f = 0; f < it->second.first.size(); f++ )
		{
			if( f != 0 ) ret << ", ";
			switch( it->second.first[f].second )
			{
				case WVT_BOOL: ret << "bool"; break;
				case WVT_INT: ret << "int"; break;
				case WVT_FLOAT: ret << "float"; break;
				case WVT_STRING: ret << "string"; break;
				case WVT_COLOR: ret << "color"; break;
			};
			ret << " " << it->second.first[f].first;
		};
		ret << " )\n";
	};
	return ret.str();
};

bool CGuiSkin::InitLayouts( const std::string & filename )
{
	Init();
	m_instance->m_guis.clear();
	m_instance->m_guisShowing.clear();
	CGuiSkinnedLayout * ll = GetLayout( filename );
	if( ll == NULL )
		return false;
	m_instance->m_guisShowing.push_back(ll);
	return true;
};

void CGuiSkin::DrawLayouts( SDL_Surface *bmpDest )
{
	Init();
	if( m_instance->m_guisShowing.size() == 0 ) 
		return;
	// TODO: if layout is covering all screen do not draw layouts below it
	for( unsigned i=0; i<m_instance->m_guisShowing.size(); i++ )
	{
		m_instance->m_guisShowing[i]->Draw( bmpDest );
	};
	//m_instance->m_guisShowing[ m_instance->m_guisShowing.size()-1 ] -> Draw( bmpDest );
};

bool CGuiSkin::ProcessLayouts()
{
	Init();
	if( m_instance->m_guisShowing.size() == 0 ) 
		return false;
	gui_event_t * ev = m_instance->m_guisShowing[ m_instance->m_guisShowing.size()-1 ] -> Process();
	if( ev != NULL )
	{
		ev->cWidget->ProcessGuiSkinEvent(ev->iEventMsg);	// Loop event back to widget
	};
	return true;
};

static bool xmlGetBool(xmlNodePtr Node, const std::string& Name);
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
	{
		printf("Cannot read GUI skin file %s\n", ("data/frontend/skins/" + skinpath + "/" + filename + ".xml").c_str() );
		return NULL;
	};

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
	
	if ( stringcasecmp( (const char *)Node->name, "dialog" ) ) 
	{
		printf("GUI skin file %s is invalid: root item should be \"dialog\"\n", filepath.c_str() );
		return NULL;
	};
	
	int widgetID = 0;

	Node = Node->xmlChildrenNode;
	while (Node != NULL)  
	{
		int left   = xmlGetInt(Node,"left");
		int top    = xmlGetInt(Node,"top");
		int width  = xmlGetInt(Node,"width");
		int height = xmlGetInt(Node,"height");
		bool disabled = xmlGetBool(Node,"disabled");	// By default all widgets are enabled and all bools are false

		std::map< std::string, std::pair< paramListVector_t, WidgetCreator_t > > :: iterator it;
		if (CMP(Node->name,"text"))	// Some extra newline - skip it
		{
			//printf("XML text inside \"%s\": \"%s\"\n", Node->parent->name, Node->content );
		}
		else
		for( it = m_instance->m_widgets.begin(); it != m_instance->m_widgets.end(); it++ )
		{
			if( stringcasecmp( it->first.c_str(), (const char *)Node->name ) )
				continue;
			
			std::vector< WidgetVar_t > params;
			for( unsigned i = 0; i < it->second.first.size(); i++ )
			{
				if( it->second.first[i].second == WVT_BOOL )
					params.push_back( WidgetVar_t( xmlGetBool( Node, it->second.first[i].first ) ) );
				else if( it->second.first[i].second == WVT_INT )
					params.push_back( WidgetVar_t( xmlGetInt( Node, it->second.first[i].first ) ) );
				else if( it->second.first[i].second == WVT_FLOAT )
					params.push_back( WidgetVar_t( xmlGetFloat( Node, it->second.first[i].first ) ) );
				else if( it->second.first[i].second == WVT_STRING )
					params.push_back( WidgetVar_t( xmlGetString( Node, it->second.first[i].first ) ) );
				else if( it->second.first[i].second == WVT_COLOR )
					params.push_back( WidgetVar_t( xmlGetColor( Node, it->second.first[i].first ) ) );
				else params.push_back( WidgetVar_t( ) );	// Compile-time error here
			};
			
			CWidget * widget = it->second.second( params );
			widget->setEnabled( ! disabled );
			gui->Add( widget, widgetID++, left, top, width, height );
			widget->ProcessGuiSkinEvent(INIT_WIDGET);

			break;
		};
		if( it == m_instance->m_widgets.end() )
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
// Get a bool from the specified property
bool xmlGetBool(xmlNodePtr Node, const std::string& Name)
{
	xmlChar *sValue;
	sValue = xmlGetProp(Node,(const xmlChar *)Name.c_str());
	if(!sValue)
		return false;
	bool result = false;
	if( !stringcasecmp( "true", (const char *)sValue ) )
		result = true;
	if( !stringcasecmp( "1", (const char *)sValue ) )
		result = true;
	xmlFree(sValue);
	return result;
}


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
	if (!sValue)
		return 0;
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

int		Menu_CGuiSkinInitialize(void)
{
	DrawRectFill(tMenu->bmpBuffer, 0, 0, 640-1, 480-1, tLX->clBlack);
	//DrawRectFill(tMenu->bmpBuffer, 0, 0, 640-1, 480-1, tLX->clBlack);
	//Menu_DrawBox(tMenu->bmpBuffer, 0, 0, 640-1, 480-1,);
	//DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_common,0,0);
	CGuiSkin::InitLayouts();
	SetGameCursor(CURSOR_ARROW);
	tMenu->iMenuType = MNU_GUISKIN;
	//DrawImage(tMenu->bmpScreen, tMenu->bmpBuffer, 0, 0);	// TODO: hacky hacky
	return true;
};

void	Menu_CGuiSkinFrame(void)
{
	if( ! CGuiSkin::ProcessLayouts() )
	{
		Menu_CGuiSkinShutdown();
		Menu_MainInitialize();
	};	
	CGuiSkin::DrawLayouts(tMenu->bmpBuffer);
	DrawCursor(tMenu->bmpBuffer);
	DrawImage(tMenu->bmpScreen, tMenu->bmpBuffer, 0, 0);	// TODO: hacky hacky
};

void	Menu_CGuiSkinShutdown(void)
{
	DrawRectFill(tMenu->bmpBuffer, 0, 0, 640-1, 480-1, tLX->clBlack);
	DrawImage(tMenu->bmpScreen, tMenu->bmpBuffer, 0, 0);
	SetGameCursor(CURSOR_NONE);
};

void CGuiSkin::ExitDialog( const std::string & param )
{
	Init();
	if( m_instance->m_guisShowing.size() == 0 )
		return;
	m_instance->m_guisShowing.pop_back();
};

void CGuiSkin::ChildDialog( const std::string & param )
{
	Init();
	// Simple parsing of params
	std::vector<std::string> v = explode(param, ",");
	int x = 0, y = 0;
	if( v.size() > 2 )
	{
		x = atoi( v[1] );
		y = atoi( v[2] );
	};
	std::string file = v[0];
	TrimSpaces(file);
	CGuiSkinnedLayout * ll = GetLayout( file );
	if( ll == NULL )
		return;
	ll->SetOffset(x,y);
	m_instance->m_guisShowing.push_back(ll);
};

void CGuiSkin::SubstituteDialog( const std::string & param )
{
	Init();
	if( m_instance->m_guisShowing.size() == 0 )
		m_instance->m_guisShowing.pop_back();
	m_instance->m_guisShowing.push_back( GetLayout( param ) );
};

void CGuiSkin::CallbackHandler::Init( const std::string & s1 )
{
	// TODO: put LUA handler here, this handmade string parser works but the code is ugly
	//printf( "CGuiSkin::CallbackHandler::Init(\"%s\"): ", s1.c_str() );
	m_callbacks.clear();
	std::string s(s1);
	TrimSpaces(s);
	while( s.size() > 0 )
	{
		std::string::size_type i = s.find_first_of( " \t(" );
		std::string func = s.substr( 0, i );
		std::string param;
		if( i != std::string::npos )
			i = s.find_first_not_of( " \t", i );
		if( i != std::string::npos )
			if( s[i] == '(' )	// Param
			{
				i++;
				std::string::size_type i1 = i;
				i = s.find_first_of( ")", i );
				if( i != std::string::npos )
				{
					param = s.substr( i1, i-i1 );
					i++;
				};
			};
		TrimSpaces(param);
		if( i != std::string::npos )
			s = s.substr( i );
		else
			s = "";
		TrimSpaces(s);

		for( std::map< std::string, SkinVarPtr_t > :: iterator it = CGuiSkin::Vars().begin();
				it != CGuiSkin::Vars().end(); it++ )
		{
			if( !stringcasecmp( it->first, func ) && it->second.type == SVT_CALLBACK )
			{
				m_callbacks.push_back( std::pair< SkinCallback_t, std::string > ( it->second.c, param ) );
				//printf("%s(\"%s\") ", it->first.c_str(), param.c_str());
			};
		};
	};
	//printf("\n");
};

void CGuiSkin::CallbackHandler::Call()
{
	for( unsigned f=0; f<m_callbacks.size(); f++ )
		m_callbacks[f].first( m_callbacks[f].second );
};

bool bRegisteredCallbacks = CGuiSkin::RegisterVars("GUI")
	( & CGuiSkin::ExitDialog, "ExitDialog" )
	( & CGuiSkin::ChildDialog, "ChildDialog" )
	( & CGuiSkin::SubstituteDialog, "SubstituteDialog" )
	;

void MakeSound( const std::string & param )	// For debug
{
	if( param == "" || param == "click" )
		PlaySoundSample(sfxGeneral.smpClick);
	if( param == "chat" )
		PlaySoundSample(sfxGeneral.smpChat);
	if( param == "chat" )
		PlaySoundSample(sfxGeneral.smpChat);
	if( param == "ninja" )
		PlaySoundSample(sfxGame.smpNinja);
	if( param == "pickup" )
		PlaySoundSample(sfxGame.smpPickup);
	if( param == "bump" )
		PlaySoundSample(sfxGame.smpBump);
	if( param == "death" || param == "death1" )
		PlaySoundSample(sfxGame.smpDeath[0]);
	if( param == "death2" )
		PlaySoundSample(sfxGame.smpDeath[1]);
	if( param == "death3" )
		PlaySoundSample(sfxGame.smpDeath[2]);
};

bool bRegisteredCallbacks1 = CGuiSkin::RegisterVars("GUI")
	( & MakeSound, "MakeSound" );

