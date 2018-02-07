/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Local menu
// Created 30/6/02
// Jason Boettcher


#include <assert.h>
#include <string>

#include "LieroX.h"
#include "CGameScript.h"
#include "AuxLib.h"
#include "DeprecatedGUI/Graphics.h"
#include "CClient.h"
#include "CServer.h"
#include "DeprecatedGUI/Menu.h"
#include "DeprecatedGUI/CListview.h"
#include "GfxPrimitives.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "DeprecatedGUI/CButton.h"
#include "DeprecatedGUI/CLabel.h"
#include "DeprecatedGUI/CImage.h"
#include "DeprecatedGUI/CTextbox.h"
#include "DeprecatedGUI/CSlider.h"
#include "DeprecatedGUI/CCheckbox.h"
#include "DeprecatedGUI/CTextButton.h"
#include "DeprecatedGUI/CListview.h"
#include "DeprecatedGUI/CGuiSkinnedLayout.h"
#include "DeprecatedGUI/CBrowser.h"
#include "Sounds.h"
#include "ProfileSystem.h"
#include "FeatureList.h"
#include "Options.h"
#include "CGameMode.h"


namespace DeprecatedGUI {


/*
=======================

	 Game Settings

 For both local & host

=======================
*/

//short			GameTabPane = 0;
CGuiLayout		cGameSettings;
//CGuiLayout		cGeneralSettings;
//CGuiLayout		cBonusSettings;

// Game Settings
enum {
	gs_Ok,
	gs_Default,
	gs_AdvancedLevel,
	gs_AdvancedLevelLabel,
	
