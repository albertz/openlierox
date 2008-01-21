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

#include <string>
#include <map>
#include <vector>
#include <list>

#include "Color.h"

class CWidget;
class CGuiLayoutBase;
class CGuiSkinnedLayout;

// Do not confuse this with game script - these vars mainly for GUI frontend and options
class CScriptableVars	// Singletone
{
public:

	typedef void ( * ScriptCallback_t ) ( const std::string & param, CWidget * source );
	typedef Uint32 Color_t;

	// What can we attach to GUI element - bool, int, float, string or function.
	enum ScriptVarType_t
	{
		SVT_BOOL,
		SVT_INT,
		SVT_FLOAT,
		SVT_STRING,
		SVT_COLOR,		// uint32 actually, used only in XML files
		SVT_CALLBACK	// Cannot be referenced from XML files directly, only as string
	};

	struct ScriptVarPtr_t
	{
		ScriptVarType_t type;
		// we use union to save some memory
		union
		{
			bool * b;	// Pointer to static var
			int * i;
			float * f;
			std::string * s;
			Color_t * cl;
			ScriptCallback_t cb;
		};
		union	// Is there any point in doing that union?
		{
			bool bdef;	// Default value for that var for config file loading / saving
			int idef;
			float fdef;
			const char * sdef;	// Not std::string to keep this structure plain-old-data
			Color_t cldef;
			// No default value for skin callback, 'cause it's not saved into cfg file
		};
		ScriptVarPtr_t(): type(SVT_CALLBACK) { b = NULL; }; // Invalid value initially (all pointer are NULL in union)
		ScriptVarPtr_t( bool * v, bool def = false ): type(SVT_BOOL) { b = v; bdef = def; };
		ScriptVarPtr_t( int * v, int def = 0 ): type(SVT_INT) { i = v; idef = def; };
		ScriptVarPtr_t( float * v, float def = 0.0 ): type(SVT_FLOAT) { f = v; fdef = def; };
		ScriptVarPtr_t( std::string * v, const char * def = "" ): type(SVT_STRING) { s = v; sdef = def; };
		ScriptVarPtr_t( Color_t * v, Color_t def = MakeColour(255,0,255) ): type(SVT_COLOR) { cl = v; cldef = def; };
		ScriptVarPtr_t( ScriptCallback_t v ): type(SVT_CALLBACK) { cb = v; };
	};

	struct ScriptVar_t
	{
		ScriptVarType_t type;
		// Cannot do union because of std::string
		bool b;
		int i;
		float f;
		std::string s;
		Color_t c;	// color
		// No callback here - we cannot assign callbacks to each other
		ScriptVar_t(): type(SVT_BOOL), b(false), i(0), f(0.0), s(""), c(0) { };
		ScriptVar_t( bool v ): type(SVT_BOOL), b(v), i(0), f(0.0), s(""), c(0) { };
		ScriptVar_t( int v ): type(SVT_INT), b(false), i(v), f(0.0), s(""), c(0) { };
		ScriptVar_t( float v ): type(SVT_FLOAT), b(false), i(0), f(v), s(""), c(0) { };
		ScriptVar_t( const std::string & v ): type(SVT_STRING), b(false), i(0), f(0.0), s(v), c(0) { };
		ScriptVar_t( const char * v ): type(SVT_STRING), b(false), i(0), f(0.0), s(v), c(0) { };
		ScriptVar_t( Color_t v ): type(SVT_COLOR), b(false), i(0), f(0.0), s(""), c(v) { };
	};

	static CScriptableVars & Init();

	static std::map< std::string, ScriptVarPtr_t > & Vars()
	{
		Init();
		return m_instance->m_vars;
	};
	
	static ScriptVarPtr_t GetVar( const std::string & name );	// Case-insensitive search, returns NULL on fail
	static ScriptVarPtr_t GetVar( const std::string & name, ScriptVarType_t type );	// Case-insensitive search, returns NULL on fail
	static std::string DumpVars();	// For debug output
	
	static void SetVarByString(const CScriptableVars::ScriptVarPtr_t& var, const std::string& str);

	// Stolen from boost::program_options

	// Convert "tLXOptions -> iNetworkPort " to "iNetworkPort"
	static std::string StripClassName( const std::string & c );

	class VarRegisterHelper
	{
		friend class CScriptableVars;

		std::map< std::string, ScriptVarPtr_t > & m_vars;	// Reference to CScriptableVars::m_vars
		std::string m_prefix;

		VarRegisterHelper( CScriptableVars * parent, const std::string & prefix ): 
			m_vars( parent->m_vars ), m_prefix(prefix) {};

		std::string Name( const std::string & c )
		{
			if( m_prefix != "" ) 
				return m_prefix + "." + c;
			return c;
		};

		public:

