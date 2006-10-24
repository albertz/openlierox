/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// GUI skins procedures
// Created 5/8/02
// Jason Boettcher

/*
#ifndef __CGUISKIN_H__
#define __CGUISKIN_H__


typedef struct {
	int Left,Top,Width,Height;
	bool Visible;
	Uint32 Color,BackColor;
} properties_s;

enum frametype_e {
	Inset,
	Outset,
	Solid
};

typedef struct {
	Uint32 LightColor,DarkColor;
	frametype_e FrameType;
	int Thickness,Width,Height,Left,Top;
	bool Visible,Round;
} frame_s;





	// Main Menu
typedef struct {
	SDL_Surface *tBackground;
	SDL_Surface *tBtnLocalPlay;
	properties_s pBtnLocalPlay;
	SDL_Surface *tBtnNetPlay;
	properties_s pBtnNetPlay;
	SDL_Surface *tBtnPlayerProfiles;
	properties_s pBtnPlayerProfiles;
	SDL_Surface *tBtnLevelEditor;
	properties_s pBtnLevelEditor;
	SDL_Surface *tBtnOptions;
	properties_s pBtnOptions;
	SDL_Surface *tBtnQuit;
	properties_s pBtnQuit;
	SDL_Surface *tLXPLogo;
	properties_s pLXPLogo;
	frame_s pFrame;
	properties_s pAbout;
} mainmenu_s;


	// Local Play Lobby
typedef struct  {
	SDL_Surface *tBackground;
	SDL_Surface *tBtnGameSettings;
	properties_s pBtnGameSettings;
	SDL_Surface *tBtnWeaponOptions;
	properties_s pBtnWeaponOptions;
	SDL_Surface *tBtnBack;
	properties_s pBtnBack;
	SDL_Surface *tBtnStart;
	properties_s pBtnStart;
	SDL_Surface *tCaption;
	properties_s pCaption;
	properties_s pMiniMap;
	properties_s pLblMod;
	properties_s pLblLevel;
	properties_s pLblGameType;
	properties_s pCbbMod;
	properties_s pCbbLevel;
	properties_s pCbbGameType;
	properties_s pPlayers;
	properties_s pPlaying;
	frame_s pFrame;
} locallobby_s;

	//
	// Net Play
	//

	// Internet Tab
struct internet_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tBtnInternet;
	properties_s pBtnInternet;
	SDL_Surface *tBtnLAN;
	properties_s pBtnLAN;
	SDL_Surface *tBtnHost;
	properties_s pBtnHost;
	SDL_Surface *tBtnBack;
	properties_s pBtnBack;
	SDL_Surface *tBtnAdd;
	properties_s pBtnAdd;
	SDL_Surface *tBtnRefresh;
	properties_s pBtnRefresh;
	SDL_Surface *tBtnUpdateList;
	properties_s pBtnUpdateList;
	SDL_Surface *tBtnJoin;
	properties_s pBtnJoin;
	SDL_Surface *tCaption;
	properties_s pCaption;
	properties_s pListView;
	frame_s pFrame;
};

	// LAN Tab
struct lan_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tBtnInternet;
	properties_s pBtnInternet;
	SDL_Surface *tBtnLAN;
	properties_s pBtnLAN;
	SDL_Surface *tBtnHost;
	properties_s pBtnHost;
	SDL_Surface *tBtnBack;
	properties_s pBtnBack;
	SDL_Surface *tBtnRefresh;
	properties_s pBtnRefresh;
	SDL_Surface *tBtnJoin;
	properties_s pBtnJoin;
	SDL_Surface *tCaption;
	properties_s pCaption;
	properties_s pListView;
	frame_s pFrame;
};

	// Host Tab
struct host_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tBtnInternet;
	properties_s pBtnInternet;
	SDL_Surface *tBtnLAN;
	properties_s pBtnLAN;
	SDL_Surface *tBtnHost;
	properties_s pBtnHost;
	SDL_Surface *tBtnBack;
	properties_s pBtnBack;
	SDL_Surface *tBtnOk;
	properties_s pBtnOk;
	SDL_Surface *tCaption;
	properties_s pCaption;
	properties_s pLblServerSettings;
	properties_s pLblPlayerSettings;
	properties_s pLblServerName;
	properties_s pEdtServerName;
	properties_s pLblMaxPlayers;
	properties_s pEdtMaxPlayers;
	properties_s pLblRegisterServer;
	properties_s pCbxRegisterServer;
	properties_s pPlayers;
	properties_s pPlaying;
	frame_s pFrame;
};

	// Player list (in lobby) item
struct playerlistitem_s  {
	SDL_Surface *tBtnCommandButtonUp;
	properties_s pBtnCommandButtonUp;
	SDL_Surface *tBtnCommandButtonOvr;
	properties_s pBtnCommandButtonOvr;
	SDL_Surface *tBtnCommandButtonDown;
	properties_s pBtnCommandButtonDown;
	SDL_Surface *tReadyControlNotReady;
	properties_s pReadyControlNotReady;
	SDL_Surface *tReadyControlReady;
	properties_s pReadyControlReady;
	properties_s pWormSkin;
	properties_s pLblWormNick;
	SDL_Surface *tBorderLeft;
	properties_s pBorderLeft;
	SDL_Surface *tBorderRight;
	properties_s pBorderRight;
	SDL_Surface *tBorderTop;
	properties_s pBorderTop;
	SDL_Surface *tBorderBottom;
	properties_s pBorderBottom;
};

	// Host Lobby
struct hostlobby_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tBtnLeave;
	properties_s pBtnLeave;
	SDL_Surface *tBtnStart;
	properties_s pBtnStart;
	SDL_Surface *tBtnBanList;
	properties_s pBtnBanList;
	SDL_Surface *tBtnWeaponOptions;
	properties_s pBtnWeaponOptions;
	SDL_Surface *tBtnGameSettings;
	properties_s pBtnGameSettings;
	properties_s pMiniMap;
	properties_s pLblMod;
	properties_s pLblLevel;
	properties_s pLblGameType;
	properties_s pCbbMod;
	properties_s pCbbLevel;
	properties_s pCbbGameType;
	properties_s pLblPlayers;
	properties_s pLsvPlayers;
	properties_s pEdtChat;
	properties_s pLsvChat;
	SDL_Surface *tChatBackground;
	frame_s pChatBorder;

	properties_s pLblCaption;

	frame_s pFrame;
};

	// Client Lobby
struct clientlobby_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tBtnLeave;
	properties_s pBtnLeave;
	SDL_Surface *tBtnReady;
	properties_s pBtnReady;

	properties_s pLblMod;
	properties_s pLblLevel;
	properties_s pLblGameType;
	properties_s pLblLives;
	properties_s pLblMaxKills;
	properties_s pLblLoadingTime;
	properties_s pLblBonuses;

	properties_s pLblModName;
	properties_s pLblLevelName;
	properties_s pLblGameTypeText;
	properties_s pLblLivesCount;
	properties_s pLblMaxKillsCount;
	properties_s pLblLoadingTimeCount;
	properties_s pLblBonusesEnabled;

	properties_s pLblGameSettings;
	properties_s pLblPlayers;
	properties_s pLsvPlayers;
	properties_s pEdtChat;
	properties_s pLsvChat;
	SDL_Surface *tChatBackground;
	frame_s pChatBorder;

	properties_s pLblCaption;

	frame_s pFrame;
};

	// "Connecting" screen
struct connecting_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tBtnCancel;
	properties_s pBtnCancel;

	properties_s pLblText;
	frame_s pFrame;
};

	// "Connect - select player" screen
struct connect_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tCaption;
	properties_s pCaption;
	SDL_Surface *tBtnInternet;
	properties_s pBtnInternet;
	SDL_Surface *tBtnLAN;
	properties_s pBtnLAN;
	SDL_Surface *tBtnHost;
	properties_s pBtnHost;
	SDL_Surface *tBtnBack;
	properties_s pBtnBack;
	SDL_Surface *tBtnOk;
	properties_s pBtnOk;

	properties_s pLblSelectPlayer;
	properties_s pLblPlayers;
	properties_s pLblPlaying;
	properties_s pLsvPlayers;
	properties_s pLsvPlaying;

	frame_s pFrame;
};

	//
	// Player Profiles
	//

	// New Player
struct newplayer_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tBtnNewPlayer;
	properties_s pBtnNewPlayer;
	SDL_Surface *tBtnViewPlayers;
	properties_s pBtnViewPlayers;
	SDL_Surface *tBtnBack;
	properties_s pBtnBack;
	SDL_Surface *tBtnCreate;
	properties_s pBtnCreate;
	SDL_Surface *tCaption;
	properties_s pCaption;
	properties_s pLblDetails;
	properties_s pLblName;
	properties_s pLblType;
	properties_s pLblSkin;
	properties_s pLblRed;
	properties_s pLblGreen;
	properties_s pLblBlue;
	properties_s pEdtName;
	properties_s pCbbType;
	properties_s pCbbSkin;
	properties_s pSldRed;
	properties_s pSldGreen;
	properties_s pSldBlue;
	properties_s pSkinPreview;
	frame_s pSkinPreviewBorder;
	frame_s pFrame;
};

	// View Players
struct viewplayers_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tBtnNewPlayer;
	properties_s pBtnNewPlayer;
	SDL_Surface *tBtnViewPlayers;
	properties_s pBtnViewPlayers;
	SDL_Surface *tBtnBack;
	properties_s pBtnBack;
	SDL_Surface *tBtnDelete;
	properties_s pBtnDelete;
	SDL_Surface *tBtnApply;
	properties_s pBtnApply;
	SDL_Surface *tCaption;
	properties_s pCaption;
	properties_s pLblDetails;
	properties_s pLblName;
	properties_s pLblType;
	properties_s pLblSkin;
	properties_s pLblRed;
	properties_s pLblGreen;
	properties_s pLblBlue;
	properties_s pEdtName;
	properties_s pCbbType;
	properties_s pCbbSkin;
	properties_s pSldRed;
	properties_s pSldGreen;
	properties_s pSldBlue;
	properties_s pSkinPreview;
	frame_s pSkinPreviewBorder;
	properties_s pLblPlayers;
	properties_s pLsvPlayers;
	frame_s pFrame;
};

	// Level Editor
struct leveleditor_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tBtnClean;
	properties_s pBtnClean;
	SDL_Surface *tBtnStone;
	properties_s pBtnStone;
	SDL_Surface *tBtnBone;
	properties_s pBtnBone;
	SDL_Surface *tBtnDirt;
	properties_s pBtnDirt;
	SDL_Surface *tBtnNew;
	properties_s pBtnNew;
	SDL_Surface *tBtnRandom;
	properties_s pBtnRandom;
	SDL_Surface *tBtnLoad;
	properties_s pBtnLoad;
	SDL_Surface *tBtnSave;
	properties_s pBtnSave;
	SDL_Surface *tBtnQuit;
	properties_s pBtnQuit;
	SDL_Surface *tCaption;
	properties_s pCaption;
	properties_s pLevel;
	properties_s pBrush;
	frame_s pBrushBorder;
	frame_s pFrame;
};

	//
	// Options
	//

	// Controls
struct controlsoptions_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tBtnControls;
	properties_s pBtnControls;
	SDL_Surface *tBtnGame;
	properties_s pBtnGame;
	SDL_Surface *tBtnSystem;
	properties_s pBtnSystem;
	SDL_Surface *tBtnBack;
	properties_s pBtnBack;
	SDL_Surface *tCaption;
	properties_s pCaption;
	properties_s pLblUp;
	properties_s pLblDown;
	properties_s pLblLeft;
	properties_s pLblRight;
	properties_s pLblShoot;
	properties_s pLblJump;
	properties_s pLblSelectWeapon;
	properties_s pLblNinjaRope;
	properties_s pLblChat;
	properties_s pLblScore;
	properties_s pLblPlayer1;
	properties_s pLblPlayer2;
	
	// Player 1
	properties_s pEdtPl1Up;
	properties_s pEdtPl1Down;
	properties_s pEdtPl1Left;
	properties_s pEdtPl1Right;
	properties_s pEdtPl1Shoot;
	properties_s pEdtPl1Jump;
	properties_s pEdtPl1SelectWeapon;
	properties_s pEdtPl1NinjaRope;

	// Player 2
	properties_s pEdtPl2Up;
	properties_s pEdtPl2Down;
	properties_s pEdtPl2Left;
	properties_s pEdtPl2Right;
	properties_s pEdtPl2Shoot;
	properties_s pEdtPl2Jump;
	properties_s pEdtPl2SelectWeapon;
	properties_s pEdtPl2NinjaRope;

	// General
	properties_s pEdtChat;
	properties_s pEdtScore;

	frame_s pFrame;
};


	// Game
struct gameoptions_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tBtnControls;
	properties_s pBtnControls;
	SDL_Surface *tBtnGame;
	properties_s pBtnGame;
	SDL_Surface *tBtnSystem;
	properties_s pBtnSystem;
	SDL_Surface *tBtnBack;
	properties_s pBtnBack;
	SDL_Surface *tCaption;
	properties_s pCaption;
	properties_s pLblBloodAmount;
	properties_s pLblShadows;
	properties_s pLblParticles;
	properties_s pLblOldschoolRope;
	properties_s pSldBloodAmount;
	properties_s pCbxShadows;
	properties_s pCbxParticles;
	properties_s pCbxOldschoolRope;
	frame_s pFrame;
};

	// System
struct systemoptions_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tBtnControls;
	properties_s pBtnControls;
	SDL_Surface *tBtnGame;
	properties_s pBtnGame;
	SDL_Surface *tBtnSystem;
	properties_s pBtnSystem;
	SDL_Surface *tBtnBack;
	properties_s pBtnBack;
	SDL_Surface *tBtnApply;
	properties_s pBtnApply;
	SDL_Surface *tCaption;
	properties_s pCaption;

	// Labels
	properties_s pLblVideo;
	properties_s pLblFullscreen;
	properties_s pLblAudio;
	properties_s pLblSoundOn;
	properties_s pLblSoundVolume;
	properties_s pLblNetwork;
	properties_s pLblNetworkPort;
	properties_s pLblNetworkSpeed;
	properties_s pLblMisc;
	properties_s pLblShowFPS;
	properties_s pLblFilteredLevel;
	properties_s pLblLogConvos;
	
	// Controls
	properties_s pCbxFullscreen;
	properties_s pCbxSoundOn;
	properties_s pSldSoundVolume;
	properties_s pEdtNetworkPort;
	properties_s pCbbNetworkSpeed;
	properties_s pCbxShowFPS;
	properties_s pCbxFiltered;
	properties_s pCbxLogConvos;

	frame_s pFrame;
};


	//
	// In Game
	//

	// Game
struct ingame_s  {
	SDL_Surface *tBarBackground;
	properties_s pBar;

	// Border
	frame_s pBorder;

	// Labels
	properties_s pLblHealth;
	properties_s pLblWeapon;
	properties_s pLblLives;
	properties_s pLblKills;
	properties_s pLblLivesCount;
	properties_s pLblKillsCount;

	// Mini Map
	properties_s pMiniMap;

	// "Progress bars"
	  // Health
	SDL_Surface *tHealthBackground;
	SDL_Surface *tHealthForeground;
	properties_s pHealth;
	frame_s pHealthBorder;

	  // Weapon
	SDL_Surface *tWeaponBackground;
	SDL_Surface *tWeaponForeground;
	SDL_Surface *tWeaponLoading;
	properties_s pWeapon;
	properties_s pWeaponLoading;
	frame_s pWeaponBorder;

	// Chat
	SDL_Surface *tChatBackground;
	properties_s pChat;
	frame_s pChatBorder;
};

	//
	// Dialogs
	//

	// Scoreboard
struct scoreboard_s  {
	SDL_Surface *tBackground;
	frame_s pBorder;

	properties_s pLblPlayers;
	properties_s pLblL;
	properties_s pLblK;

	frame_s pHeaderBorder;
	SDL_Surface *tHeaderSeparator;
	properties_s pHeaderSeparator;
	properties_s pHeader;

	frame_s pItemBorder;
	properties_s pItem;

	SDL_Surface *tListBackground;
	frame_s pListBorder;
	properties_s pList;
};

	// Game Menu
struct gamemenu_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tCaption;
	properties_s pCaption;
	frame_s pBorder;

	properties_s pLblPlayers;
	properties_s pLblLives;
	properties_s pLblKills;

	properties_s pLblWaiting;
	properties_s pLblReady;

	SDL_Surface *tHeaderSeparator;
	frame_s pHeaderBorder;
	properties_s pHeaderSeparator;
	properties_s pHeader;

	frame_s pItemBorder;
	properties_s pItem;

	SDL_Surface *tListBackground;
	frame_s pListBorder;
	properties_s pList;

	SDL_Surface *tBtnResume;
	properties_s pBtnResume;
	SDL_Surface *tBtnQuitGame;
	properties_s pBtnQuitGame;
};

	// Game Over (Local)
struct gameoverlocal_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tCaption;
	properties_s pCaption;
	frame_s pBorder;

	properties_s pLblPlayers;
	properties_s pLblLives;
	properties_s pLblKills;

	SDL_Surface *tHeaderSeparator;
	frame_s pHeaderBorder;
	properties_s pHeaderSeparator;
	properties_s pHeader;

	frame_s pItemBorder;
	properties_s pItem;

	SDL_Surface *tListBackground;
	frame_s pListBorder;
	properties_s pList;

	SDL_Surface *tBtnOk;
	properties_s pBtnOk;
};

	// Game Over (Net)
struct gameovernet_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tCaption;
	properties_s pCaption;
	frame_s pBorder;

	properties_s pLblPlayers;
	properties_s pLblLives;
	properties_s pLblKills;

	SDL_Surface *tHeaderSeparator;
	frame_s pHeaderBorder;
	properties_s pHeaderSeparator;
	properties_s pHeader;

	frame_s pItemBorder;
	properties_s pItem;

	SDL_Surface *tListBackground;
	frame_s pListBorder;
	properties_s pList;

	properties_s pLblReturning;

	SDL_Surface *tBtnLeave;
	properties_s pBtnLeave;
};

	// Viewport Manager
struct viewport_s  {
	SDL_Surface *tBackground;
	SDL_Surface *tCaption;
	properties_s pCaption;
	frame_s pBorder;
	properties_s pGameMenu;

	properties_s pLblViewport1;
	properties_s pLblViewport2;
	properties_s pLblUsed;
	properties_s pLblType;
	properties_s pCbbType1;
	properties_s pCbbType2;
	properties_s pCbxUsed;

	SDL_Surface *tBtnOk;
	properties_s pBtnOk;
};

	// Console
struct console_s  {
	SDL_Surface *tBackground;
	frame_s pBorder;
	properties_s pConsole;

	properties_s pFntTypedText;
	properties_s pFntNote;
};

	// Game Settings
struct gamesettings_s  {
	SDL_Surface *tBackground;
	properties_s pLblCaption;
	frame_s pBorder;
	properties_s pGameSettings;

	properties_s pLblLives;
	properties_s pLblMaxKills;
	properties_s pLblLoadingTime;
	properties_s pLblBonuses;
	properties_s pLblBonusNames;
	properties_s pLblLoadingTimeCount;
	properties_s pEdtLives;
	properties_s pEdtMaxKills;
	properties_s pSldLoadingTime;
	properties_s pCbxBonuses;
	properties_s pCbxBonusNames;

	SDL_Surface *tBtnOk;
	properties_s pBtnOk;

	SDL_Surface *tBtnDefault;
	properties_s pBtnDefault;
};

	// Weapon Options
struct weaponoptions_s  {
	SDL_Surface *tBackground;
	properties_s pLblCaption;
	frame_s pBorder;
	properties_s pWeaponOptions;

	// Stuff
	properties_s pScrollbar;
	properties_s pItem;
	properties_s pItemMouseOver;
	properties_s pItemMouseDown;
	
	// Buttons
	SDL_Surface *tBtnOk;
	properties_s pBtnOk;
	SDL_Surface *tBtnRandom;
	properties_s pBtnRandom;
	SDL_Surface *tBtnCycle;
	properties_s pBtnCycle;
};

	// Ban List
struct banlistgui_s  {
	SDL_Surface *tBackground;
	properties_s pLblCaption;
	frame_s pBorder;
	properties_s pBanList;

	// List
	properties_s pList;
	
	// Buttons
	SDL_Surface *tBtnOk;
	properties_s pBtnOk;
	SDL_Surface *tBtnUnban;
	properties_s pBtnUnban;
	SDL_Surface *tBtnClear;
	properties_s pBtnClear;
};

enum textalign_e {
	Left,
	Right,
	Center
};

enum textvalign_e {
	Top,
	Middle,
	Bottom
};

	// Quit Game dialog
struct quitgame_s  {
	SDL_Surface *tBackground;
	properties_s pLblCaption;
	frame_s pBorder;
	properties_s pQuitGame;

	
	// Text
	properties_s pText;
	textalign_e pTextAlign;
	textvalign_e pTextValign;
	

	// Buttons
	SDL_Surface *tBtnYes;
	properties_s pBtnYes;
	SDL_Surface *tBtnNo;
	properties_s pBtnNo;
};

	// Standard dialog
struct dialog_s  {
	SDL_Surface *tBackground;
	properties_s pLblCaption;
	SDL_Surface *tCaption;
	properties_s pCaption;
	frame_s pBorder;
	properties_s pDialog;

	
	// Text
	properties_s pText;
	textalign_e pTextAlign;
	textvalign_e pTextValign;

	// Buttons
	SDL_Surface *tBtnOk;
	properties_s pBtnOk;
};

	//
	//	Widgets
	//

	// Checkbox
struct checkbox_s  {
	SDL_Surface *tCheckbox;
	properties_s pCheckbox;
};

	// Combo Box
struct combobox_s  {
	SDL_Surface *tArrow;
	properties_s pCombobox;
	properties_s pArrow;
	properties_s pFont;
	properties_s pDisabledFont;
	frame_s pBorder;
};

	// Input Box
struct inputbox_s  {
	SDL_Surface *tInputbox;
	properties_s pInputbox;
	properties_s pFont;
	frame_s pBorder;
	frame_s pBorderMouseOver;
	frame_s pBorderMouseDown;
};

	// List View
struct listview_s  {
	properties_s pListview;
	properties_s pItem;
	properties_s pFont;
	properties_s pDisabledFont;
	frame_s pBorder;
};

	// Popup Menu
struct popupmenu_s  {
	properties_s pMenu;
	//SDL_Surface *pItemBackground;
	properties_s pItem;
	//SDL_Surface *pItemOverBackground;
	properties_s pItemOver;
	properties_s pFont;
	properties_s pFontOver;
	frame_s pBorder;
};

	// Scrollbar
struct scrollbar_s  {
	SDL_Surface *tArrow;
	SDL_Surface *tBody;
	SDL_Surface *tBackground;
	properties_s pScrollbar;
	properties_s pScrollbarOver;
	properties_s pScrollbarDown;
	frame_s pBorder;
	frame_s pBorderOver;
	frame_s pBorderDown;
};

	// Slider
struct slider_s  {
	properties_s pSlider;
	SDL_Surface *tCursor;
	SDL_Surface *tStop;
	SDL_Surface *tLine;
};

	// Text Box
struct textbox_s  {
	properties_s pTextbox;
	properties_s pFont;
	frame_s pBorder;
};

	// Main Game Layout Properties
struct main_s  {
	properties_s pFntNormal;
	properties_s pFntChat;
	properties_s pFntNotice;
	properties_s pFntNetwork;

	SDL_Surface *tMouseArrow;
	properties_s pMouseArrow;
	SDL_Surface *tMouseHand;
	properties_s pMouseHand;
	SDL_Surface *tMouseText;
	properties_s pMouseText;

	SDL_Surface *tFont1;
	SDL_Surface *tFont2;
	SDL_Surface *tFont3;
};




/*class CGUISkin {
public:
	// Constructor
	CGUISkin() {
		tMainMenu = NULL;
		tLocalLobby = NULL;
		tInternet = NULL;
		tLAN = NULL;
		tHost = NULL;
		tHostLobby = NULL;
		tClientLobby = NULL;
		tNewPlayer = NULL;
		tViewPlayers = NULL;
		tLevelEditor = NULL;
		tControlsOptions = NULL;
		tGameOptions = NULL;
		tSystemOptions = NULL;
	}


private:
	// Attributes*/
