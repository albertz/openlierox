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

void CGuiSkin::ClearLayouts()
{
	Init();
	for( std::map< std::string, CGuiSkinnedLayout * > :: iterator it = m_instance->m_guis.begin();
		it != m_instance->m_guis.end(); it++ )
				delete it->second;
	m_instance->m_guis.clear();
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
	if( skinpath == "" )
		return NULL;
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
	
	Node = Node->xmlChildrenNode;
	while (Node != NULL)  
	{
		int left   = xmlGetInt(Node,"left");
		int top    = xmlGetInt(Node,"top");
		int width  = xmlGetInt(Node,"width");
		int height = xmlGetInt(Node,"height");
		bool disabled = xmlGetBool(Node,"disabled");	// By default all widgets are enabled and all bools are false
		std::string s_id = xmlGetString(Node,"id");	// Widget ID (used for enable/disable it by func handlers)
		std::string init = xmlGetString(Node,"init");	// OnInit handler - fills list or combobox etc
		std::string s_pos = xmlGetString(Node,"pos");
		if( s_pos != "" )
		{
			std::vector<std::string> pos = explode( s_pos, "," ); // "left,top,width,height" as single string
			if( pos.size() > 0 )
				left = atoi( pos[0] );
			if( pos.size() > 1 )
				top = atoi( pos[1] );
			if( pos.size() > 2 )
				width = atoi( pos[2] );
			if( pos.size() > 3 )
				height = atoi( pos[3] );
		};
		std::map< std::string, std::pair< paramListVector_t, WidgetCreator_t > > :: iterator it;
		if ( CMP(Node->name,"text") || CMP(Node->name,"comment") )	// Some extra newline or comment - skip it
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
			int i_id = -1;
			if( s_id != "" )
				i_id = gui->GetIdByName( s_id );
			gui->Add( widget, i_id, left, top, width, height );
			widget->ProcessGuiSkinEvent(INIT_WIDGET);
			if( init != "" )
			{
				CallbackHandler c_init( init, widget );
				c_init.Call();
			};
			widget->setEnabled( ! disabled );

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

CGuiSkinnedLayout * MainLayout = NULL;
int		Menu_CGuiSkinInitialize(void)
{
	DrawRectFill(tMenu->bmpBuffer, 0, 0, 640-1, 480-1, tLX->clBlack);
	//DrawRectFill(tMenu->bmpBuffer, 0, 0, 640-1, 480-1, tLX->clBlack);
	//Menu_DrawBox(tMenu->bmpBuffer, 0, 0, 640-1, 480-1,);
	//DrawImage(tMenu->bmpBuffer,tMenu->bmpMainBack_common,0,0);
	//CGuiSkin::InitLayouts();
	SetGameCursor(CURSOR_ARROW);
	tMenu->iMenuType = MNU_GUISKIN;
	//DrawImage(tMenu->bmpScreen, tMenu->bmpBuffer, 0, 0);	// TODO: hacky hacky
	MainLayout = CGuiSkin::GetLayout( "main" );
	if( MainLayout == NULL )
	{
		Menu_CGuiSkinShutdown();
		Menu_MainInitialize();
		return false;
	};
	return true;
};

void	Menu_CGuiSkinFrame(void)
{
	if( ! MainLayout->Process() )
	{
		Menu_CGuiSkinShutdown();
		Menu_MainInitialize();
		return;
	};
	if( MainLayout == NULL )
		return;
	MainLayout->Draw(tMenu->bmpBuffer);
	DrawCursor(tMenu->bmpBuffer);
	DrawImage(tMenu->bmpScreen, tMenu->bmpBuffer, 0, 0);	// TODO: hacky hacky, high CPU load
};

void	Menu_CGuiSkinShutdown(void)
{
	if( MainLayout == NULL )
		return;
	DrawRectFill(tMenu->bmpBuffer, 0, 0, 640-1, 480-1, tLX->clBlack);
	DrawImage(tMenu->bmpScreen, tMenu->bmpBuffer, 0, 0);
	SetGameCursor(CURSOR_NONE);
	MainLayout = NULL;
	CGuiSkin::ClearLayouts();
};

void CGuiSkin::CallbackHandler::Init( const std::string & s1, CWidget * source )
{
	// TODO: put LUA handler here, this handmade string parser works but the code is ugly
	//printf( "CGuiSkin::CallbackHandler::Init(\"%s\"): ", s1.c_str() );
	m_source = source;
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
		
		std::map< std::string, SkinVarPtr_t > :: iterator it;
		for( it = CGuiSkin::Vars().begin();
				it != CGuiSkin::Vars().end(); it++ )
		{
			if( !stringcasecmp( it->first, func ) && it->second.type == SVT_CALLBACK )
			{
				m_callbacks.push_back( std::pair< SkinCallback_t, std::string > ( it->second.c, param ) );
				//printf("%s(\"%s\") ", it->first.c_str(), param.c_str());
				break;
			};
		};
		if( it == CGuiSkin::Vars().end() )
		{
			printf("Cannot find GUI skin callback \"%s(%s)\"\n", func.c_str(), param.c_str());
		};
	};
	//printf("\n");
};

void CGuiSkin::CallbackHandler::Call()
{
	unsigned size = m_callbacks.size();	// Some callbacks may destroy *this, m_callbacks.size() call will crash
	for( unsigned f=0; f<size; f++ )	// I know that's hacky, oh well...
		m_callbacks[f].first( m_callbacks[f].second, m_source );	// Here *this may be destroyed
};

class CGuiSkin_Destroyer
{
	public:
	CGuiSkin_Destroyer() { };
	~CGuiSkin_Destroyer()
	{
		if( CGuiSkin::m_instance != NULL )
		{
			CGuiSkin::ClearLayouts();
			delete CGuiSkin::m_instance;
			CGuiSkin::m_instance = NULL;
		};
	};
}
CGuiSkin_Destroyer_instance;

void MakeSound( const std::string & param, CWidget * source )
{
	if( param == "" || param == "click" )
		PlaySoundSample(sfxGeneral.smpClick);
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

	class GUISkinAdder 
	{ 
		public:
	   	CCombobox* cb;
	   	int index;
		GUISkinAdder(CCombobox* cb_) : cb(cb_), index(1) {}
		inline bool operator() (std::string dir) 
		{
			size_t slash = findLastPathSep(dir);
			if(slash != std::string::npos)
				dir.erase(0, slash+1);

			if( dir == ".svn" )
				return true;

			cb->addItem(index, dir, dir);
			
			index++;
			return true;
		};
	};

std::string sSkinCombobox_OldSkinPath;

void SkinCombobox_Init( const std::string & param, CWidget * source )
{
	if( source->getType() != wid_Combobox )
		return;
	//CCombobox * cb = dynamic_cast< CCombobox * > (source);	// MSVC 6 build crashes on this!
	CCombobox * cb = ( CCombobox * ) (source);
	cb->setUnique(true);
	cb->clear();
	cb->addItem( 0, "", "None" );
	FindFiles(GUISkinAdder(cb), "data/frontend/skins", FM_DIR);
	cb->setCurSIndexItem( tLXOptions->sSkinPath );
};

void SkinCombobox_Change( const std::string & param, CWidget * source )
{
	if( source->getType() != wid_Combobox )
		return;
	if( tLXOptions->sSkinPath == "" && sSkinCombobox_OldSkinPath != "" )	// Go to non-skinned menu from skinned menu
	{
		Menu_CGuiSkinShutdown();
		Menu_MainInitialize();
	}
	else if( tLXOptions->sSkinPath != "" && sSkinCombobox_OldSkinPath == "" )	// Go to skinned menu from non-skinned menu
	{
		Menu_MainShutdown();
		Menu_CGuiSkinInitialize();
	}
	else if( tLXOptions->sSkinPath != "" && sSkinCombobox_OldSkinPath != tLXOptions->sSkinPath )	// Load another skin
	{
		Menu_CGuiSkinShutdown();
		Menu_MainInitialize();
		Menu_MainShutdown();
		Menu_CGuiSkinInitialize();
	};
	sSkinCombobox_OldSkinPath = tLXOptions->sSkinPath;
};

static bool bRegisteredCallbacks = CGuiSkin::RegisterVars("GUI")
	( & MakeSound, "MakeSound" )
	( & SkinCombobox_Init, "SkinCombobox_Init" )
	( & SkinCombobox_Change, "SkinCombobox_Change" );

