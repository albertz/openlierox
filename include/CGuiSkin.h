/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#ifndef __CGUISKIN_H__
#define __CGUISKIN_H__

#include "GfxPrimitives.h"
#include <string>
#include <map>
#include <vector>
#include <list>

class CWidget;
class CGuiLayoutBase;
class CGuiSkinnedLayout;

class CGuiSkin	// Singletone
{
public:

	// WORST FIX EVER. Pelya, please, fix your code. Also, COMPILE your code before uploading.
	// One thing to have windows-only errors like your VT_ thingies
	// But having restriction issues is not acceptable
	// it should be used for iterators, not i, atleast it's used so far, might aswell make it standard
	// --- Thanks for the input, SteelSide, there was enough to say "pelya go fix your code ****ing n**b" :-P

	// What can we attach to GUI element - bool, int, float, string or function.
	enum SkinVarType_t
	{
		SVT_BOOL,
		SVT_INT,
		SVT_FLOAT,
		SVT_STRING,
		SVT_CALLBACK
	};

	typedef void ( * SkinCallback_t ) ( const std::string & param, CWidget * source );
	// param is ignored by now by all callbacks, but maybe in future it will be used

	struct SkinVarPtr_t
	{
		SkinVarType_t type;
		// TODO: this is not possible to be an union. sizeof(SkinCallback_t) != sizeof(void*) in general
		union	// Is there any point in doing that union?
		{
			bool * b;	// Pointer to static var
			int * i;
			float * f;
			std::string * s;
			SkinCallback_t c;
		};
		union	// Is there any point in doing that union?
		{
			bool bdef;	// Default value for that var for config file loading / saving
			int idef;
			float fdef;
			const char * sdef;	// Not std::string to keep this structure plain-old-data
			// No default value for skin callback, 'cause it's not saved into cfg file
		};
		SkinVarPtr_t() {};
		SkinVarPtr_t( bool * v, bool def = false ): type(SVT_BOOL) { b = v; bdef = def; };
		SkinVarPtr_t( int * v, int def = 0 ): type(SVT_INT) { i = v; idef = def; };
		SkinVarPtr_t( float * v, float def = 0.0 ): type(SVT_FLOAT) { f = v; fdef = def; };
		SkinVarPtr_t( std::string * v, const char * def = "" ): type(SVT_STRING) { s = v; sdef = def; };
		SkinVarPtr_t( SkinCallback_t v ): type(SVT_CALLBACK) { c = v; };
	};

	static CGuiSkin & Init()
	{
		if( m_instance == NULL )
			m_instance = new CGuiSkin;
		return *m_instance;
	};

	static std::map< std::string, SkinVarPtr_t > & Vars()
	{
		Init();
		return m_instance->m_vars;
	};
	
	static SkinVarPtr_t GetVar( const std::string & name, SkinVarType_t type );	// Case-insensitive search

	// Stolen from boost::program_options

	// Convert "tLXOptions -> iNetworkPort " to "iNetworkPort"
	static std::string StripClassName( const std::string & c );

	/*
	// Not used anywhere, don't want to make another #define
	#define VARSTR( V ) ( V, CGuiSkin::StripClassName( STRINGIZE( V ) ) )
	#define STRINGIZE( V ) STRINGIZE1( V )
	#define STRINGIZE1( V ) #V
	*/
	
	class CGuiSkin_RegisterVarDarkMagic
	{
		friend class CGuiSkin;

		std::map< std::string, SkinVarPtr_t > & m_vars;	// Reference to CGuiSkin.m_vars
		std::string m_prefix;

		CGuiSkin_RegisterVarDarkMagic( CGuiSkin * parent, const std::string & prefix ): 
			m_vars( parent->m_vars ), m_prefix(prefix) {};

		std::string Name( const std::string & c )
		{
			if( m_prefix != "" ) 
				return m_prefix + "." + c;
			return c;
		};

		public:

		operator bool () { return true; };	// To be able to write static expressions

		CGuiSkin_RegisterVarDarkMagic & operator() ( bool & v, const std::string & c, bool def = false )
			{ m_vars[Name(c)] = SkinVarPtr_t( &v, def ); return *this; };

		CGuiSkin_RegisterVarDarkMagic & operator() ( int & v, const std::string & c, int def = 0 )
			{ m_vars[Name(c)] = SkinVarPtr_t( &v, def ); return *this; };

		CGuiSkin_RegisterVarDarkMagic & operator() ( float & v, const std::string & c, float def = 0.0 )
			{ m_vars[Name(c)] = SkinVarPtr_t( &v, def ); return *this; };

		CGuiSkin_RegisterVarDarkMagic & operator() ( std::string & v, const std::string & c, const char * def = "" )
			{ m_vars[Name(c)] = SkinVarPtr_t( &v, def ); return *this; };

		CGuiSkin_RegisterVarDarkMagic & operator() ( SkinCallback_t v, const std::string & c )
			{ m_vars[Name(c)] = SkinVarPtr_t( v ); return *this; };
	};

	static CGuiSkin_RegisterVarDarkMagic RegisterVars( const std::string & base = "" )
	{
		Init();
		return CGuiSkin_RegisterVarDarkMagic( m_instance, base );
	};
	/* 
	Now we can write: 
	static bool VarsAddedJustOnce = 
		CGuiSkin::AddVars("Options") // Name of class or section
			( tLXOptions->iNetworkPort, "iNetworkPort" ) // Variable reference and name
			( tLXOptions->fUpdatePeriod, "fUpdatePeriod" ) 
			( tLXOptions->bUseIpToCountry, "bUseIpToCountry" ); 
	It doesn't hurt at all to add same var twice.
	*/
	static std::string DumpVars();	// For debug output
	
