/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////

#ifndef __CGUISKIN_H__DEPRECATED_GUI__
#define __CGUISKIN_H__DEPRECATED_GUI__

#include <string>
#include <map>
#include <vector>
#include <list>

#include "CScriptableVars.h"

namespace DeprecatedGUI {


class CWidget;
class CGuiLayoutBase;
class CGuiSkinnedLayout;


class CGuiSkin	// Singletone
{
public:

	static CGuiSkin & Init();	// Called automatically
	static void DeInit();	// Should be called from main()

	static CGuiSkinnedLayout * GetLayout( const std::string & filename );	// Get GUI layout from cache or create it from disk
	static void ClearLayouts();

	// has to be a vector because the order is important in the WidgetCreator
	typedef std::vector< std::pair< std::string, ScriptVarType_t > > paramListVector_t;
	// WidgetCreator will create widget and add it to specified CGuiLayout (and init it a bit after that if necessary).
	typedef CWidget * ( * WidgetCreator_t ) ( const std::vector< ScriptVar_t > & params, CGuiLayoutBase * layout, int id, int x, int y, int w, int h );

	// Allows registering params with daisy-chaining
	class WidgetRegisterHelper
	{
		friend class CGuiSkin;

		paramListVector_t & m_params;	// Reference to CGuiSkin.m_vars

		WidgetRegisterHelper( paramListVector_t & params ):
			m_params( params ) {};

		public:

		operator bool () { return true; };	// To be able to write static expressions

		WidgetRegisterHelper & operator() ( const std::string & c, ScriptVarType_t vt )
			{ m_params.push_back( std::pair< std::string, ScriptVarType_t >(c, vt) ); return *this; };
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
		std::vector< std::pair< ScriptCallback_t, std::string > > m_callbacks;
		CWidget * m_source;

	public:
		void Init( const std::string & param, CWidget * source );
		void Call();
		CallbackHandler(): m_source(NULL) { };
		CallbackHandler( const std::string & param, CWidget * source ) { Init( param, source ); };
	};

	// Update will be called on each frame with following params
	static void RegisterUpdateCallback( ScriptCallback_t update, const std::string & param, CWidget * source );
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

	// Should be private, please use CGuiSkin::Vars() and other member functions to have access to them
	static CGuiSkin * m_instance;
	std::map< std::string, CGuiSkinnedLayout * > m_guis;	// All loaded in-game GUI layouts
	std::map< std::string, std::pair< paramListVector_t, WidgetCreator_t > > m_widgets;	// All widget classes
	struct UpdateList_t
	{
		UpdateList_t( CWidget * s, ScriptCallback_t u, const std::string & p ):
			source( s ), update( u ), param( p ) { };
		CWidget * source;
		ScriptCallback_t update;
		std::string param;
	};
	std::list< UpdateList_t > m_updateCallbacks;
};

}; // namespace DeprecatedGUI

#endif