	gs_FeaturesList,
	gs_FeaturesListLabel,
	
};

static void initFeaturesList(CListview* l);

///////////////////
// Initialize the game settings
void Menu_GameSettings()
{
	//GameTabPane = 0;
	// Setup the buffer
	Menu_DrawBox(tMenu->bmpBuffer.get(), 80,120, 560,460);
	DrawRectFillA(tMenu->bmpBuffer.get(), 82,122, 558,458, tLX->clDialogBackground, 245);

	Menu_RedrawMouse(true);


	// Shutdowns all 3 following instances
	Menu_GameSettingsShutdown();

	cGameSettings.Initialize();
	//cGeneralSettings.Initialize();

	// Keep text, it's the window text - the rest you can easily figure out by yourself.
	cGameSettings.Add( new CLabel("Game Settings", tLX->clNormalLabel),		    -1,	        280,135, 0, 0);

	// Game settings, stuff on each pane.

	cGameSettings.Add( new CButton(BUT_OK, DeprecatedGUI::tMenu->bmpButtons),	    gs_Ok,      180,435, 40,15);
    cGameSettings.Add( new CButton(BUT_DEFAULT, DeprecatedGUI::tMenu->bmpButtons), gs_Default, 390,435, 80,15);

	cGameSettings.Add( new CSlider(__AdvancedLevelType_Count - 1, 0, tLXOptions->iAdvancedLevelLimit), gs_AdvancedLevel, 365, 155, 80,15);
	cGameSettings.Add( new CLabel("Detail Level:", tLX->clNormalLabel), -1, 285, 155, 70, 15);
	float warningCoeff = CLAMP((float)tLXOptions->iAdvancedLevelLimit / (__AdvancedLevelType_Count - 1), 0.0f, 1.0f);
	cGameSettings.Add( new CLabel(AdvancedLevelShortDescription((AdvancedLevel)tLXOptions->iAdvancedLevelLimit), tLX->clNormalLabel * (1.0f - warningCoeff) + tLX->clError * warningCoeff), gs_AdvancedLevelLabel, 450, 155, 70, 15);

	CListview* features = new CListview();
	cGameSettings.Add( features, gs_FeaturesList, 95, 170, 450, 205);

	features->setDrawBorder(true);
	features->setRedrawMenu(false);
	features->setShowSelect(false);
	features->setOldStyle(true);
	features->subItemsAreAligned() = true;
	features->setMouseOverEventEnabled(true);
	
	int maxWidth = 0; // Width of the widest item in this column + some space
	CScriptableVars::const_iterator upper_bound = CScriptableVars::upper_bound("GameOptions.");
	for( CScriptableVars::const_iterator it = CScriptableVars::lower_bound("GameOptions."); it != upper_bound; it++ ) 
	{
		if( tLX->cFont.GetWidth(it->second.shortDesc) > maxWidth )
			maxWidth = tLX->cFont.GetWidth(it->second.shortDesc);
	}
	
	features->AddColumn("", maxWidth + 10); 
	features->AddColumn("", 190); 
	
	initFeaturesList(features);

	cGameSettings.Add( new CLabel("", tLX->clNormalLabel), gs_FeaturesListLabel, 95, 390, 450, 40);
}

// Features listview

static void addFeautureListGroupHeading(CListview* l, GameInfoGroup group) {
	if( l->getItemCount() > 0 )
		l->AddItem("", l->getNumItems(), tLX->clNormalLabel); // Empty line
	l->AddItem(GameInfoGroupDescriptions[group][0], l->getNumItems(), tLX->clHeading);
	l->AddSubitem(LVS_TEXT, std::string("--- ") + GameInfoGroupDescriptions[group][0] + " ---" + 
				  (tLXOptions->iGameInfoGroupsShown[group] ? " [-]" : " [+]"), (DynDrawIntf*)NULL, NULL);	
}

static int getListItemGroupInfoNr(const std::string& sindex) {
	for( int group = 0; group < GIG_Size; group++ )
		if( sindex == GameInfoGroupDescriptions[group][0] )
			return group;
	return -1;
}
	
	
static void updateFeatureListItemColor(lv_item_t* item) {
	lv_subitem_t* sub = item->tSubitems; if(!sub) return;
	if(((CListview*)cGameSettings.getWidget(gs_FeaturesList))->getMouseOverSIndex() == item->sIndex) {
		sub->iColour = tLX->clMouseOver;
		return;
	}
	
	int group = getListItemGroupInfoNr(item->sIndex);
	if(group >= 0) sub->iColour = tLX->clHeading;
	else {
		// Note: commented out because I am not sure if it is that nice
		//RegisteredVar* var = CScriptableVars::GetVar(item->sIndex);
		float warningCoeff = 0.0f; //CLAMP((float)var->advancedLevel / (__AdvancedLevelType_Count - 1), 0.0f, 1.0f);
		sub->iColour = tLX->clNormalLabel * (1.0f - warningCoeff) + tLX->clError * warningCoeff;
	}
}


static void initFeaturesList(CListview* l)
{
	l->Clear();
	for( GameInfoGroup group = (GameInfoGroup)0; group < GIG_Size; group = (GameInfoGroup)(group + 1) )
	{
		if( group == GIG_GameModeSpecific_Start )
			continue;
		if( group > GIG_GameModeSpecific_Start && 
			tLXOptions->tGameInfo.gameMode->getGameInfoGroupInOptions() != group )
			continue;

		size_t countGroupOpts = 0;
		CScriptableVars::const_iterator upper_bound = CScriptableVars::upper_bound("GameOptions.");
		for( CScriptableVars::const_iterator it = CScriptableVars::lower_bound("GameOptions."); it != upper_bound; it++ ) 
		{
			if( it->second.group != group ) continue;
			if( (int)it->second.advancedLevel > tLXOptions->iAdvancedLevelLimit ) continue;
			
			if( it->second.var.s == &tLXOptions->tGameInfo.sModDir || 
				it->second.var.s == &tLXOptions->tGameInfo.sMapFile ||
				it->first == "GameOptions.GameInfo.GameType" ||
				it->second.var.i == &tLXOptions->tGameInfo.iMaxPlayers )
				continue;	// We have nice comboboxes for them, skip them in the list
			
			if( tMenu && tMenu->iMenuType == MNU_LOCAL )
				if( it->second.var.b == &tLXOptions->tGameInfo.bAllowConnectDuringGame )
					continue;
			
			if( !tLXOptions->tGameInfo.gameMode || !tLXOptions->tGameInfo.gameMode->isTeamGame() ) {
				if( it->second.var.i == &tLXOptions->iRandomTeamForNewWorm ) continue;
				if( it->second.var.b == &tLXOptions->tGameInfo.bRespawnGroupTeams ) continue;
				if( it->second.var == &tLXOptions->tGameInfo.features[FT_TeamScoreLimit] ) continue;
				if( it->second.var == &tLXOptions->tGameInfo.features[FT_TeamkillDecreasesScore] ) continue;
				if( it->second.var == &tLXOptions->tGameInfo.features[FT_TeamInjure] ) continue;				
				if( it->second.var == &tLXOptions->tGameInfo.features[FT_TeamHit] ) continue;				
			}
			
			if(countGroupOpts == 0)
				addFeautureListGroupHeading(l, group);
			countGroupOpts++;

			if( ! tLXOptions->iGameInfoGroupsShown[group] )
				continue;
			
			lv_item_t * item = l->AddItem(it->first, l->getNumItems(), tLX->clNormalLabel);
			updateFeatureListItemColor(item);
			l->AddSubitem(LVS_TEXT, it->second.shortDesc, (DynDrawIntf*)NULL, NULL); 
			item->iHeight = 24; // So checkbox / textbox will fit okay

			if( it->second.var.type == SVT_BOOL )
			{
				CCheckbox * cb = new CCheckbox( * it->second.var.b );
				l->AddSubitem(LVS_WIDGET, "", (DynDrawIntf*)NULL, cb);
				cb->Create();
				cb->Setup(-1, 0, 0, 20, 20);
			}
			else
			{
				int textboxSize = 228;
				if( it->second.haveMinMax() )
				{
					int imin=0, imax=0;
					float fScale = 1.0f;
					int iVal = 0;
					if( it->second.var.type == SVT_FLOAT )
					{
						// Adding some small number to round it up correctly
						imin = int( float(it->second.min) *10.0f + 0.00001f );	// Scale them up
						imax = int( float(it->second.max) *10.0f + 0.00001f );
						iVal = int( (*it->second.var.f) * 10.0f + 0.00001f );
						fScale = 0.1f;
					} else {
						imin = it->second.min;
						imax = it->second.max;
						iVal = * it->second.var.i;
					}
					CSlider * sld = new CSlider( imax, imin, imin, false, 190, 0, tLX->clNormalLabel, fScale );
					CLAMP_DIRECT(iVal, sld->getMin(), sld->getMax() );
					sld->setValue(iVal);
					l->AddSubitem(LVS_WIDGET, "", (DynDrawIntf*)NULL, sld);
					sld->Create();
					sld->Setup(-1, 0, 0, 180, tLX->cFont.GetHeight());
					textboxSize = 40;
				}
				CTextbox * txt = new CTextbox();
				l->AddSubitem(LVS_WIDGET, "", (DynDrawIntf*)NULL, txt);
				txt->Create();
				txt->Setup(-1, 0, 0, textboxSize, tLX->cFont.GetHeight());
				if ((it->second.var.type == SVT_INT && it->second.var.isUnsigned && *it->second.var.i < 0) ||
					(it->second.var.type == SVT_FLOAT && it->second.var.isUnsigned && *it->second.var.f < 0))
					txt->setText("");  // Leave blank for infinite values
				else
					txt->setText( it->second.var.toString() );
			}
		}
	}
}


// Copy values from listview to features list
static void updateFeaturesList(CListview* l) 
{
	CScriptableVars::const_iterator upper_bound = CScriptableVars::upper_bound("GameOptions.");
	for( CScriptableVars::const_iterator it = CScriptableVars::lower_bound("GameOptions."); it != upper_bound; it++ ) 
	{
		if( it->second.group == GIG_Invalid ) continue;

		lv_item_t * item = l->getItem(it->first);
		if( ! item )
			continue;
		lv_subitem_t * si = item->tSubitems->tNext;
		if( ! si )
			continue;
		CWidget * w = si->tWidget;
		if( ! w )
			continue;
			
		if( it->second.var.type == SVT_BOOL )
		{
			if( w->getType() == wid_Checkbox )
			{
				* it->second.var.b = ((CCheckbox *)w)->getValue();
			}
		}
		else
		{
			if( w->getType() == wid_Textbox )
			{
				it->second.var.fromString( ((CTextbox *)w)->getText() );
			}
			if( w->getType() == wid_Slider && 
				si->tNext && si->tNext->tWidget && si->tNext->tWidget->getType() == wid_Textbox &&
				l->getWidgetEvent() && l->getWidgetEvent()->cWidget )
			{
				CSlider *slider = (CSlider *)w;
				CTextbox *textBox = (CTextbox *)si->tNext->tWidget;
					
				if( l->getWidgetEvent()->cWidget->getType() == wid_Slider ) // User moved slider - update textbox
				{
					int iVal = slider->getValue();
					if( it->second.var.type == SVT_INT )
					{
						* it->second.var.i = iVal;
						textBox->setText(itoa(iVal));
						if( it->second.var.isUnsigned && iVal < 0 )
							textBox->setText("");
					}
					if( it->second.var.type == SVT_FLOAT )
					{
						* it->second.var.f = iVal / 10.0f;
						textBox->setText(to_string<float>(iVal / 10.0f));
						if( it->second.var.isUnsigned && iVal < 0 )
							textBox->setText("");
					}
				}
				if( l->getWidgetEvent()->cWidget->getType() == wid_Textbox ) // User typed in textbox - update slider
				{
					it->second.var.fromString(textBox->getText());
					int iVal = 0;
					if( it->second.var.type == SVT_INT )
					{
						// Do not do min/max check on typed value, it's sole user responsibility if game crashes (though it should not)
						//CLAMP_DIRECT(* it->second.var.i, it->second.min.i, it->second.max.i );
						iVal = * it->second.var.i;
					}
					if( it->second.var.type == SVT_FLOAT )
					{
						// Do not do min/max check on typed value, it's sole user responsibility if game crashes (though it should not)
						//CLAMP_DIRECT(*it->second.var.f, it->second.min.f, it->second.max.f );
						iVal = int(* it->second.var.f * 10.0f);
					}
					CLAMP_DIRECT(iVal, slider->getMin(), slider->getMax() );
					slider->setValue(iVal);
				}
			}
		}
	}
	if( tLXOptions->tGameInfo.iLives < 0 )
		tLXOptions->tGameInfo.iLives = WRM_UNLIM;
}

/////////////
// Shutdown
void Menu_GameSettingsShutdown()
{
	cGameSettings.Shutdown();
}



///////////////////
// Game settings frame
// Returns whether of not we have finised with the game settings
bool Menu_GameSettings_Frame()
{
	gui_event_t *ev = NULL;

	ev = cGameSettings.Process();

	if(ev)
	{

		switch(ev->iControlID)
		{

			// OK, done
			case gs_Ok:
				if(ev->iEventMsg == BTN_CLICKED)
				{
					Menu_GameSettings_GrabInfo();
					Menu_GameSettingsShutdown();

					return true;
				}
				break;

            // Set the default values
            case gs_Default:
                if( ev->iEventMsg == BTN_CLICKED ) {
                    Menu_GameSettings_Default();
                }
                break;

			case gs_AdvancedLevel:
				if( ev->iEventMsg == SLD_CHANGE ) {
					tLXOptions->iAdvancedLevelLimit = ((CSlider*)cGameSettings.getWidget(gs_AdvancedLevel))->getValue();
					CListview* features = (CListview*)cGameSettings.getWidget(gs_FeaturesList);
					features->SaveScrollbarPos();
					initFeaturesList(features);
					features->RestoreScrollbarPos();

					CLabel* featuresLabel = (CLabel*)cGameSettings.getWidget(gs_FeaturesListLabel);
					float warningCoeff = CLAMP((float)tLXOptions->iAdvancedLevelLimit / (__AdvancedLevelType_Count - 1), 0.0f, 1.0f);
					featuresLabel->ChangeColour( tLX->clNormalLabel * (1.0f - warningCoeff) + tLX->clError * warningCoeff );
					featuresLabel->setText( splitStringWithNewLine(AdvancedLevelDescription((AdvancedLevel)tLXOptions->iAdvancedLevelLimit), (size_t)-1, 450, tLX->cFont) );		
					
					CLabel* advancenessLabel = (CLabel*)cGameSettings.getWidget(gs_AdvancedLevelLabel);
					advancenessLabel->setText( AdvancedLevelShortDescription((AdvancedLevel)tLXOptions->iAdvancedLevelLimit) );
					advancenessLabel->ChangeColour( tLX->clNormalLabel * (1.0f - warningCoeff) + tLX->clError * warningCoeff );
				}
				break;
				
			case gs_FeaturesList:
				CListview* features = (CListview*)ev->cWidget;
				if( ev->iEventMsg == LV_WIDGETEVENT )
				{
					updateFeaturesList(features);
				}
				if( ev->iEventMsg == LV_MOUSEOVER )
				{
					CLabel* featuresLabel = (CLabel*)cGameSettings.getWidget(gs_FeaturesListLabel);
					featuresLabel->ChangeColour( tLX->clNormalLabel );
					featuresLabel->setText( "" );
					for(lv_item_t* item = features->getItems(); item != NULL; item = item->tNext)
						updateFeatureListItemColor(item);
					if(	features->getMouseOverSIndex() != "" )
					{
						{
							lv_item_t* item = features->getItem(features->getMouseOverSIndex());
							if(item && item->tSubitems)
								item->tSubitems->iColour = tLX->clMouseOver;
						}
						std::string desc;
						{
							int group = getListItemGroupInfoNr(features->getMouseOverSIndex());
							if(group >= 0)
								desc = GameInfoGroupDescriptions[group][1];
						}
						if(desc == "") {
							RegisteredVar* var = CScriptableVars::GetVar( features->getMouseOverSIndex() );
							if(var) {
								desc = var->longDesc;
								float warningCoeff = CLAMP((float)var->advancedLevel / (__AdvancedLevelType_Count - 1), 0.0f, 1.0f);
								featuresLabel->ChangeColour( tLX->clNormalLabel * (1.0f - warningCoeff) + tLX->clError * warningCoeff );
							}
						}
						featuresLabel->setText( splitStringWithNewLine(desc, (size_t)-1, 450, tLX->cFont) );
					}
				}
				if( ev->iEventMsg == LV_CHANGED )
				{
					for( int group = 0; group < GIG_Size; group++ )
						if( features->getMouseOverSIndex() == GameInfoGroupDescriptions[group][0] ) {
							tLXOptions->iGameInfoGroupsShown[group] = ! tLXOptions->iGameInfoGroupsShown[group];
							features->SaveScrollbarPos();
							initFeaturesList(features);
							features->RestoreScrollbarPos();
							break;
						}
				}
				break;
		}

	}

	DrawImageAdv(VideoPostProcessor::videoSurface(), tMenu->bmpBuffer, 120,150, 120,150, 400,300);
	cGameSettings.Draw(VideoPostProcessor::videoSurface());

	// Draw the mouse
	DrawCursor(VideoPostProcessor::videoSurface());

	return false;
}


///////////////////
// Grab the game settings info
void Menu_GameSettings_GrabInfo()
{
	// Stub
}


///////////////////
// Set the default game settings info
void Menu_GameSettings_Default()
{
	CScriptableVars::const_iterator upper_bound = CScriptableVars::upper_bound("GameOptions.");
	for( CScriptableVars::const_iterator it = CScriptableVars::lower_bound("GameOptions."); it != upper_bound; it++ ) 
	{
		if( it->second.group == GIG_Invalid ) continue;
		
		if( it->first == "GameOptions.GameInfo.ModName" || 
			it->first == "GameOptions.GameInfo.LevelName" ||
			it->first == "GameOptions.GameInfo.GameType" )
			continue;	// We have nice comboboxes for them, skip them in the list

		it->second.var.setDefault();
    }

    CListview * features = (CListview *)cGameSettings.getWidget(gs_FeaturesList);
    features->Clear();
	initFeaturesList(features);
    
}

}; // namespace DeprecatedGUI