	static CGuiSkinnedLayout * GetLayout( const std::string & filename );	// Get GUI layout from cache or create it from disk
	static void ClearLayouts();
	
	// Registering widget types with CGuiSkin
	
	enum WidgetVarType_t	// Var types used to initialize widget in XML
	{
		WVT_BOOL,
		WVT_INT,
		WVT_FLOAT,
		WVT_STRING,
		WVT_COLOR
	};

	struct WidgetVar_t
	{
		WidgetVarType_t type;
		// Cannot do union because of std::string
		bool b;
		int i;
		float f;
		std::string s;
		Uint32 c;	// color
		WidgetVar_t(): type(WVT_BOOL), b(false), i(0), f(0.0), s(""), c(0) { };
		WidgetVar_t( bool v ): type(WVT_BOOL), b(v), i(0), f(0.0), s(""), c(0) { };
		WidgetVar_t( int v ): type(WVT_INT), b(false), i(v), f(0.0), s(""), c(0) { };
		WidgetVar_t( float v ): type(WVT_FLOAT), b(false), i(0), f(v), s(""), c(0) { };
		WidgetVar_t( const std::string & v ): type(WVT_STRING), b(false), i(0), f(0.0), s(v), c(0) { };
		WidgetVar_t( const char * v ): type(WVT_STRING), b(false), i(0), f(0.0), s(v), c(0) { };
		WidgetVar_t( Uint32 v ): type(WVT_COLOR), b(false), i(0), f(0.0), s(""), c(v) { };
	};
	
	typedef std::vector< std::pair< std::string, WidgetVarType_t > > paramListVector_t;
	// WidgetCreator will create widget and add it to specified CGuiLayout (and init it a bit after that if necessary).
	typedef CWidget * ( * WidgetCreator_t ) ( const std::vector< WidgetVar_t > & params, CGuiLayoutBase * layout, int id, int x, int y, int w, int h );
	
	class CGuiSkin_RegisterWidgetDarkMagic
	{
		friend class CGuiSkin;

		paramListVector_t & m_params;	// Reference to CGuiSkin.m_vars

		CGuiSkin_RegisterWidgetDarkMagic( paramListVector_t & params ): 
			m_params( params ) {};
		
		public:
		
		operator bool () { return true; };	// To be able to write static expressions

		CGuiSkin_RegisterWidgetDarkMagic & operator() ( const std::string & c, WidgetVarType_t vt )
			{ m_params.push_back( std::pair< std::string, WidgetVarType_t >( c, vt ) ); return *this; };
	};
	
	static CGuiSkin_RegisterWidgetDarkMagic RegisterWidget( const std::string & name, WidgetCreator_t creator )
	{
		Init();
		m_instance->m_widgets[name] = std::pair< paramListVector_t, WidgetCreator_t > ( paramListVector_t(), creator );
		return CGuiSkin_RegisterWidgetDarkMagic( m_instance->m_widgets[name].first );
	};

	static std::string DumpWidgets();	// For debug output
	
	// Helper class for callback info thst should be inserted into widget
	class CallbackHandler
	{
		std::vector< std::pair< SkinCallback_t, std::string > > m_callbacks;
		CWidget * m_source;
		public:
		
		void Init( const std::string & param, CWidget * source );
		void Call();
		CallbackHandler(): m_source(NULL) { };
		CallbackHandler( const std::string & param, CWidget * source ) { Init( param, source ); };
	};
	
	// Update will be called on each frame with following params
	static void RegisterUpdateCallback( SkinCallback_t update, const std::string & param, CWidget * source );
	// Remove widget from update list ( called from widget destructor )
	static void DeRegisterUpdateCallback( CWidget * source );
	// Called on each frame
	static void ProcessUpdateCallbacks();
	
	// INIT_WIDGET is special event for Widget::ProcessGuiSkinEvent() which is called 
	// after widget added to CGuiLayout - some widgets (textbox and combobox) require this.
	// SHOW_WIDGET is special event for Widget::ProcessGuiSkinEvent() which is called
	// when the dialog is shown - dialogs are cached into memory and may be shown or hidden.
	// No need for HIDE_WIDGET yet.
	enum { SHOW_WIDGET = -4 };	// Removed INIT_WIDGET because of more intelligent WidgetCreator_t
private:

	friend class CGuiSkin_RegisterVarDarkMagic;
	friend class CGuiSkin_RegisterWidgetDarkMagic;
	friend class CGuiSkin_Destroyer;	// Deletes CGuiSkin instance on exit

	// Should be private, please use CGuiSkin::Vars() and other member functions to have access to them
	static CGuiSkin * m_instance;
	std::map< std::string, SkinVarPtr_t > m_vars;	// All in-game variables and callbacks
	std::map< std::string, CGuiSkinnedLayout * > m_guis;	// All loaded in-game GUI layouts
	std::map< std::string, std::pair< paramListVector_t, WidgetCreator_t > > m_widgets;	// All widget classes
	struct UpdateList_t
	{
		UpdateList_t( CWidget * s, SkinCallback_t u, const std::string & p ): 
			source( s ), update( u ), param( p ) { };
		CWidget * source;
		SkinCallback_t update;
		std::string param;
	};
	std::list< UpdateList_t > m_updateCallbacks;
};

#endif