		operator bool () { return true; };	// To be able to write static expressions

		VarRegisterHelper & operator() ( bool & v, const std::string & c, bool def = false )
			{ m_vars[Name(c)] = ScriptVarPtr_t( &v, def ); return *this; };

		VarRegisterHelper & operator() ( int & v, const std::string & c, int def = 0 )
			{ m_vars[Name(c)] = ScriptVarPtr_t( &v, def ); return *this; };

		VarRegisterHelper & operator() ( float & v, const std::string & c, float def = 0.0 )
			{ m_vars[Name(c)] = ScriptVarPtr_t( &v, def ); return *this; };

		VarRegisterHelper & operator() ( std::string & v, const std::string & c, const char * def = "" )
			{ m_vars[Name(c)] = ScriptVarPtr_t( &v, def ); return *this; };

		VarRegisterHelper & operator() ( Color_t & v, const std::string & c, Color_t def = MakeColour(255,0,255) )
			{ m_vars[Name(c)] = ScriptVarPtr_t( &v, def ); return *this; };

		VarRegisterHelper & operator() ( ScriptCallback_t v, const std::string & c )
			{ m_vars[Name(c)] = ScriptVarPtr_t( v ); return *this; };
	};

	static VarRegisterHelper RegisterVars( const std::string & base = "" )
	{
		Init();
		return VarRegisterHelper( m_instance, base );
	};

private:
	friend class VarRegisterHelper;
	friend class CGuiSkin_Destroyer;	// Deletes CGuiSkin instance on exit

	static CScriptableVars * m_instance;
	std::map< std::string, ScriptVarPtr_t > m_vars;	// All in-game variables and callbacks
};


class CGuiSkin	// Singletone
{
public:

	static CGuiSkin & Init();

	static CGuiSkinnedLayout * GetLayout( const std::string & filename );	// Get GUI layout from cache or create it from disk
	static void ClearLayouts();
	
	typedef std::vector< std::pair< std::string, CScriptableVars::ScriptVarType_t > > paramListVector_t;
	// WidgetCreator will create widget and add it to specified CGuiLayout (and init it a bit after that if necessary).
	typedef CWidget * ( * WidgetCreator_t ) ( const std::vector< CScriptableVars::ScriptVar_t > & params, CGuiLayoutBase * layout, int id, int x, int y, int w, int h );
	
	class WidgetRegisterHelper
	{
		friend class CGuiSkin;

		paramListVector_t & m_params;	// Reference to CGuiSkin.m_vars

		WidgetRegisterHelper( paramListVector_t & params ): 
			m_params( params ) {};
		
		public:
		
		operator bool () { return true; };	// To be able to write static expressions

		WidgetRegisterHelper & operator() ( const std::string & c, CScriptableVars::ScriptVarType_t vt )
			{ m_params.push_back( std::pair< std::string, CScriptableVars::ScriptVarType_t >( c, vt ) ); return *this; };
	};
	
	static WidgetRegisterHelper RegisterWidget( const std::string & name, WidgetCreator_t creator )
	{
		Init();
		m_instance->m_widgets[name] = std::pair< paramListVector_t, WidgetCreator_t > ( paramListVector_t(), creator );
		return WidgetRegisterHelper( m_instance->m_widgets[name].first );
	};

	static std::string DumpWidgets();	// For debug output
	
	// Helper class for callback info thst should be inserted into widget
	class CallbackHandler
	{
		std::vector< std::pair< CScriptableVars::ScriptCallback_t, std::string > > m_callbacks;
		CWidget * m_source;
	
	public:
		void Init( const std::string & param, CWidget * source );
		void Call();
		CallbackHandler(): m_source(NULL) { };
		CallbackHandler( const std::string & param, CWidget * source ) { Init( param, source ); };
	};
	
	// Update will be called on each frame with following params
	static void RegisterUpdateCallback( CScriptableVars::ScriptCallback_t update, const std::string & param, CWidget * source );
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
	friend class WidgetRegisterHelper;
	friend class CGuiSkin_Destroyer;	// Deletes CGuiSkin instance on exit

	// Should be private, please use CGuiSkin::Vars() and other member functions to have access to them
	static CGuiSkin * m_instance;
	std::map< std::string, CGuiSkinnedLayout * > m_guis;	// All loaded in-game GUI layouts
	std::map< std::string, std::pair< paramListVector_t, WidgetCreator_t > > m_widgets;	// All widget classes
	struct UpdateList_t
	{
		UpdateList_t( CWidget * s, CScriptableVars::ScriptCallback_t u, const std::string & p ): 
			source( s ), update( u ), param( p ) { };
		CWidget * source;
		CScriptableVars::ScriptCallback_t update;
		std::string param;
	};
	std::list< UpdateList_t > m_updateCallbacks;
};

#endif
