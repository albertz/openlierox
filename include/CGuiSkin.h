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

#include "CGuiSkinnedLayout.h"
#include "CWidgetList.h"
#include <string>
#include <map>

// What can we attach to GUI element - bool, int, float, string or function.
enum SkinVarType_t
{
	SVT_BOOL,
	SVT_INT,
	SVT_FLOAT,
	SVT_STRING,
	SVT_CALLBACK
};

typedef void ( * SkinCallback_t ) ();

struct SkinVarPtr_t
{
	SkinVarType_t type;
	union	// Pointer to static var
	{
		bool * b;
		int * i;
		float * f;
		std::string * s;
		SkinCallback_t * c;
	};
	union	// Default value for that var for config file loading / saving
	{
		bool bdef;
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
	SkinVarPtr_t( SkinCallback_t * v ): type(SVT_CALLBACK) { c = v; };
};

class CGuiSkin	// Singletone
{
	
	
	std::map< std::string, CGuiSkinnedLayout * > m_guis;	// All loaded in-game GUI layouts

public:

	// WORST FIX EVER. Pelya, please, fix your code. Also, COMPILE your code before uploading.
	// One thing to have windows-only errors like your VT_ thingies
	// But having restriction issues is not acceptable
	// it should be used for iterators, not i, atleast it's used so far, might aswell make it standard
	static CGuiSkin * m_instance;
	std::map< std::string, SkinVarPtr_t > m_vars;	// All in-game variables and callbacks
	
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

	// Stolen from boost::program_options

	#define VARSTR( V ) ( V, CGuiSkin::StripClassName( STRINGIZE( V ) ) )
	#define STRINGIZE( V ) STRINGIZE1( V )
	#define STRINGIZE1( V ) #V

	// Convert "tLXOptions -> iNetworkPort " to "iNetworkPort"
	static std::string StripClassName( const std::string & c );

	class CGuiSkin_RegisterVarDarkMagic
	{
		std::map< std::string, SkinVarPtr_t > & m_vars;	// Reference to CGuiSkin.m_vars
		std::string m_prefix;

		CGuiSkin_RegisterVarDarkMagic( CGuiSkin * parent, const std::string & prefix ): 
			m_vars( parent->m_vars ), m_prefix(prefix) {};

		friend class CGuiSkin;

		std::string Name( const std::string & c )
		{
			if( m_prefix != "" ) 
				return m_prefix + "." + c;
			return c;
		};

		public:

		operator bool () { return true; };	// To be able to write static expressions

		CGuiSkin_RegisterVarDarkMagic operator() ( bool & v, const std::string & c, bool def = false )
			{ m_vars[Name(c)] = SkinVarPtr_t( &v, def ); return *this; };

		CGuiSkin_RegisterVarDarkMagic operator() ( int & v, const std::string & c, int def = 0 )
			{ m_vars[Name(c)] = SkinVarPtr_t( &v, def ); return *this; };

		CGuiSkin_RegisterVarDarkMagic operator() ( float & v, const std::string & c, float def = 0.0 )
			{ m_vars[Name(c)] = SkinVarPtr_t( &v, def ); return *this; };

		CGuiSkin_RegisterVarDarkMagic operator() ( std::string & v, const std::string & c, const char * def = "" )
			{ m_vars[Name(c)] = SkinVarPtr_t( &v, def ); return *this; };

		CGuiSkin_RegisterVarDarkMagic operator() ( SkinCallback_t & v, const std::string & c )
			{ m_vars[Name(c)] = SkinVarPtr_t( &v ); return *this; };
	};

	static CGuiSkin_RegisterVarDarkMagic AddVars( const std::string & base = "" )
	{
		Init();
		return CGuiSkin_RegisterVarDarkMagic( m_instance, base );
	};
	/* 
	Now we can write: 
	static bool VarsAddedJustOnce = 
		CGuiSkin::AddVars("Options") // Name of class or section
			( tLXOptions->iNetworkPort, "iNetworkPort" ) // Variable reference and name
			( tLXOptions->fUpdatePeriod, "tLXOptions->fUpdatePeriod" ) // Class name up to "->" auto-stripped
			VARSTR( tLXOptions->bUseIpToCountry ); // Defines to ( tLXOptions->bUseIpToCountry, "tLXOptions->bUseIpToCountry" )
	It doesn't hurt at all to add same var twice.
	*/
	static std::string DumpVars();	// For debug output
	
	static CGuiSkinnedLayout * GetLayout( const std::string & filename );	// Get GUI layout from cache or create it from disk
	static void ClearLayouts();	// Clears layouts cache so they are re-loaded from disk
};

#endif