/*
	// Screens
extern  mainmenu_s			GUI_MainMenu;
extern  locallobby_s		GUI_LocalLobby;
extern 	internet_s			GUI_Internet;
extern 	lan_s				GUI_LAN;
extern 	host_s				GUI_Host;
extern 	hostlobby_s			GUI_HostLobby;
extern 	clientlobby_s		GUI_ClientLobby;
extern 	newplayer_s			GUI_NewPlayer;
extern 	viewplayers_s		GUI_ViewPlayers;
extern 	leveleditor_s		GUI_LevelEditor;
extern 	controlsoptions_s	GUI_ControlsOptions;
extern 	gameoptions_s		GUI_GameOptions;
extern 	systemoptions_s		GUI_SystemOptions;
extern 	connecting_s		GUI_Connecting;
extern 	connect_s			GUI_Connect;
extern 	ingame_s			GUI_InGame;

	// Dialogs
extern 	scoreboard_s		GUI_ScoreBoard;
extern 	gamemenu_s			GUI_GameMenu;
extern 	gameoverlocal_s		GUI_GameOverLocal;
extern 	gameovernet_s		GUI_GameOverNet;
extern 	viewport_s			GUI_Viewport;
extern 	console_s			GUI_Console;
extern 	gamesettings_s		GUI_GameSettings;
extern 	weaponoptions_s		GUI_WeaponOptions;
extern  banlistgui_s		GUI_BanList;
extern 	quitgame_s			GUI_QuitGame;
extern 	dialog_s			GUI_Dialog;

	// Widgets
extern 	checkbox_s			GUI_Checkbox;
extern 	combobox_s			GUI_Combobox;
extern 	inputbox_s			GUI_Inputbox;
extern 	listview_s			GUI_Listview;
extern 	popupmenu_s			GUI_Popupmenu;
extern 	scrollbar_s			GUI_Scrollbar;
extern 	slider_s			GUI_Slider;
extern 	textbox_s			GUI_Textbox;
extern 	main_s				GUI_Main;


extern 	bool				GUI_bMainMenuLoaded;
extern 	bool				GUI_bLocalLobbyLoaded;
extern 	bool				GUI_bInternetLoaded;
extern 	bool				GUI_bLanLoaded;
extern 	bool				GUI_bHostLoaded;
extern 	bool				GUI_bHostLobbyLoaded;
extern 	bool				GUI_bClientLobbyLoaded;
extern 	bool				GUI_bNewPlayerLoaded;
extern 	bool				GUI_bViewPlayersLoaded;
extern 	bool				GUI_bLevelEditorLoaded;
extern 	bool				GUI_bControlOptionsLoaded;
extern 	bool				GUI_bGameOptionsLoaded;
extern 	bool				GUI_bSystemOptionsLoaded;
extern 	bool				GUI_bInGameLoaded;
extern 	bool				GUI_bGameDialogsLoaded;
extern 	bool				GUI_bNonGameDialogsLoaded;
extern 	bool				GUI_bWidgetsLoaded;



//public:
	// Methods
	bool	 GUI_LoadMain(void);
	bool	 GUI_LoadMainMenu(void);
	bool	 GUI_LoadLocalLobby(void);
	bool	 GUI_LoadInternet(void);
	bool	 GUI_LoadLAN(void);
	bool	 GUI_LoadHost(void);
	bool	 GUI_LoadHostLobby(void);
	bool	 GUI_LoadClientLobby(void);
	bool	 GUI_LoadNewPlayer(void);
	bool	 GUI_LoadViewPlayers(void);
	bool	 GUI_LoadLevelEditor(void);
	bool	 GUI_LoadControlsOptions(void);
	bool	 GUI_LoadMainGameOptions(void);
	bool	 GUI_LoadMainSystemOptions(void);
	bool	 GUI_LoadInGame(void);
	bool	 GUI_LoadGameDialogs(void);
	bool	 GUI_LoadNonGameDialogs(void);
	bool	 GUI_LoadWidgets(void);
//};




#endif  //  __CGUISKIN_H__*/