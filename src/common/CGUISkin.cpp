/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// GUI Skinning Functions
// Created 5/6/02
// Jason Boettcher
/*
#include "defs.h"
#include "LieroX.h"
#include "CGUISkin.h"
#include "Ini.h"

// Variables
	mainmenu_s			GUI_MainMenu;
	locallobby_s		GUI_LocalLobby;
 	internet_s			GUI_Internet;
 	lan_s				GUI_LAN;
 	host_s				GUI_Host;
 	hostlobby_s			GUI_HostLobby;
 	clientlobby_s		GUI_ClientLobby;
 	newplayer_s			GUI_NewPlayer;
 	viewplayers_s		GUI_ViewPlayers;
 	leveleditor_s		GUI_LevelEditor;
 	controlsoptions_s	GUI_ControlsOptions;
 	gameoptions_s		GUI_GameOptions;
 	systemoptions_s		GUI_SystemOptions;
 	connecting_s		GUI_Connecting;
 	connect_s			GUI_Connect;
 	ingame_s			GUI_InGame;

	// Dialogs
 	scoreboard_s		GUI_ScoreBoard;
 	gamemenu_s			GUI_GameMenu;
 	gameoverlocal_s		GUI_GameOverLocal;
 	gameovernet_s		GUI_GameOverNet;
 	viewport_s			GUI_Viewport;
 	console_s			GUI_Console;
 	gamesettings_s		GUI_GameSettings;
 	weaponoptions_s		GUI_WeaponOptions;
	banlistgui_s		GUI_BanList;
 	quitgame_s			GUI_QuitGame;
 	dialog_s			GUI_Dialog;

	// Widgets
 	checkbox_s			GUI_Checkbox;
 	combobox_s			GUI_Combobox;
 	inputbox_s			GUI_Inputbox;
 	listview_s			GUI_Listview;
 	popupmenu_s			GUI_Popupmenu;
 	scrollbar_s			GUI_Scrollbar;
 	slider_s			GUI_Slider;
 	textbox_s			GUI_Textbox;
 	main_s				GUI_Main;


 	bool				GUI_bMainMenuLoaded;
 	bool				GUI_bLocalLobbyLoaded;
 	bool				GUI_bInternetLoaded;
 	bool				GUI_bLanLoaded;
 	bool				GUI_bHostLoaded;
 	bool				GUI_bHostLobbyLoaded;
 	bool				GUI_bClientLobbyLoaded;
 	bool				GUI_bNewPlayerLoaded;
 	bool				GUI_bViewPlayersLoaded;
 	bool				GUI_bLevelEditorLoaded;
 	bool				GUI_bControlOptionsLoaded;
 	bool				GUI_bGameOptionsLoaded;
 	bool				GUI_bSystemOptionsLoaded;
 	bool				GUI_bInGameLoaded;
 	bool				GUI_bGameDialogsLoaded;
 	bool				GUI_bNonGameDialogsLoaded;
 	bool				GUI_bWidgetsLoaded;


char *GetIniStr(CIni *Ini,LPCTSTR Section,LPCTSTR Key, LPCTSTR Default = NULL)
{
 char *buf;
 DWORD buflen;
 Ini->GetStr(Section,Key,buf,buflen,Default);
 return buf;
}

Uint32 StringToColor(char *Text)
{
	if (strlen((char *) Text) != 7 || !Text)
		return 0;
	char *R;
	char *G;
	char *B;
	sprintf(R,"0x%s%s",Text[1],Text[2]);
	sprintf(G,"0x%s%s",Text[3],Text[4]);
	sprintf(B,"0x%s%s",Text[5],Text[6]);
	return MakeColour(atoi(R),atoi(G),atoi(B));
}

void FillProperties(CIni *Ini,properties_s *Properties,LPCTSTR Section)
{
 Properties->Left = Ini->GetInt(Section,"Left",0);
 Properties->Top = Ini->GetInt(Section,"Top",0);
 Properties->Width = Ini->GetInt(Section,"Width",0);
 Properties->Height = Ini->GetInt(Section,"Height",0);
 Properties->Visible = Ini->GetBool(Section,"Visible",0);
 Properties->BackColor = StringToColor(GetIniStr(Ini,Section,"BackColor"));
 Properties->Color = StringToColor(GetIniStr(Ini,Section,"Color"));
}

void FillFrame(CIni *Ini,frame_s *Properties,LPCTSTR Section)
{
 Properties->Left = Ini->GetInt(Section,"Left",0);
 Properties->Top = Ini->GetInt(Section,"Top",0);
 Properties->Width = Ini->GetInt(Section,"Width",0);
 Properties->Height = Ini->GetInt(Section,"Height",0);
 Properties->Round = Ini->GetBool(Section,"Round",false);
 Properties->Visible = Ini->GetBool(Section,"Visible",0);
 Properties->Thickness = Ini->GetInt(Section,"Thickness",2);
 Properties->DarkColor = StringToColor(GetIniStr(Ini,Section,"DarkColor"));
 Properties->LightColor = StringToColor(GetIniStr(Ini,Section,"LightColor"));
}

void FillAlign(CIni *Ini,textalign_e *Align,LPCTSTR Section)
{
 char *sAlign = GetIniStr(Ini,Section,"Align",NULL);
 if(!sAlign)
	 *Align = Center;  // default
 sAlign = strlwr(sAlign);
 if (sAlign == "left")
	 *Align = Left;
 else if (sAlign == "right")
	 *Align = Right;
 else if (sAlign == "center")
	 *Align = Center;
 else
	 *Align = Center;

}


void FillValign(CIni *Ini,textvalign_e *Valign,LPCTSTR Section)
{
 char *sValign = GetIniStr(Ini,Section,"Align",NULL);
 if(!sValign)
	 *Valign = Middle;  // default
 sValign = strlwr(sValign);
 if (sValign == "top")
	 *Valign = Top;
 else if (sValign == "middle")
	 *Valign = Middle;
 else if (sValign == "bottom")
	 *Valign = Bottom;
 else
	 *Valign = Middle;

}


bool GUI_LoadMainMenu(void)
{

 CIni Ini;
 char *SkinPath;
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/MainMenu.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_MainMenu.pBtnLocalPlay,"LocalPlayButton");
 FillProperties(&Ini,&GUI_MainMenu.pBtnNetPlay,"NetPlayButton");
 FillProperties(&Ini,&GUI_MainMenu.pBtnPlayerProfiles,"PlayerProfilesButton");
 FillProperties(&Ini,&GUI_MainMenu.pBtnLevelEditor,"LevelEditorButton");
 FillProperties(&Ini,&GUI_MainMenu.pBtnOptions,"OptionsButton");
 FillProperties(&Ini,&GUI_MainMenu.pBtnQuit,"QuitButton");
 FillProperties(&Ini,&GUI_MainMenu.pLXPLogo,"Logo");
 FillFrame(&Ini,&GUI_MainMenu.pFrame,"Frame");

 LOAD_IMAGE(GUI_MainMenu.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_MainMenu.pBtnLocalPlay.Visible)
   LOAD_IMAGE(GUI_MainMenu.tBtnLocalPlay,sumchar(SkinPath,GetIniStr(&Ini,"LocalPlayButton","Image",NULL)));
 if(GUI_MainMenu.pBtnNetPlay.Visible)
   LOAD_IMAGE(GUI_MainMenu.tBtnNetPlay,sumchar(SkinPath,GetIniStr(&Ini,"NetPlayButton","Image",NULL)));
 if(GUI_MainMenu.pBtnPlayerProfiles.Visible)
   LOAD_IMAGE(GUI_MainMenu.tBtnPlayerProfiles,sumchar(SkinPath,GetIniStr(&Ini,"PlayerProfilesButton","Image",NULL)));
 if(GUI_MainMenu.pBtnLevelEditor.Visible)
   LOAD_IMAGE(GUI_MainMenu.tBtnLevelEditor,sumchar(SkinPath,GetIniStr(&Ini,"LevelEditorButton","Image",NULL)));
 if(GUI_MainMenu.pBtnOptions.Visible)
   LOAD_IMAGE(GUI_MainMenu.tBtnOptions,sumchar(SkinPath,GetIniStr(&Ini,"OptionsButton","Image",NULL)));
 if(GUI_MainMenu.pBtnQuit.Visible)
   LOAD_IMAGE(GUI_MainMenu.tBtnQuit,sumchar(SkinPath,GetIniStr(&Ini,"QuitButton","Image",NULL)));
 if(GUI_MainMenu.pLXPLogo.Visible)
   LOAD_IMAGE(GUI_MainMenu.tLXPLogo,sumchar(SkinPath,GetIniStr(&Ini,"Logo","Image",NULL)));

 return true;
}


bool GUI_LoadLocalLobby(void)
{
 CIni Ini;
 char *SkinPath;
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/LocalLobby.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_LocalLobby.pBtnGameSettings,"GameSettingsButton");
 FillProperties(&Ini,&GUI_LocalLobby.pBtnWeaponOptions,"WeaponOptionsButton");
 FillProperties(&Ini,&GUI_LocalLobby.pBtnBack,"BackButton");
 FillProperties(&Ini,&GUI_LocalLobby.pBtnStart,"StartButton");
 FillProperties(&Ini,&GUI_LocalLobby.pCaption,"Caption");
 FillProperties(&Ini,&GUI_LocalLobby.pMiniMap,"MiniMap");
 FillProperties(&Ini,&GUI_LocalLobby.pLblMod,"ModLabel");
 FillProperties(&Ini,&GUI_LocalLobby.pLblLevel,"LevelLabel");
 FillProperties(&Ini,&GUI_LocalLobby.pLblGameType,"GameTypeLabel");
 FillProperties(&Ini,&GUI_LocalLobby.pCbbMod,"ModCombobox");
 FillProperties(&Ini,&GUI_LocalLobby.pCbbLevel,"LevelCombobox");
 FillProperties(&Ini,&GUI_LocalLobby.pCbbGameType,"GameTypeCombobox");
 FillProperties(&Ini,&GUI_LocalLobby.pPlayers,"PlayersList");
 FillProperties(&Ini,&GUI_LocalLobby.pPlaying,"PlayingList");
 FillFrame(&Ini,&GUI_LocalLobby.pFrame,"Frame");

 LOAD_IMAGE(GUI_LocalLobby.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_LocalLobby.pBtnGameSettings.Visible)
   LOAD_IMAGE(GUI_LocalLobby.tBtnGameSettings,sumchar(SkinPath,GetIniStr(&Ini,"GameSettingsButton","Image",NULL)));
 if(GUI_LocalLobby.pBtnWeaponOptions.Visible)
   LOAD_IMAGE(GUI_LocalLobby.tBtnWeaponOptions,sumchar(SkinPath,GetIniStr(&Ini,"WeaponOptionsButton","Image",NULL)));
 if(GUI_LocalLobby.pBtnBack.Visible)
   LOAD_IMAGE(GUI_LocalLobby.tBtnBack,sumchar(SkinPath,GetIniStr(&Ini,"BackButton","Image",NULL)));
 if(GUI_LocalLobby.pBtnStart.Visible)
   LOAD_IMAGE(GUI_LocalLobby.tBtnStart,sumchar(SkinPath,GetIniStr(&Ini,"StartButton","Image",NULL)));
 if(GUI_LocalLobby.pCaption.Visible)
   LOAD_IMAGE(GUI_LocalLobby.tCaption,sumchar(SkinPath,GetIniStr(&Ini,"Caption","Image",NULL)));

 return true;
}


bool GUI_LoadInternet(void)
{

 CIni Ini;
 char *SkinPath;

 // Internet Tab
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Net_Internet.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Internet.pBtnInternet,"InternetButton");
 FillProperties(&Ini,&GUI_Internet.pBtnLAN,"LANButton");
 FillProperties(&Ini,&GUI_Internet.pBtnHost,"HostButton");
 FillProperties(&Ini,&GUI_Internet.pBtnBack,"BackButton");
 FillProperties(&Ini,&GUI_Internet.pBtnAdd,"AddButton");
 FillProperties(&Ini,&GUI_Internet.pBtnRefresh,"RefreshButton");
 FillProperties(&Ini,&GUI_Internet.pBtnUpdateList,"UpdateListButton");
 FillProperties(&Ini,&GUI_Internet.pBtnJoin,"JoinButton");
 FillProperties(&Ini,&GUI_Internet.pCaption,"Caption");
 FillProperties(&Ini,&GUI_Internet.pListView,"ServerList");
 FillFrame(&Ini,&GUI_Internet.pFrame,"Frame");

 LOAD_IMAGE(GUI_Internet.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_Internet.pBtnInternet.Visible)
   LOAD_IMAGE(GUI_Internet.tBtnInternet,sumchar(SkinPath,GetIniStr(&Ini,"InternetButton","Image",NULL)));
 if(GUI_Internet.pBtnLAN.Visible)
   LOAD_IMAGE(GUI_Internet.tBtnLAN,sumchar(SkinPath,GetIniStr(&Ini,"LANButton","Image",NULL)));
 if(GUI_Internet.pBtnHost.Visible)
   LOAD_IMAGE(GUI_Internet.tBtnHost,sumchar(SkinPath,GetIniStr(&Ini,"HostButton","Image",NULL)));
 if(GUI_Internet.pBtnBack.Visible)
   LOAD_IMAGE(GUI_Internet.tBtnBack,sumchar(SkinPath,GetIniStr(&Ini,"BackButton","Image",NULL)));
 if(GUI_Internet.pBtnAdd.Visible)
   LOAD_IMAGE(GUI_Internet.tBtnAdd,sumchar(SkinPath,GetIniStr(&Ini,"AddButton","Image",NULL)));
 if(GUI_Internet.pBtnRefresh.Visible)
   LOAD_IMAGE(GUI_Internet.tBtnRefresh,sumchar(SkinPath,GetIniStr(&Ini,"RefreshButton","Image",NULL)));
 if(GUI_Internet.pBtnUpdateList.Visible)
   LOAD_IMAGE(GUI_Internet.tBtnUpdateList,sumchar(SkinPath,GetIniStr(&Ini,"UpdateListButton","Image",NULL)));
 if(GUI_Internet.pBtnJoin.Visible)
   LOAD_IMAGE(GUI_Internet.tBtnJoin,sumchar(SkinPath,GetIniStr(&Ini,"JoinButton","Image",NULL)));
 if(GUI_Internet.pCaption.Visible)
   LOAD_IMAGE(GUI_Internet.tCaption,sumchar(SkinPath,GetIniStr(&Ini,"Caption","Image",NULL)));

 // Connect page
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Net_Connect.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Connect.pBtnInternet,"InternetButton");
 FillProperties(&Ini,&GUI_Connect.pBtnLAN,"LANButton");
 FillProperties(&Ini,&GUI_Connect.pBtnHost,"HostButton");
 FillProperties(&Ini,&GUI_Connect.pBtnBack,"BackButton");
 FillProperties(&Ini,&GUI_Connect.pBtnOk,"OkButton");
 FillProperties(&Ini,&GUI_Connect.pLblSelectPlayer,"SelectPlayerLabel");
 FillProperties(&Ini,&GUI_Connect.pLblPlayers,"PlayersLabel");
 FillProperties(&Ini,&GUI_Connect.pLblPlaying,"PlayingLabel");
 FillProperties(&Ini,&GUI_Connect.pLsvPlayers,"PlayersList");
 FillProperties(&Ini,&GUI_Connect.pLsvPlaying,"PlayingList");
 FillProperties(&Ini,&GUI_Connect.pCaption,"Caption");
 FillFrame(&Ini,&GUI_Connect.pFrame,"Frame");

 LOAD_IMAGE(GUI_Connect.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_Connect.pBtnInternet.Visible)
   LOAD_IMAGE(GUI_Connect.tBtnInternet,sumchar(SkinPath,GetIniStr(&Ini,"InternetButton","Image",NULL)));
 if(GUI_Connect.pBtnLAN.Visible)
   LOAD_IMAGE(GUI_Connect.tBtnLAN,sumchar(SkinPath,GetIniStr(&Ini,"LANButton","Image",NULL)));
 if(GUI_Connect.pBtnHost.Visible)
   LOAD_IMAGE(GUI_Connect.tBtnHost,sumchar(SkinPath,GetIniStr(&Ini,"HostButton","Image",NULL)));
 if(GUI_Connect.pBtnBack.Visible)
   LOAD_IMAGE(GUI_Connect.tBtnBack,sumchar(SkinPath,GetIniStr(&Ini,"BackButton","Image",NULL)));
 if(GUI_Connect.pBtnOk.Visible)
   LOAD_IMAGE(GUI_Connect.tBtnOk,sumchar(SkinPath,GetIniStr(&Ini,"AddButton","Image",NULL)));
 if(GUI_Connect.pCaption.Visible)
   LOAD_IMAGE(GUI_Connect.tCaption,sumchar(SkinPath,GetIniStr(&Ini,"Caption","Image",NULL)));

 // Connecting page
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Net_Connecting.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Connecting.pBtnCancel,"CancelButton");
 FillProperties(&Ini,&GUI_Connecting.pLblText,"ConnectingLabel");
 FillFrame(&Ini,&GUI_Connecting.pFrame,"Frame");

 LOAD_IMAGE(GUI_Connecting.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_Connecting.pBtnCancel.Visible)
   LOAD_IMAGE(GUI_Connecting.tBtnCancel,sumchar(SkinPath,GetIniStr(&Ini,"CancelButton","Image",NULL)));

 return true;
}


bool GUI_LoadLAN(void)
{
 CIni Ini;
 char *SkinPath;
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Net_LAN.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_LAN.pBtnInternet,"InternetButton");
 FillProperties(&Ini,&GUI_LAN.pBtnLAN,"LANButton");
 FillProperties(&Ini,&GUI_LAN.pBtnHost,"HostButton");
 FillProperties(&Ini,&GUI_LAN.pBtnBack,"BackButton");
 FillProperties(&Ini,&GUI_LAN.pBtnRefresh,"RefreshButton");
 FillProperties(&Ini,&GUI_LAN.pBtnJoin,"JoinButton");
 FillProperties(&Ini,&GUI_LAN.pCaption,"Caption");
 FillProperties(&Ini,&GUI_LAN.pListView,"ServerList");
 FillFrame(&Ini,&GUI_LAN.pFrame,"Frame");

 LOAD_IMAGE(GUI_LAN.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_LAN.pBtnInternet.Visible)
   LOAD_IMAGE(GUI_LAN.tBtnInternet,sumchar(SkinPath,GetIniStr(&Ini,"InternetButton","Image",NULL)));
 if(GUI_LAN.pBtnLAN.Visible)
   LOAD_IMAGE(GUI_LAN.tBtnLAN,sumchar(SkinPath,GetIniStr(&Ini,"LANButton","Image",NULL)));
 if(GUI_LAN.pBtnHost.Visible)
   LOAD_IMAGE(GUI_LAN.tBtnHost,sumchar(SkinPath,GetIniStr(&Ini,"HostButton","Image",NULL)));
 if(GUI_LAN.pBtnBack.Visible)
   LOAD_IMAGE(GUI_LAN.tBtnBack,sumchar(SkinPath,GetIniStr(&Ini,"BackButton","Image",NULL)));
 if(GUI_LAN.pBtnRefresh.Visible)
   LOAD_IMAGE(GUI_LAN.tBtnRefresh,sumchar(SkinPath,GetIniStr(&Ini,"RefreshButton","Image",NULL)));
 if(GUI_LAN.pBtnJoin.Visible)
   LOAD_IMAGE(GUI_LAN.tBtnJoin,sumchar(SkinPath,GetIniStr(&Ini,"JoinButton","Image",NULL)));
 if(GUI_LAN.pCaption.Visible)
   LOAD_IMAGE(GUI_LAN.tCaption,sumchar(SkinPath,GetIniStr(&Ini,"Caption","Image",NULL)));

 return true;
}


bool GUI_LoadHost(void)
{
 CIni Ini;
 char *SkinPath;
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Net_Host.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Host.pBtnInternet,"InternetButton");
 FillProperties(&Ini,&GUI_Host.pBtnLAN,"LANButton");
 FillProperties(&Ini,&GUI_Host.pBtnHost,"HostButton");
 FillProperties(&Ini,&GUI_Host.pBtnBack,"BackButton");
 FillProperties(&Ini,&GUI_Host.pBtnOk,"OkButton");
 FillProperties(&Ini,&GUI_Host.pCaption,"Caption");
 FillProperties(&Ini,&GUI_Host.pLblServerSettings,"ServerSettingsLabel");
 FillProperties(&Ini,&GUI_Host.pLblPlayerSettings,"PlayerSettingsLabel");
 FillProperties(&Ini,&GUI_Host.pLblServerName,"ServerNameLabel");
 FillProperties(&Ini,&GUI_Host.pEdtServerName,"ServerNameEdit");
 FillProperties(&Ini,&GUI_Host.pLblMaxPlayers,"MaxPlayersLabel");
 FillProperties(&Ini,&GUI_Host.pEdtMaxPlayers,"MaxPlayersEdit");
 FillProperties(&Ini,&GUI_Host.pLblRegisterServer,"RegisterServerLabel");
 FillProperties(&Ini,&GUI_Host.pCbxRegisterServer,"RegisterServerCheckbox");
 FillProperties(&Ini,&GUI_Host.pPlayers,"PlayersList");
 FillProperties(&Ini,&GUI_Host.pPlaying,"PlayingList");
 FillFrame(&Ini,&GUI_Host.pFrame,"Frame");

 LOAD_IMAGE(GUI_Host.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_Host.pBtnInternet.Visible)
   LOAD_IMAGE(GUI_Host.tBtnInternet,sumchar(SkinPath,GetIniStr(&Ini,"InternetButton","Image",NULL)));
 if(GUI_Host.pBtnLAN.Visible)
   LOAD_IMAGE(GUI_Host.tBtnLAN,sumchar(SkinPath,GetIniStr(&Ini,"LANButton","Image",NULL)));
 if(GUI_Host.pBtnHost.Visible)
   LOAD_IMAGE(GUI_Host.tBtnHost,sumchar(SkinPath,GetIniStr(&Ini,"HostButton","Image",NULL)));
 if(GUI_Host.pBtnBack.Visible)
   LOAD_IMAGE(GUI_Host.tBtnBack,sumchar(SkinPath,GetIniStr(&Ini,"BackButton","Image",NULL)));
 if(GUI_Host.pBtnOk.Visible)
   LOAD_IMAGE(GUI_Host.tBtnOk,sumchar(SkinPath,GetIniStr(&Ini,"OkButton","Image",NULL)));
 if(GUI_Host.pCaption.Visible)
   LOAD_IMAGE(GUI_Host.tCaption,sumchar(SkinPath,GetIniStr(&Ini,"Caption","Image",NULL)));

 return true;
}

bool GUI_LoadHostLobby(void)
{
 CIni Ini;
 char *SkinPath;
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Net_HostLobby.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_HostLobby.pBtnLeave,"LeaveButton");
 FillProperties(&Ini,&GUI_HostLobby.pBtnStart,"StartButton");
 FillProperties(&Ini,&GUI_HostLobby.pBtnBanList,"BanListButton");
 FillProperties(&Ini,&GUI_HostLobby.pBtnWeaponOptions,"WeaponOptionsButton");
 FillProperties(&Ini,&GUI_HostLobby.pBtnGameSettings,"GameSettingsButton");
 FillProperties(&Ini,&GUI_HostLobby.pMiniMap,"MiniMap");
 FillProperties(&Ini,&GUI_HostLobby.pLblMod,"ModLabel");
 FillProperties(&Ini,&GUI_HostLobby.pLblLevel,"LevelLabel");
 FillProperties(&Ini,&GUI_HostLobby.pLblGameType,"GameTypeLabel");
 FillProperties(&Ini,&GUI_HostLobby.pCbbMod,"ModCombobox");
 FillProperties(&Ini,&GUI_HostLobby.pCbbLevel,"LevelCombobox");
 FillProperties(&Ini,&GUI_HostLobby.pCbbGameType,"GameTypeCombobox");
 FillProperties(&Ini,&GUI_HostLobby.pLblPlayers,"PlayersLabel");
 FillProperties(&Ini,&GUI_HostLobby.pLsvPlayers,"RegisterServerCheckbox");
 FillProperties(&Ini,&GUI_HostLobby.pEdtChat,"ChatEdit");
 FillProperties(&Ini,&GUI_HostLobby.pLsvChat,"ChatBox");
 FillProperties(&Ini,&GUI_HostLobby.pLblCaption,"CaptionLabel");
 FillFrame(&Ini,&GUI_HostLobby.pChatBorder,"ChatBorder");
 FillFrame(&Ini,&GUI_HostLobby.pFrame,"Frame");

 LOAD_IMAGE(GUI_HostLobby.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_HostLobby.pBtnLeave.Visible)
   LOAD_IMAGE(GUI_HostLobby.tBtnLeave,sumchar(SkinPath,GetIniStr(&Ini,"LeaveButton","Image",NULL)));
 if(GUI_HostLobby.pBtnStart.Visible)
   LOAD_IMAGE(GUI_HostLobby.tBtnStart,sumchar(SkinPath,GetIniStr(&Ini,"StartButton","Image",NULL)));
 if(GUI_HostLobby.pBtnBanList.Visible)
   LOAD_IMAGE(GUI_HostLobby.tBtnBanList,sumchar(SkinPath,GetIniStr(&Ini,"BanListButton","Image",NULL)));
 if(GUI_HostLobby.pBtnWeaponOptions.Visible)
   LOAD_IMAGE(GUI_HostLobby.tBtnWeaponOptions,sumchar(SkinPath,GetIniStr(&Ini,"WeaponOptionsButton","Image",NULL)));
 if(GUI_HostLobby.pBtnGameSettings.Visible)
   LOAD_IMAGE(GUI_HostLobby.tBtnGameSettings,sumchar(SkinPath,GetIniStr(&Ini,"GameSettingsButton","Image",NULL)));
 if(GUI_HostLobby.pLsvChat.Visible)
   LOAD_IMAGE(GUI_HostLobby.tChatBackground,sumchar(SkinPath,GetIniStr(&Ini,"ChatBox","Image",NULL)));

 return true;
}


bool GUI_LoadClientLobby(void)
{
 CIni Ini;
 char *SkinPath;
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Net_ClientLobby.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_ClientLobby.pBtnLeave,"LeaveButton");
 FillProperties(&Ini,&GUI_ClientLobby.pBtnReady,"ReadyButton");
 FillProperties(&Ini,&GUI_ClientLobby.pLblMod,"ModLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblLevel,"LevelLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblGameType,"GameTypeLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblLives,"LivesLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblMaxKills,"MaxKillsLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblLoadingTime,"LoadingTimeLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblBonuses,"BonusesLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblModName,"ModNameLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblLevelName,"LevelNameLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblGameTypeText,"GameTypeTextLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblLivesCount,"LivesCountLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblMaxKillsCount,"MaxKillsCountLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblLoadingTimeCount,"LoadingTimeCountLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblBonusesEnabled,"BonusesEnabledLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblGameSettings,"GameSettingsLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblPlayers,"GameSettingsLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLsvPlayers,"GameSettingsLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pEdtChat,"GameSettingsLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLsvChat,"GameSettingsLabel");
 FillProperties(&Ini,&GUI_ClientLobby.pLblCaption,"CaptionLabel");
 FillFrame(&Ini,&GUI_ClientLobby.pChatBorder,"ChatBorder");
 FillFrame(&Ini,&GUI_ClientLobby.pFrame,"Frame");

 LOAD_IMAGE(GUI_ClientLobby.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_ClientLobby.pBtnLeave.Visible)
   LOAD_IMAGE(GUI_ClientLobby.tBtnLeave,sumchar(SkinPath,GetIniStr(&Ini,"LeaveButton","Image",NULL)));
 if(GUI_ClientLobby.pBtnReady.Visible)
   LOAD_IMAGE(GUI_ClientLobby.tBtnReady,sumchar(SkinPath,GetIniStr(&Ini,"ReadyButton","Image",NULL)));
 if(GUI_ClientLobby.pLsvChat.Visible)
   LOAD_IMAGE(GUI_ClientLobby.tChatBackground,sumchar(SkinPath,GetIniStr(&Ini,"ChatBox","Image",NULL)));

 return true;
}


bool GUI_LoadNewPlayer(void)
{
 CIni Ini;
 char *SkinPath;
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Profiles_NewPlayer.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_NewPlayer.pBtnNewPlayer,"NewPlayerButton");
 FillProperties(&Ini,&GUI_NewPlayer.pBtnViewPlayers,"ViewPlayersButton");
 FillProperties(&Ini,&GUI_NewPlayer.pBtnBack,"BackButton");
 FillProperties(&Ini,&GUI_NewPlayer.pBtnCreate,"CreateButton");
 FillProperties(&Ini,&GUI_NewPlayer.pCaption,"Caption");
 FillProperties(&Ini,&GUI_NewPlayer.pLblDetails,"DetailsLabel");
 FillProperties(&Ini,&GUI_NewPlayer.pLblName,"NameLabel");
 FillProperties(&Ini,&GUI_NewPlayer.pLblType,"TypeLabel");
 FillProperties(&Ini,&GUI_NewPlayer.pLblSkin,"SkinTypeLabel");
 FillProperties(&Ini,&GUI_NewPlayer.pLblRed,"RedLabel");
 FillProperties(&Ini,&GUI_NewPlayer.pLblGreen,"GreenLabel");
 FillProperties(&Ini,&GUI_NewPlayer.pLblBlue,"BlueLabel");
 FillProperties(&Ini,&GUI_NewPlayer.pEdtName,"NameEdit");
 FillProperties(&Ini,&GUI_NewPlayer.pCbbType,"TypeCombobox");
 FillProperties(&Ini,&GUI_NewPlayer.pCbbSkin,"SkinCombobox");
 FillProperties(&Ini,&GUI_NewPlayer.pSldRed,"RedSlider");
 FillProperties(&Ini,&GUI_NewPlayer.pSldGreen,"GreenSlider");
 FillProperties(&Ini,&GUI_NewPlayer.pSldBlue,"BlueSlider");
 FillProperties(&Ini,&GUI_NewPlayer.pSkinPreview,"SkinPreview");
 FillFrame(&Ini,&GUI_NewPlayer.pSkinPreviewBorder,"SkinPreviewBorder");
 FillFrame(&Ini,&GUI_NewPlayer.pFrame,"Frame");

 LOAD_IMAGE(GUI_NewPlayer.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_NewPlayer.pBtnNewPlayer.Visible)
   LOAD_IMAGE(GUI_NewPlayer.tBtnNewPlayer,sumchar(SkinPath,GetIniStr(&Ini,"NewPlayerButton","Image",NULL)));
 if(GUI_NewPlayer.pBtnViewPlayers.Visible)
   LOAD_IMAGE(GUI_NewPlayer.tBtnViewPlayers,sumchar(SkinPath,GetIniStr(&Ini,"ViewPlayersButton","Image",NULL)));
 if(GUI_NewPlayer.pBtnBack.Visible)
   LOAD_IMAGE(GUI_NewPlayer.tBtnBack,sumchar(SkinPath,GetIniStr(&Ini,"BackButton","Image",NULL)));
 if(GUI_NewPlayer.pBtnCreate.Visible)
   LOAD_IMAGE(GUI_NewPlayer.tBtnCreate,sumchar(SkinPath,GetIniStr(&Ini,"CreateButton","Image",NULL)));
 if(GUI_NewPlayer.pCaption.Visible)
   LOAD_IMAGE(GUI_NewPlayer.tCaption,sumchar(SkinPath,GetIniStr(&Ini,"Caption","Image",NULL)));

 return true;
}

bool GUI_LoadViewPlayers(void)
{
 CIni Ini;
 char *SkinPath;
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Profiles_ViewPlayers.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_ViewPlayers.pBtnNewPlayer,"NewPlayerButton");
 FillProperties(&Ini,&GUI_ViewPlayers.pBtnViewPlayers,"ViewPlayersButton");
 FillProperties(&Ini,&GUI_ViewPlayers.pBtnBack,"BackButton");
 FillProperties(&Ini,&GUI_ViewPlayers.pBtnDelete,"DeleteButton");
 FillProperties(&Ini,&GUI_ViewPlayers.pBtnApply,"ApplyButton");
 FillProperties(&Ini,&GUI_ViewPlayers.pCaption,"Caption");
 FillProperties(&Ini,&GUI_ViewPlayers.pLblDetails,"DetailsLabel");
 FillProperties(&Ini,&GUI_ViewPlayers.pLblName,"NameLabel");
 FillProperties(&Ini,&GUI_ViewPlayers.pLblType,"TypeLabel");
 FillProperties(&Ini,&GUI_ViewPlayers.pLblSkin,"SkinTypeLabel");
 FillProperties(&Ini,&GUI_ViewPlayers.pLblRed,"RedLabel");
 FillProperties(&Ini,&GUI_ViewPlayers.pLblGreen,"GreenLabel");
 FillProperties(&Ini,&GUI_ViewPlayers.pLblBlue,"BlueLabel");
 FillProperties(&Ini,&GUI_ViewPlayers.pEdtName,"NameEdit");
 FillProperties(&Ini,&GUI_ViewPlayers.pCbbType,"TypeCombobox");
 FillProperties(&Ini,&GUI_ViewPlayers.pCbbSkin,"SkinCombobox");
 FillProperties(&Ini,&GUI_ViewPlayers.pSldRed,"RedSlider");
 FillProperties(&Ini,&GUI_ViewPlayers.pSldGreen,"GreenSlider");
 FillProperties(&Ini,&GUI_ViewPlayers.pSldBlue,"BlueSlider");
 FillProperties(&Ini,&GUI_ViewPlayers.pLblPlayers,"PlayersLabel");
 FillProperties(&Ini,&GUI_ViewPlayers.pLsvPlayers,"PlayersList");
 FillProperties(&Ini,&GUI_ViewPlayers.pSkinPreview,"SkinPreview");
 FillFrame(&Ini,&GUI_ViewPlayers.pSkinPreviewBorder,"SkinPreviewBorder");
 FillFrame(&Ini,&GUI_ViewPlayers.pFrame,"Frame");

 LOAD_IMAGE(GUI_ViewPlayers.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_ViewPlayers.pBtnNewPlayer.Visible)
   LOAD_IMAGE(GUI_ViewPlayers.tBtnNewPlayer,sumchar(SkinPath,GetIniStr(&Ini,"NewPlayerButton","Image",NULL)));
 if(GUI_ViewPlayers.pBtnViewPlayers.Visible)
   LOAD_IMAGE(GUI_ViewPlayers.tBtnViewPlayers,sumchar(SkinPath,GetIniStr(&Ini,"ViewPlayersButton","Image",NULL)));
 if(GUI_ViewPlayers.pBtnBack.Visible)
   LOAD_IMAGE(GUI_ViewPlayers.tBtnBack,sumchar(SkinPath,GetIniStr(&Ini,"BackButton","Image",NULL)));
 if(GUI_ViewPlayers.pBtnDelete.Visible)
   LOAD_IMAGE(GUI_ViewPlayers.tBtnDelete,sumchar(SkinPath,GetIniStr(&Ini,"DeleteButton","Image",NULL)));
 if(GUI_ViewPlayers.pBtnApply.Visible)
   LOAD_IMAGE(GUI_ViewPlayers.tBtnApply,sumchar(SkinPath,GetIniStr(&Ini,"ApplyButton","Image",NULL)));
 if(GUI_ViewPlayers.pCaption.Visible)
   LOAD_IMAGE(GUI_ViewPlayers.tCaption,sumchar(SkinPath,GetIniStr(&Ini,"Caption","Image",NULL)));

 return true;
}


bool GUI_LoadLevelEditor(void)
{
 CIni Ini;
 char *SkinPath;
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Profiles_ViewPlayers.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_LevelEditor.pBtnClean,"CleanButton");
 FillProperties(&Ini,&GUI_LevelEditor.pBtnStone,"StoneButton");
 FillProperties(&Ini,&GUI_LevelEditor.pBtnBone,"BoneButton");
 FillProperties(&Ini,&GUI_LevelEditor.pBtnDirt,"DirtButton");
 FillProperties(&Ini,&GUI_LevelEditor.pBtnNew,"NewButton");
 FillProperties(&Ini,&GUI_LevelEditor.pBtnRandom,"RandomButton");
 FillProperties(&Ini,&GUI_LevelEditor.pBtnLoad,"LoadButton");
 FillProperties(&Ini,&GUI_LevelEditor.pBtnSave,"SaveButton");
 FillProperties(&Ini,&GUI_LevelEditor.pBtnQuit,"QuitButton");
 FillProperties(&Ini,&GUI_LevelEditor.pCaption,"Caption");
 FillProperties(&Ini,&GUI_LevelEditor.pLevel,"LevelWindow");
 FillProperties(&Ini,&GUI_LevelEditor.pBrush,"Brush");
 FillFrame(&Ini,&GUI_LevelEditor.pBrushBorder,"BrushBorder");
 FillFrame(&Ini,&GUI_LevelEditor.pFrame,"Frame");

 LOAD_IMAGE(GUI_LevelEditor.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_LevelEditor.pBtnClean.Visible)
   LOAD_IMAGE(GUI_LevelEditor.tBtnClean,sumchar(SkinPath,GetIniStr(&Ini,"CleanButton","Image",NULL)));
 if(GUI_LevelEditor.pBtnStone.Visible)
   LOAD_IMAGE(GUI_LevelEditor.tBtnStone,sumchar(SkinPath,GetIniStr(&Ini,"LevelEditorButton","Image",NULL)));
 if(GUI_LevelEditor.pBtnBone.Visible)
   LOAD_IMAGE(GUI_LevelEditor.tBtnBone,sumchar(SkinPath,GetIniStr(&Ini,"BoneButton","Image",NULL)));
 if(GUI_LevelEditor.pBtnDirt.Visible)
   LOAD_IMAGE(GUI_LevelEditor.tBtnDirt,sumchar(SkinPath,GetIniStr(&Ini,"DirtButton","Image",NULL)));
 if(GUI_LevelEditor.pBtnNew.Visible)
   LOAD_IMAGE(GUI_LevelEditor.tBtnNew,sumchar(SkinPath,GetIniStr(&Ini,"NewButton","Image",NULL)));
 if(GUI_LevelEditor.pBtnRandom.Visible)
   LOAD_IMAGE(GUI_LevelEditor.tBtnRandom,sumchar(SkinPath,GetIniStr(&Ini,"RandomButton","Image",NULL)));
 if(GUI_LevelEditor.pBtnLoad.Visible)
   LOAD_IMAGE(GUI_LevelEditor.tBtnLoad,sumchar(SkinPath,GetIniStr(&Ini,"LoadButton","Image",NULL)));
 if(GUI_LevelEditor.pBtnSave.Visible)
   LOAD_IMAGE(GUI_LevelEditor.tBtnSave,sumchar(SkinPath,GetIniStr(&Ini,"SaveButton","Image",NULL)));
 if(GUI_LevelEditor.pBtnQuit.Visible)
   LOAD_IMAGE(GUI_LevelEditor.tBtnQuit,sumchar(SkinPath,GetIniStr(&Ini,"QuitButton","Image",NULL)));
 if(GUI_LevelEditor.pCaption.Visible)
   LOAD_IMAGE(GUI_LevelEditor.tCaption,sumchar(SkinPath,GetIniStr(&Ini,"Caption","Image",NULL)));

 return true;
}


bool GUI_LoadControlsOptions(void)
{
 CIni Ini;
 char *SkinPath;
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Options_Controls.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_ControlsOptions.pBtnControls,"ControlsButton");
 FillProperties(&Ini,&GUI_ControlsOptions.pBtnGame,"GameButton");
 FillProperties(&Ini,&GUI_ControlsOptions.pBtnSystem,"SystemButton");
 FillProperties(&Ini,&GUI_ControlsOptions.pBtnBack,"BackButton");
 FillProperties(&Ini,&GUI_ControlsOptions.pCaption,"Caption");
 FillProperties(&Ini,&GUI_ControlsOptions.pLblUp,"UpLabel");
 FillProperties(&Ini,&GUI_ControlsOptions.pLblDown,"DownLabel");
 FillProperties(&Ini,&GUI_ControlsOptions.pLblLeft,"LeftLabel");
 FillProperties(&Ini,&GUI_ControlsOptions.pLblRight,"RightLabel");
 FillProperties(&Ini,&GUI_ControlsOptions.pLblShoot,"ShootLabel");
 FillProperties(&Ini,&GUI_ControlsOptions.pLblJump,"JumpLabel");
 FillProperties(&Ini,&GUI_ControlsOptions.pLblSelectWeapon,"SelectWeaponLabel");
 FillProperties(&Ini,&GUI_ControlsOptions.pLblNinjaRope,"NinjaRopeLabel");
 FillProperties(&Ini,&GUI_ControlsOptions.pLblChat,"ChatLabel");
 FillProperties(&Ini,&GUI_ControlsOptions.pLblScore,"ScoreLabel");
 FillProperties(&Ini,&GUI_ControlsOptions.pLblPlayer1,"Player1Label");
 FillProperties(&Ini,&GUI_ControlsOptions.pLblPlayer2,"Player2Label");

 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl1Up,"Player1UpEdit");
 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl1Down,"Player1DownEdit");
 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl1Left,"Player1LeftEdit");
 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl1Right,"Player1RightEdit");
 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl1Shoot,"Player1ShootEdit");
 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl1Jump,"Player1JumpEdit");
 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl1SelectWeapon,"Player1SelectWeaponEdit");
 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl1NinjaRope,"Player1NinjaRopeEdit");

 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl2Up,"Player2UpEdit");
 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl2Down,"Player2DownEdit");
 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl2Left,"Player2LeftEdit");
 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl2Right,"Player2RightEdit");
 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl2Shoot,"Player2ShootEdit");
 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl2Jump,"Player2JumpEdit");
 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl2SelectWeapon,"Player2SelectWeaponEdit");
 FillProperties(&Ini,&GUI_ControlsOptions.pEdtPl2NinjaRope,"Player2NinjaRopeEdit");

 FillProperties(&Ini,&GUI_ControlsOptions.pEdtChat,"ChatEdit");
 FillProperties(&Ini,&GUI_ControlsOptions.pEdtScore,"ScoreEdit");

 FillFrame(&Ini,&GUI_ControlsOptions.pFrame,"Frame");

 LOAD_IMAGE(GUI_ControlsOptions.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_ControlsOptions.pBtnControls.Visible)
   LOAD_IMAGE(GUI_ControlsOptions.tBtnControls,sumchar(SkinPath,GetIniStr(&Ini,"ControlsButton","Image",NULL)));
 if(GUI_ControlsOptions.pBtnGame.Visible)
   LOAD_IMAGE(GUI_ControlsOptions.tBtnGame,sumchar(SkinPath,GetIniStr(&Ini,"GameButton","Image",NULL)));
 if(GUI_ControlsOptions.pBtnSystem.Visible)
   LOAD_IMAGE(GUI_ControlsOptions.tBtnSystem,sumchar(SkinPath,GetIniStr(&Ini,"SystemButton","Image",NULL)));
 if(GUI_ControlsOptions.pBtnBack.Visible)
   LOAD_IMAGE(GUI_ControlsOptions.tBtnBack,sumchar(SkinPath,GetIniStr(&Ini,"BackButton","Image",NULL)));
 if(GUI_ControlsOptions.pCaption.Visible)
   LOAD_IMAGE(GUI_ControlsOptions.tCaption,sumchar(SkinPath,GetIniStr(&Ini,"Caption","Image",NULL)));

 return true;
}


bool GUI_LoadMainGameOptions(void)
{
 CIni Ini;
 char *SkinPath;
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Options_Game.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_GameOptions.pBtnControls,"ControlsButton");
 FillProperties(&Ini,&GUI_GameOptions.pBtnGame,"GameButton");
 FillProperties(&Ini,&GUI_GameOptions.pBtnSystem,"SystemButton");
 FillProperties(&Ini,&GUI_GameOptions.pBtnBack,"BackButton");
 FillProperties(&Ini,&GUI_GameOptions.pCaption,"Caption");
 FillProperties(&Ini,&GUI_GameOptions.pLblBloodAmount,"BloodAmountLabel");
 FillProperties(&Ini,&GUI_GameOptions.pLblShadows,"ShadowsLabel");
 FillProperties(&Ini,&GUI_GameOptions.pLblParticles,"ParticlesLabel");
 FillProperties(&Ini,&GUI_GameOptions.pLblOldschoolRope,"OldschoolRopeLabel");
 FillProperties(&Ini,&GUI_GameOptions.pSldBloodAmount,"BloodAmountSlider");
 FillProperties(&Ini,&GUI_GameOptions.pCbxShadows,"ShadowsCheckbox");
 FillProperties(&Ini,&GUI_GameOptions.pCbxParticles,"ParticlesCheckbox");
 FillProperties(&Ini,&GUI_GameOptions.pCbxOldschoolRope,"OldschoolRopeCheckbox");
 FillFrame(&Ini,&GUI_GameOptions.pFrame,"Frame");

 LOAD_IMAGE(GUI_GameOptions.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_GameOptions.pBtnControls.Visible)
   LOAD_IMAGE(GUI_GameOptions.tBtnControls,sumchar(SkinPath,GetIniStr(&Ini,"ControlsButton","Image",NULL)));
 if(GUI_GameOptions.pBtnGame.Visible)
   LOAD_IMAGE(GUI_GameOptions.tBtnGame,sumchar(SkinPath,GetIniStr(&Ini,"GameButton","Image",NULL)));
 if(GUI_GameOptions.pBtnSystem.Visible)
   LOAD_IMAGE(GUI_GameOptions.tBtnSystem,sumchar(SkinPath,GetIniStr(&Ini,"SystemButton","Image",NULL)));
 if(GUI_GameOptions.pBtnBack.Visible)
   LOAD_IMAGE(GUI_GameOptions.tBtnBack,sumchar(SkinPath,GetIniStr(&Ini,"BackButton","Image",NULL)));
 if(GUI_GameOptions.pCaption.Visible)
   LOAD_IMAGE(GUI_GameOptions.tCaption,sumchar(SkinPath,GetIniStr(&Ini,"Caption","Image",NULL)));

 return true;
}

bool GUI_LoadMainSystemOptions(void)
{
 CIni Ini;
 char *SkinPath;
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Options_System.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_SystemOptions.pBtnControls,"ControlsButton");
 FillProperties(&Ini,&GUI_SystemOptions.pBtnGame,"GameButton");
 FillProperties(&Ini,&GUI_SystemOptions.pBtnSystem,"SystemButton");
 FillProperties(&Ini,&GUI_SystemOptions.pBtnBack,"BackButton");
 FillProperties(&Ini,&GUI_SystemOptions.pBtnApply,"ApplyButton");
 FillProperties(&Ini,&GUI_SystemOptions.pCaption,"Caption");
 FillProperties(&Ini,&GUI_SystemOptions.pLblVideo,"VideoLabel");
 FillProperties(&Ini,&GUI_SystemOptions.pLblFullscreen,"FullscreenLabel");
 FillProperties(&Ini,&GUI_SystemOptions.pLblSoundOn,"SoundOnLabel");
 FillProperties(&Ini,&GUI_SystemOptions.pLblSoundVolume,"SoundVolumeLabel");
 FillProperties(&Ini,&GUI_SystemOptions.pLblNetwork,"NetworkLabel");
 FillProperties(&Ini,&GUI_SystemOptions.pLblNetworkPort,"NetworkPortLabel");
 FillProperties(&Ini,&GUI_SystemOptions.pLblNetworkSpeed,"NetworkSpeedLabel");
 FillProperties(&Ini,&GUI_SystemOptions.pLblMisc,"MiscLabel");
 FillProperties(&Ini,&GUI_SystemOptions.pLblShowFPS,"ShowFPSLabel");
 FillProperties(&Ini,&GUI_SystemOptions.pLblFilteredLevel,"FilteredLevelLabel");
 FillProperties(&Ini,&GUI_SystemOptions.pLblLogConvos,"LogConvosLabel");
 FillProperties(&Ini,&GUI_SystemOptions.pCbxFullscreen,"FullscreenCheckbox");
 FillProperties(&Ini,&GUI_SystemOptions.pCbxSoundOn,"SoundOnCheckbox");
 FillProperties(&Ini,&GUI_SystemOptions.pSldSoundVolume,"SoundVolumeSlider");
 FillProperties(&Ini,&GUI_SystemOptions.pEdtNetworkPort,"NetworkPortEdit");
 FillProperties(&Ini,&GUI_SystemOptions.pCbbNetworkSpeed,"NetworkSpeedCombobox");
 FillProperties(&Ini,&GUI_SystemOptions.pCbxShowFPS,"ShowFPSCheckbox");
 FillProperties(&Ini,&GUI_SystemOptions.pCbxFiltered,"FilteredCheckbox");
 FillProperties(&Ini,&GUI_SystemOptions.pCbxLogConvos,"LogConvosCheckbox");
 FillFrame(&Ini,&GUI_SystemOptions.pFrame,"Frame");

 LOAD_IMAGE(GUI_SystemOptions.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_SystemOptions.pBtnControls.Visible)
   LOAD_IMAGE(GUI_SystemOptions.tBtnControls,sumchar(SkinPath,GetIniStr(&Ini,"ControlsButton","Image",NULL)));
 if(GUI_SystemOptions.pBtnGame.Visible)
   LOAD_IMAGE(GUI_SystemOptions.tBtnGame,sumchar(SkinPath,GetIniStr(&Ini,"GameButton","Image",NULL)));
 if(GUI_SystemOptions.pBtnSystem.Visible)
   LOAD_IMAGE(GUI_SystemOptions.tBtnSystem,sumchar(SkinPath,GetIniStr(&Ini,"SystemButton","Image",NULL)));
 if(GUI_SystemOptions.pBtnBack.Visible)
   LOAD_IMAGE(GUI_SystemOptions.tBtnBack,sumchar(SkinPath,GetIniStr(&Ini,"BackButton","Image",NULL)));
 if(GUI_SystemOptions.pBtnApply.Visible)
   LOAD_IMAGE(GUI_SystemOptions.tBtnApply,sumchar(SkinPath,GetIniStr(&Ini,"ApplyButton","Image",NULL)));
 if(GUI_SystemOptions.pCaption.Visible)
   LOAD_IMAGE(GUI_SystemOptions.tCaption,sumchar(SkinPath,GetIniStr(&Ini,"Caption","Image",NULL)));

 return true;
}

bool GUI_LoadInGame(void)
{
 CIni Ini;
 char *SkinPath;
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/InGame.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_InGame.pBar,"BottomBar");
 FillProperties(&Ini,&GUI_InGame.pLblHealth,"HealthLabel");
 FillProperties(&Ini,&GUI_InGame.pLblWeapon,"WeaponLabel");
 FillProperties(&Ini,&GUI_InGame.pLblLives,"LivesLabel");
 FillProperties(&Ini,&GUI_InGame.pLblKills,"KillsLabel");
 FillProperties(&Ini,&GUI_InGame.pLblLivesCount,"LivesCountLabel");
 FillProperties(&Ini,&GUI_InGame.pLblKillsCount,"KillsCountLabel");
 FillProperties(&Ini,&GUI_InGame.pMiniMap,"MiniMap");
 FillProperties(&Ini,&GUI_InGame.pHealth,"Health");
 FillFrame(&Ini,&GUI_InGame.pHealthBorder,"HealthBorder");
 FillProperties(&Ini,&GUI_InGame.pWeapon,"Weapon");
 FillProperties(&Ini,&GUI_InGame.pWeaponLoading,"WeaponLoading");
 FillFrame(&Ini,&GUI_InGame.pWeaponBorder,"WeaponBorder");
 FillProperties(&Ini,&GUI_InGame.pChat,"ChatBox");
 FillFrame(&Ini,&GUI_InGame.pChatBorder,"ChatBorder");

 LOAD_IMAGE(GUI_InGame.tBarBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_InGame.pHealth.Visible)  {
   LOAD_IMAGE(GUI_InGame.tHealthForeground,sumchar(SkinPath,GetIniStr(&Ini,"Health","ForegroundImage",NULL)));
   LOAD_IMAGE(GUI_InGame.tHealthBackground,sumchar(SkinPath,GetIniStr(&Ini,"Health","BackgroundImage",NULL)));
 }
 if(GUI_InGame.pWeapon.Visible)  {
   LOAD_IMAGE(GUI_InGame.tWeaponForeground,sumchar(SkinPath,GetIniStr(&Ini,"Weapon","ForegroundImage",NULL)));
   LOAD_IMAGE(GUI_InGame.tWeaponBackground,sumchar(SkinPath,GetIniStr(&Ini,"Weapon","BackgroundImage",NULL)));
   LOAD_IMAGE(GUI_InGame.tWeaponLoading,sumchar(SkinPath,GetIniStr(&Ini,"WeaponLoading","Image",NULL)));
 }

 if(GUI_InGame.pChat.Visible)
   LOAD_IMAGE(GUI_InGame.tChatBackground,sumchar(SkinPath,GetIniStr(&Ini,"ChatBox","BackgroundImage",NULL)));

 return true;
}


bool GUI_LoadGameDialogs(void)
{

 // Scoreboard	
 CIni Ini;
 char *SkinPath;
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Dialog_Scoreboard.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_ScoreBoard.pLblPlayers,"PlayersLabel");
 FillProperties(&Ini,&GUI_ScoreBoard.pLblL,"LLabel");
 FillProperties(&Ini,&GUI_ScoreBoard.pLblK,"KLabel");
 FillFrame(&Ini,&GUI_ScoreBoard.pHeaderBorder,"ListHeaderBorder");
 FillProperties(&Ini,&GUI_ScoreBoard.pHeaderSeparator,"HeaderSeparator");
 FillFrame(&Ini,&GUI_ScoreBoard.pItemBorder,"ListItemBorder");
 FillProperties(&Ini,&GUI_ScoreBoard.pItem,"ListItem");
 FillFrame(&Ini,&GUI_ScoreBoard.pListBorder,"ListBorder");
 FillProperties(&Ini,&GUI_ScoreBoard.pList,"List");
 FillFrame(&Ini,&GUI_ScoreBoard.pBorder,"Border");

 LOAD_IMAGE(GUI_ScoreBoard.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_ScoreBoard.pHeaderSeparator.Visible)
   LOAD_IMAGE(GUI_ScoreBoard.tHeaderSeparator,sumchar(SkinPath,GetIniStr(&Ini,"HeaderSeparator","Image",NULL)));
 LOAD_IMAGE(GUI_ScoreBoard.tListBackground,sumchar(SkinPath,GetIniStr(&Ini,"List","BackgroundImage",NULL)));

 // Game menu
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Dialog_GameMenu.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_GameMenu.pCaption,"Caption");
 FillProperties(&Ini,&GUI_GameMenu.pLblPlayers,"PlayersLabel");
 FillProperties(&Ini,&GUI_GameMenu.pLblLives,"LivesLabel");
 FillProperties(&Ini,&GUI_GameMenu.pLblKills,"KillsLabel");
 FillFrame(&Ini,&GUI_GameMenu.pHeaderBorder,"ListHeaderBorder");
 FillProperties(&Ini,&GUI_GameMenu.pHeaderSeparator,"ListHeaderSeparator");
 FillProperties(&Ini,&GUI_GameMenu.pHeader,"ListHeader");
 FillFrame(&Ini,&GUI_GameMenu.pItemBorder,"ListItemBorder");
 FillProperties(&Ini,&GUI_GameMenu.pItem,"ListItem");
 FillFrame(&Ini,&GUI_GameMenu.pListBorder,"ListBorder");
 FillProperties(&Ini,&GUI_GameMenu.pList,"List");
 FillProperties(&Ini,&GUI_GameMenu.pBtnResume,"ResumeButton");
 FillProperties(&Ini,&GUI_GameMenu.pBtnQuitGame,"QuitGameButton");
 FillFrame(&Ini,&GUI_GameMenu.pBorder,"Border");

 LOAD_IMAGE(GUI_GameMenu.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 LOAD_IMAGE(GUI_GameMenu.tListBackground,sumchar(SkinPath,GetIniStr(&Ini,"List","BackgroundImage",NULL)));

 if(GUI_GameMenu.pHeaderSeparator.Visible)
   LOAD_IMAGE(GUI_GameMenu.tHeaderSeparator,sumchar(SkinPath,GetIniStr(&Ini,"HeaderSeparator","Image",NULL)));
 if(GUI_GameMenu.pBtnResume.Visible)
   LOAD_IMAGE(GUI_GameMenu.tBtnResume,sumchar(SkinPath,GetIniStr(&Ini,"ResumeButton","Image",NULL)));
 if(GUI_GameMenu.pBtnQuitGame.Visible)
   LOAD_IMAGE(GUI_GameMenu.tBtnQuitGame,sumchar(SkinPath,GetIniStr(&Ini,"QuitButton","Image",NULL)));

 // Game Over (Local)
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Dialog_GameOverLocal.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_GameOverLocal.pCaption,"Caption");
 FillProperties(&Ini,&GUI_GameOverLocal.pLblPlayers,"PlayersLabel");
 FillProperties(&Ini,&GUI_GameOverLocal.pLblLives,"LivesLabel");
 FillProperties(&Ini,&GUI_GameOverLocal.pLblKills,"KillsLabel");
 FillFrame(&Ini,&GUI_GameOverLocal.pHeaderBorder,"ListHeaderBorder");
 FillProperties(&Ini,&GUI_GameOverLocal.pHeaderSeparator,"ListHeaderSeparator");
 FillProperties(&Ini,&GUI_GameOverLocal.pHeader,"ListHeader");
 FillFrame(&Ini,&GUI_GameOverLocal.pItemBorder,"ListItemBorder");
 FillProperties(&Ini,&GUI_GameOverLocal.pItem,"ListItem");
 FillFrame(&Ini,&GUI_GameOverLocal.pListBorder,"ListBorder");
 FillProperties(&Ini,&GUI_GameOverLocal.pList,"List");
 FillProperties(&Ini,&GUI_GameOverLocal.pBtnOk,"OkButton");
 FillFrame(&Ini,&GUI_GameOverLocal.pBorder,"Border");

 LOAD_IMAGE(GUI_GameOverLocal.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 LOAD_IMAGE(GUI_GameOverLocal.tListBackground,sumchar(SkinPath,GetIniStr(&Ini,"List","BackgroundImage",NULL)));

 if(GUI_GameOverLocal.pHeaderSeparator.Visible)
   LOAD_IMAGE(GUI_GameOverLocal.tHeaderSeparator,sumchar(SkinPath,GetIniStr(&Ini,"HeaderSeparator","Image",NULL)));
 if(GUI_GameOverLocal.pBtnOk.Visible)
   LOAD_IMAGE(GUI_GameOverLocal.tBtnOk,sumchar(SkinPath,GetIniStr(&Ini,"OkButton","Image",NULL)));

 // Game Over (Net)
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Dialog_GameOverNet.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_GameOverNet.pCaption,"Caption");
 FillProperties(&Ini,&GUI_GameOverNet.pLblPlayers,"PlayersLabel");
 FillProperties(&Ini,&GUI_GameOverNet.pLblLives,"LivesLabel");
 FillProperties(&Ini,&GUI_GameOverNet.pLblKills,"KillsLabel");
 FillFrame(&Ini,&GUI_GameOverNet.pHeaderBorder,"ListHeaderBorder");
 FillProperties(&Ini,&GUI_GameOverNet.pHeaderSeparator,"ListHeaderSeparator");
 FillProperties(&Ini,&GUI_GameOverNet.pHeader,"ListHeader");
 FillFrame(&Ini,&GUI_GameOverNet.pItemBorder,"ListItemBorder");
 FillProperties(&Ini,&GUI_GameOverNet.pItem,"ListItem");
 FillFrame(&Ini,&GUI_GameOverNet.pListBorder,"ListBorder");
 FillProperties(&Ini,&GUI_GameOverNet.pList,"List");
 FillProperties(&Ini,&GUI_GameOverNet.pLblReturning,"ReturningLabel");
 FillProperties(&Ini,&GUI_GameOverNet.pBtnLeave,"LeaveButton");
 FillFrame(&Ini,&GUI_GameOverNet.pBorder,"Border");

 LOAD_IMAGE(GUI_GameOverNet.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 LOAD_IMAGE(GUI_GameOverNet.tListBackground,sumchar(SkinPath,GetIniStr(&Ini,"List","BackgroundImage",NULL)));

 if(GUI_GameOverNet.pHeaderSeparator.Visible)
   LOAD_IMAGE(GUI_GameOverNet.tHeaderSeparator,sumchar(SkinPath,GetIniStr(&Ini,"HeaderSeparator","Image",NULL)));
 if(GUI_GameOverNet.pBtnLeave.Visible)
   LOAD_IMAGE(GUI_GameOverNet.tBtnLeave,sumchar(SkinPath,GetIniStr(&Ini,"LeaveButton","Image",NULL)));

 // Viewport manager
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Dialog_ViewportManager.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Viewport.pCaption,"Caption");
 FillProperties(&Ini,&GUI_Viewport.pGameMenu,"Main");
 FillProperties(&Ini,&GUI_Viewport.pLblViewport1,"Viewport1Label");
 FillProperties(&Ini,&GUI_Viewport.pLblViewport2,"Viewport2Label");
 FillProperties(&Ini,&GUI_Viewport.pLblUsed,"UsedLabel");
 FillProperties(&Ini,&GUI_Viewport.pLblType,"TypeLabel");
 FillProperties(&Ini,&GUI_Viewport.pCbbType1,"Type1Combobox");
 FillProperties(&Ini,&GUI_Viewport.pCbbType2,"Type2Combobox");
 FillProperties(&Ini,&GUI_Viewport.pCbxUsed,"UsedCheckbox");
 FillProperties(&Ini,&GUI_Viewport.pBtnOk,"OkButton");
 FillFrame(&Ini,&GUI_Viewport.pBorder,"Border");

 LOAD_IMAGE(GUI_Viewport.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_Viewport.pCaption.Visible)
   LOAD_IMAGE(GUI_Viewport.tCaption,sumchar(SkinPath,GetIniStr(&Ini,"Caption","Image",NULL)));
 if(GUI_Viewport.pBtnOk.Visible)
   LOAD_IMAGE(GUI_Viewport.tBtnOk,sumchar(SkinPath,GetIniStr(&Ini,"OkButton","Image",NULL)));

 // Console
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Dialog_Console.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Console.pConsole,"Main");
 FillProperties(&Ini,&GUI_Console.pFntTypedText,"TypedTextFont");
 FillProperties(&Ini,&GUI_Console.pFntNote,"NoteFont");
 FillFrame(&Ini,&GUI_Console.pBorder,"Border");

 LOAD_IMAGE(GUI_Console.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));

 return true;

}


bool GUI_LoadNonGameDialogs(void)
{
 CIni Ini;
 char *SkinPath;

 // Weapon options
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Dialog_WeaponOptions.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_WeaponOptions.pLblCaption,"CaptionLabel");
 FillProperties(&Ini,&GUI_WeaponOptions.pWeaponOptions,"Main");
 FillProperties(&Ini,&GUI_WeaponOptions.pScrollbar,"Scrollbar");
 FillProperties(&Ini,&GUI_WeaponOptions.pItem,"ListItem");
 FillProperties(&Ini,&GUI_WeaponOptions.pItemMouseOver,"ListItemMouseOver");
 FillProperties(&Ini,&GUI_WeaponOptions.pItemMouseDown,"ListItemMouseDown");
 FillProperties(&Ini,&GUI_WeaponOptions.pBtnOk,"OkButton");
 FillProperties(&Ini,&GUI_WeaponOptions.pBtnRandom,"RandomButton");
 FillProperties(&Ini,&GUI_WeaponOptions.pBtnCycle,"CycleButton");
 FillFrame(&Ini,&GUI_WeaponOptions.pBorder,"Border");

 LOAD_IMAGE(GUI_WeaponOptions.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_WeaponOptions.pBtnOk.Visible)
   LOAD_IMAGE(GUI_WeaponOptions.tBtnOk,sumchar(SkinPath,GetIniStr(&Ini,"OkButton","Image",NULL)));
 if(GUI_WeaponOptions.pBtnRandom.Visible)
   LOAD_IMAGE(GUI_WeaponOptions.tBtnRandom,sumchar(SkinPath,GetIniStr(&Ini,"GameButton","Image",NULL)));
 if(GUI_WeaponOptions.pBtnCycle.Visible)
   LOAD_IMAGE(GUI_WeaponOptions.tBtnCycle,sumchar(SkinPath,GetIniStr(&Ini,"SystemButton","Image",NULL)));

 // Quit Game
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Dialog_QuitGame.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_QuitGame.pLblCaption,"CaptionLabel");
 FillProperties(&Ini,&GUI_QuitGame.pQuitGame,"Main");
 FillProperties(&Ini,&GUI_QuitGame.pText,"TextBox");
 FillAlign(&Ini,&GUI_QuitGame.pTextAlign,"TextBox");
 FillValign(&Ini,&GUI_QuitGame.pTextValign,"TextBox");
 FillProperties(&Ini,&GUI_QuitGame.pBtnYes,"YesButton");
 FillProperties(&Ini,&GUI_QuitGame.pBtnNo,"NoButton");
 FillFrame(&Ini,&GUI_QuitGame.pBorder,"Border");

 LOAD_IMAGE(GUI_QuitGame.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_QuitGame.pBtnYes.Visible)
   LOAD_IMAGE(GUI_QuitGame.tBtnYes,sumchar(SkinPath,GetIniStr(&Ini,"YesButton","Image",NULL)));
 if(GUI_QuitGame.pBtnNo.Visible)
   LOAD_IMAGE(GUI_QuitGame.tBtnNo,sumchar(SkinPath,GetIniStr(&Ini,"NoButton","Image",NULL)));

 // Ban List
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Dialog_WeaponOptions.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_BanList.pLblCaption,"CaptionLabel");
 FillProperties(&Ini,&GUI_BanList.pBanList,"Main");
 FillProperties(&Ini,&GUI_BanList.pList,"Scrollbar");
 FillProperties(&Ini,&GUI_BanList.pBtnOk,"OkButton");
 FillProperties(&Ini,&GUI_BanList.pBtnUnban,"UnbanButton");
 FillProperties(&Ini,&GUI_BanList.pBtnClear,"ClearButton");
 FillFrame(&Ini,&GUI_BanList.pBorder,"Border");

 LOAD_IMAGE(GUI_BanList.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_BanList.pBtnOk.Visible)
   LOAD_IMAGE(GUI_BanList.tBtnOk,sumchar(SkinPath,GetIniStr(&Ini,"OkButton","Image",NULL)));
 if(GUI_BanList.pBtnUnban.Visible)
   LOAD_IMAGE(GUI_BanList.tBtnUnban,sumchar(SkinPath,GetIniStr(&Ini,"UnbanButton","Image",NULL)));
 if(GUI_BanList.pBtnClear.Visible)
   LOAD_IMAGE(GUI_BanList.tBtnClear,sumchar(SkinPath,GetIniStr(&Ini,"ClearButton","Image",NULL)));


 // Standard dialog
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Dialog_Standard.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Dialog.pLblCaption,"CaptionLabel");
 FillProperties(&Ini,&GUI_Dialog.pCaption,"Caption");
 FillProperties(&Ini,&GUI_Dialog.pDialog,"Main");
 FillProperties(&Ini,&GUI_Dialog.pText,"TextBox");
 FillAlign(&Ini,&GUI_Dialog.pTextAlign,"TextBox");
 FillValign(&Ini,&GUI_Dialog.pTextValign,"TextBox");
 FillProperties(&Ini,&GUI_Dialog.pBtnOk,"OkButton");
 FillFrame(&Ini,&GUI_Dialog.pBorder,"Border");

 LOAD_IMAGE(GUI_Dialog.tBackground,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));
 if(GUI_Dialog.pCaption.Visible)
   LOAD_IMAGE(GUI_Dialog.tCaption,sumchar(SkinPath,GetIniStr(&Ini,"Caption","BackgroundImage",NULL)));
 if(GUI_Dialog.pBtnOk.Visible)
   LOAD_IMAGE(GUI_Dialog.tBtnOk,sumchar(SkinPath,GetIniStr(&Ini,"OkButton","Image",NULL)));

 return true;

}


bool GUI_LoadWidgets(void)
{
 CIni Ini;
 char *SkinPath;

 // Checkbox
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Widget_Checkbox.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Checkbox.pCheckbox,"Main");

 LOAD_IMAGE(GUI_Checkbox.tCheckbox,sumchar(SkinPath,GetIniStr(&Ini,"Main","Image",NULL)));

 // Combobox
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Widget_Combobox.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Combobox.pCombobox,"Main");
 FillProperties(&Ini,&GUI_Combobox.pArrow,"Arrow");
 FillProperties(&Ini,&GUI_Combobox.pFont,"Font");
 FillProperties(&Ini,&GUI_Combobox.pDisabledFont,"DisabledFont");
 FillFrame(&Ini,&GUI_Combobox.pBorder,"Border");

 LOAD_IMAGE(GUI_Combobox.tArrow,sumchar(SkinPath,GetIniStr(&Ini,"Main","ArrowImage",NULL)));
 
 // Inputbox
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Widget_Inputbox.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Inputbox.pInputbox,"Main");
 FillProperties(&Ini,&GUI_Inputbox.pFont,"Font");
 FillFrame(&Ini,&GUI_Inputbox.pBorder,"Border");
 FillFrame(&Ini,&GUI_Inputbox.pBorderMouseOver,"BorderMouseOver");
 FillFrame(&Ini,&GUI_Inputbox.pBorderMouseDown,"BorderMouseDown");

 LOAD_IMAGE(GUI_Inputbox.tInputbox,sumchar(SkinPath,GetIniStr(&Ini,"Main","Image",NULL)));

  // List View
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Widget_Listview.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Listview.pListview,"Main");
 FillProperties(&Ini,&GUI_Listview.pItem,"Item");
 FillProperties(&Ini,&GUI_Listview.pFont,"Font");
 FillProperties(&Ini,&GUI_Listview.pDisabledFont,"DisabledFont");
 FillFrame(&Ini,&GUI_Listview.pBorder,"Border");

 // Popup Menu
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Widget_Popupmenu.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Popupmenu.pMenu,"Main");
 FillProperties(&Ini,&GUI_Popupmenu.pItem,"Item");
 FillProperties(&Ini,&GUI_Popupmenu.pItemOver,"ItemMouseOver");
 FillProperties(&Ini,&GUI_Popupmenu.pFont,"Font");
 FillProperties(&Ini,&GUI_Popupmenu.pFontOver,"FontMouseOver");
 FillFrame(&Ini,&GUI_Popupmenu.pBorder,"Border");

 // Scrollbar
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Widget_Scrollbar.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Scrollbar.pScrollbar,"Normal");
 FillProperties(&Ini,&GUI_Scrollbar.pScrollbarOver,"MouseOver");
 FillProperties(&Ini,&GUI_Scrollbar.pScrollbarDown,"MouseDown");
 FillFrame(&Ini,&GUI_Scrollbar.pBorder,"Border");
 FillFrame(&Ini,&GUI_Scrollbar.pBorderOver,"BorderOver");
 FillFrame(&Ini,&GUI_Scrollbar.pBorderDown,"BorderDown");

 LOAD_IMAGE(GUI_Scrollbar.tArrow,sumchar(SkinPath,GetIniStr(&Ini,"Main","ArrowImage",NULL)));
 LOAD_IMAGE(GUI_Scrollbar.tArrow,sumchar(SkinPath,GetIniStr(&Ini,"Main","BodyImage",NULL)));
 LOAD_IMAGE(GUI_Scrollbar.tArrow,sumchar(SkinPath,GetIniStr(&Ini,"Main","BackgroundImage",NULL)));

 // Slider
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Widget_Slider.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Slider.pSlider,"Main");

 LOAD_IMAGE(GUI_Slider.tCursor,sumchar(SkinPath,GetIniStr(&Ini,"Main","CursorImage",NULL)));
 LOAD_IMAGE(GUI_Slider.tStop,sumchar(SkinPath,GetIniStr(&Ini,"Main","StopImage",NULL)));
 LOAD_IMAGE(GUI_Slider.tLine,sumchar(SkinPath,GetIniStr(&Ini,"Main","LineImage",NULL)));

 // Text box
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Widget_Textbox.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Textbox.pTextbox,"Main");
 FillProperties(&Ini,&GUI_Textbox.pFont,"Font");
 FillFrame(&Ini,&GUI_Textbox.pBorder,"Border");


 return true;
}

bool GUI_LoadMain(void)
{
	/*struct main_s  {
	properties_s pFntNormal;
	properties_s pFntChat;
	properties_s pFntNotice;
	properties_s pFntNetwork;

	SDL_Surface *tMouseArrow;
	properties_s pMouseArrow;
	SDL_Surface *tMouseHand;
	properties_s pMouseHand;

	SDL_Surface *tFont1;
	SDL_Surface *tFont2;
	SDL_Surface *tFont3;
};
*//*
 CIni Ini;
 char *SkinPath;
 sprintf(SkinPath,"userdata/GuiSkins/%s/%s/Main.gui",tLXOptions->sSkinPath,tLXOptions->sResolution); 
 Ini.SetPathName((LPCTSTR) SkinPath);
 FillProperties(&Ini,&GUI_Main.pFntNormal,"NormalFont");
 FillProperties(&Ini,&GUI_Main.pFntChat,"ChatFont");
 FillProperties(&Ini,&GUI_Main.pFntNotice,"NoticeFont");
 FillProperties(&Ini,&GUI_Main.pFntNetwork,"NetworkFont");
 FillProperties(&Ini,&GUI_Main.pMouseArrow,"MouseArrow");
 FillProperties(&Ini,&GUI_Main.pMouseHand,"MouseHand");
 FillProperties(&Ini,&GUI_Main.pMouseText,"MouseText");
 /*FillProperties(&Ini,&GUI_Main.pMiniMap,"MiniMap");
 FillProperties(&Ini,&GUI_Main.pHealth,"Health");
 FillFrame(&Ini,&GUI_Main.pHealthBorder,"HealthBorder");
 FillProperties(&Ini,&GUI_Main.pWeapon,"Weapon");
 FillProperties(&Ini,&GUI_Main.pWeaponLoading,"WeaponLoading");
 FillFrame(&Ini,&GUI_Main.pWeaponBorder,"WeaponBorder");
 FillProperties(&Ini,&GUI_Main.pChat,"ChatBox");
 FillFrame(&Ini,&GUI_Main.pChatBorder,"ChatBorder");*//*

 if(GUI_Main.pMouseArrow.Visible)  
   LOAD_IMAGE(GUI_Main.tMouseArrow,sumchar(SkinPath,GetIniStr(&Ini,"MouseArrow","Image",NULL)));
 if(GUI_Main.pMouseHand.Visible)  
   LOAD_IMAGE(GUI_Main.tMouseHand,sumchar(SkinPath,GetIniStr(&Ini,"MouseHand","Image",NULL)));
 if(GUI_Main.pMouseText.Visible)  
   LOAD_IMAGE(GUI_Main.tMouseText,sumchar(SkinPath,GetIniStr(&Ini,"MouseText","Image",NULL)));

 LOAD_IMAGE(GUI_Main.tFont1,sumchar(SkinPath,GetIniStr(&Ini,"Fonts","Font1",NULL)));
 LOAD_IMAGE(GUI_Main.tFont2,sumchar(SkinPath,GetIniStr(&Ini,"Fonts","Font2",NULL)));
 LOAD_IMAGE(GUI_Main.tFont3,sumchar(SkinPath,GetIniStr(&Ini,"Fonts","Font3",NULL)));

 return true;
}*/