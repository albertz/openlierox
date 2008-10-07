/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Menu header file
// Created 30/6/02
// Jason Boettcher


#ifndef __MENU_H__DEPRECATED_GUI__
#define __MENU_H__DEPRECATED_GUI__

#include <SDL.h>
#include <string>

#include "DeprecatedGUI/CWidgetList.h"
#include "DeprecatedGUI/CCombobox.h"
#include "DeprecatedGUI/CInputBox.h"
#include "Networking.h"
#include "CBytestream.h"
#include "DeprecatedGUI/CListview.h"
#include "CWpnRest.h"
#include "CClient.h" // for MAX_PLAYERS
#include "CWormSkin.h"

namespace DeprecatedGUI {

enum {
// Menu sockets
	SCK_LAN = 0,
	SCK_NET = 1,
	SCK_FOO = 2,
// Serverlist timeout
	SVRLIST_TIMEOUT =  7000, 
// Server info dialog dimensions
	INFO_W = 350,
	INFO_H = 390,

	MAX_QUERIES = 3
};


// Menu types
enum {
	MNU_MAIN=0,
	MNU_LOCAL,
	MNU_NETWORK,
	MNU_PLAYER,
	MNU_OPTIONS,
	MNU_MAPED,
	MNU_GUISKIN
};


// Layout IDs
enum {
	L_MAINMENU=0,
	L_LOCALPLAY,
	L_GAMESETTINGS,
	L_WEAPONOPTIONS,
	L_LOADWEAPONS,
	L_SAVEWEAPONS,
	L_NET,
	L_NETINTERNET,
	L_INTERNETDETAILS,
	L_ADDSERVER,
	L_NETLAN,
	L_LANDETAILS,
	L_NETHOST,
	L_NETFAVOURITES,
	L_FAVOURITESDETAILS,
	L_RENAMESERVER,
	L_ADDFAVOURITE,
	L_CONNECTING,
	L_NETJOINLOBBY,
	L_NETHOSTLOBBY,
	L_SERVERSETTINGS,
	L_BANLIST,
	L_PLAYERPROFILES,
	L_CREATEPLAYER,
	L_VIEWPLAYERS,
	L_LEVELEDITOR,
	L_NEWDIALOG,
	L_SAVELOADLEVEL,
	L_OPTIONS,
	L_OPTIONSCONTROLS,
	L_OPTIONSGAME,
	L_OPTIONSSYSTEM,
	L_MESSAGEBOXOK,
	L_MESSAGEBOXYESNO,
	LAYOUT_COUNT
};

// Box types
enum {
	BX_OUTSET = 0,
	BX_INSET,
	BX_SOLID
};


// Sub title id's
enum {
	SUB_LOCAL=0,
	SUB_NETWORK,
	SUB_PLAYER,
	SUB_MAPED,
	SUB_OPTIONS,
	SUB_LOBBY
};


// Message box types
enum {
	LMB_OK = 0,
	LMB_YESNO,

	// Return types
	MBR_OK=0,
	MBR_YES,
	MBR_NO
};


// Buttons
enum {
	BUT_MAIN=0,
	BUT_INTERNET,
	BUT_LAN,
	BUT_HOST,
	BUT_FAVOURITES,
	BUT_BACK,
	BUT_OK,
	BUT_CREATE,
	BUT_NEW,
	BUT_RANDOM,
	BUT_LOAD,
	BUT_SAVE,
	BUT_QUIT,
	BUT_CANCEL,
	BUT_QUITGAME,
	BUT_PLAYAGAIN,
	BUT_YES,
	BUT_NO,
	BUT_CONTROLS,
	BUT_GAME,
	BUT_SYSTEM,
	BUT_APPLY,
	BUT_RESUME,
	BUT_NEWPLAYER,
	BUT_VIEWPLAYERS,
	BUT_DELETE,
	BUT_START,
	BUT_JOIN,
	BUT_REFRESH,
	BUT_ADD,
	BUT_READY,
	BUT_LEAVE,
	BUT_GAMESETTINGS,
	BUT_UPDATELIST,
    BUT_WEAPONOPTIONS,
    BUT_RESET,
    BUT_DEFAULT,
	BUT_BANNED,
	BUT_UNBAN,
	BUT_CLEAR,
	BUT_SERVERSETTINGS,
	BUT_FILTER,
	BUT_ADDTOFAVOURITES,
	BUT_NEWS,
	BUT_GENERAL,
	BUT_BONUS,
};

// Frontend info
class frontendinfo_t { public:
	int				iMainTitlesLeft;
	int				iMainTitlesTop;
	int				iMainTitlesSpacing;
	int				iCreditsLeft;
	int				iCreditsTop;
	int				iCreditsSpacing;
	bool			bPageBoxes;
	std::string		sFrontendCredits;
	float			fLoadingAnimFrameTime;
	int				iLoadingAnimLeft;
	int				iLoadingAnimTop;
	int				iLoadingBarLeft;
	int				iLoadingBarTop;
	int				iLoadingLabelLeft;
	int				iLoadingLabelTop;
};

// Menu structure
class menu_t { public:

	// Graphics
	//SmartPointer<SDL_Surface> bmpMainBack;
    //SmartPointer<SDL_Surface> bmpMainBack_lg;
    SmartPointer<SDL_Surface> bmpMainBack_wob;
	SmartPointer<SDL_Surface> bmpMainBack_common;
	SmartPointer<SDL_Surface> bmpBuffer;

	SmartPointer<SDL_Surface> bmpMsgBuffer;
    SmartPointer<SDL_Surface> bmpMiniMapBuffer;

	SmartPointer<SDL_Surface> bmpLieroXtreme;
	SmartPointer<SDL_Surface> bmpMainTitles;
	SmartPointer<SDL_Surface> bmpTitles;
	SmartPointer<SDL_Surface> bmpSubTitles;
	SmartPointer<SDL_Surface> bmpButtons;
	SmartPointer<SDL_Surface> bmpCheckbox;
	SmartPointer<SDL_Surface> bmpInputbox;

	SmartPointer<SDL_Surface> bmpMainLocal;
	SmartPointer<SDL_Surface> bmpMainNet;
	SmartPointer<SDL_Surface> bmpLobbyReady;
	SmartPointer<SDL_Surface> bmpLobbyNotReady;
	SmartPointer<SDL_Surface> bmpHost;
	SmartPointer<SDL_Surface> bmpConnectionSpeeds[5];
	SmartPointer<SDL_Surface> bmpSpeech;
    SmartPointer<SDL_Surface> bmpHandicap;
	SmartPointer<SDL_Surface> bmpDownload;

	SmartPointer<SDL_Surface> bmpTriangleUp;
	SmartPointer<SDL_Surface> bmpTriangleDown;

	SmartPointer<SDL_Surface> bmpAI;
	CWormSkin cSkin;

	SmartPointer<SDL_Surface> bmpMapEdTool;

	// Other
	bool			bMenuRunning;
	int				iMenuType;
	frontendinfo_t	tFrontendInfo;
	std::string		sSavedChatText;
	CWorm			sLocalPlayers[MAX_PLAYERS];

	// Map Editor
	int				iEditMode;
	int				iCurHole;
	int				iCurStone;
	int				iCurMisc;
	int				iCurDirt;

	int				iReturnTo;

	// Socket for pinging
	NetworkSocket		tSocket[3];

};


// Net menu types
enum {
	net_main=0,
	net_internet,
	net_lan,
	net_host,
	net_favourites,
	net_news,
	net_join
};


// Server structure
// TODO: make std::list instead of linked list here
class server_t { public:
	server_t() {
		SetNetAddrValid(sAddress, false);
		bAllowConnectDuringGame = false;
	}

	bool	bIgnore;
	bool	bProcessing;
    bool    bManual;
	int		nPings;
	int		nQueries;
	bool	bgotPong;
	bool	bgotQuery;
	float	fInitTime;
	bool	bAddrReady;
	float	fLastPing;
	float	fLastQuery;
    float   fQueryTimes[MAX_QUERIES+1];

	// Server address
	std::string	szAddress;
	NetworkAddr	sAddress;

	// Server details
	std::string	szName;
	int		nState;
	int		nNumPlayers;
	int		nMaxPlayers;
	int		nPing;
	bool	bAllowConnectDuringGame;
	Version tVersion;

    server_t	*psPrev;
	server_t	*psNext;
};


// Menu globals
extern	menu_t		*tMenu;
extern	bool		*bGame;
extern	int			iNetMode;
extern	int			iJoinMenu;
extern	int			iHostType;
extern	int			iSkipStart;
extern  bool		bHost_Update;
extern	bool		bJoin_Update;
extern  CWidgetList	LayoutWidgets[LAYOUT_COUNT];

extern	bool		bGotDetails;
extern	bool		bOldLxBug;
extern	int			nTries;
extern	float		fStart;
extern bool		bShowFloatingOptions;


// Routines
bool	Menu_Initialize(bool *game);
void	Menu_LoadFrontendInfo();
void	Menu_Shutdown(void);
void	Menu_Start(void);
void	Menu_RedrawMouse(int total);
void	Menu_Loop(void);
void    Menu_SetSkipStart(int s);
void	Menu_DrawSubTitle(SDL_Surface * bmpDest, int id);
void    Menu_DrawSubTitleAdv(SDL_Surface * bmpDest, int id, int y);
void	Menu_DrawBox(SDL_Surface * bmpDest, int x, int y, int x2, int y2);
void	Menu_DrawBoxAdv(SDL_Surface * bmpDest, int x, int y, int x2, int y2, int border, Uint32 LightColour, Uint32 DarkColour, Uint32 BgColour, uchar type);
void    Menu_DrawBoxInset(SDL_Surface * bmpDest, int x, int y, int x2, int y2);
void    Menu_DrawWinButton(SDL_Surface * bmpDest, int x, int y, int w, int h, bool down);
bool	Menu_LoadWormGfx(profile_t *ply);
int		Menu_MessageBox(const std::string& sTitle, const std::string& sText, int type);
void	Menu_AddDefaultWidgets(void);
void	Menu_FillLevelList(CCombobox *cmb, int random);
void    Menu_redrawBufferRect(int x, int y, int w, int h);
std::string	Menu_GetLevelName(const std::string& filename, bool abs_filename = false); // TODO: move this from Menu to CMap

// Server list
void		Menu_SvrList_Clear(void);
void        Menu_SvrList_ClearAuto(void);
void		Menu_SvrList_Shutdown(void);
void		Menu_SvrList_PingLAN(void);
server_t	*Menu_SvrList_AddServer(const std::string& address, bool bManual);
server_t	*Menu_SvrList_AddNamedServer(const std::string& address, const std::string& name);
server_t    *Menu_SvrList_FindServerStr(const std::string& szAddress);
void        Menu_SvrList_RemoveServer(const std::string& szAddress);
bool		Menu_SvrList_Process(void);
bool		Menu_SvrList_ParsePacket(CBytestream *bs, NetworkSocket sock);
server_t	*Menu_SvrList_FindServer(const NetworkAddr& addr);
void		Menu_SvrList_PingServer(server_t *svr);
void		Menu_SvrList_WantsJoin(const std::string& Nick, server_t *svr);
void		Menu_SvrList_QueryServer(server_t *svr);
void		Menu_SvrList_ParseQuery(server_t *svr, CBytestream *bs);
void		Menu_SvrList_ParseUdpServerlist(CBytestream *bs);
void		Menu_SvrList_RefreshList(void);
void        Menu_SvrList_RefreshServer(server_t *s);
void		Menu_SvrList_UpdateList(void);
void		Menu_SvrList_UpdateUDPList();
void		Menu_SvrList_FillList(CListview *lv);
void        Menu_SvrList_SaveList(const std::string& szFilename);
void        Menu_SvrList_LoadList(const std::string& szFilename);
void        Menu_SvrList_DrawInfo(const std::string& szAddress, int w, int h);
void		Menu_SvrList_AddFavourite(const std::string& szName, const std::string& szAddress);
// Returns true if server is registered on UDP masterserver and won't respond on pinging
bool		Menu_SvrList_ServerBehindNat(const std::string& szAddress);

// Main menu
void	Menu_MainInitialize(void);
void	Menu_MainShutdown(void);
void	Menu_MainFrame(void);
void	Menu_MainDrawTitle(int x, int y, int id, int selected);


// Local menu
void	Menu_LocalInitialize(void);
void	Menu_LocalFrame(void);
void	Menu_LocalAddProfiles(void);
void	Menu_LocalStartGame(void);
bool	Menu_LocalCheckPlaying(int index);
void	Menu_Local_FillModList( CCombobox *cb );
void	Menu_LocalShowMinimap(bool bReload);
void	Menu_LocalAddPlaying(int index = -1);
void	Menu_LocalRemovePlaying(int index = -1);
void	Menu_LocalShutdown(void);


// Player menu
void	Menu_PlayerInitialize(void);
void	Menu_PlayerFrame(void);
void    Menu_Player_NewPlayerInit(void);
void    Menu_Player_ViewPlayerInit(void);
void	Menu_Player_NewPlayer(int mouse);
void	Menu_Player_ViewPlayers(int mouse);
void	Menu_Player_AddPlayer(const std::string& sName, Uint8 r, Uint8 g, Uint8 b);
void    Menu_Player_DrawWormImage(SDL_Surface * bmpDest, int Frame, int dx, int dy, int ColR, int ColG, int ColB);
void	Menu_Player_DeletePlayer(int index);
void    Menu_Player_FillSkinCombo(CCombobox *cb);
void	Menu_PlayerShutdown(void);

// Map editor
bool	Menu_MapEdInitialize(void);
void	Menu_MapEdFrame(SDL_Surface * bmpDest, int process);
void	Menu_MapEd_New(void);
void	Menu_MapEd_LoadSave(int save);
bool	Menu_MapEd_OkSave(const std::string& szFilename);
void	Menu_MapEdShutdown(void);

// Game Settings
void	Menu_GameSettings(void);
void	Menu_GameSettingsShutdown(void);
bool	Menu_GameSettings_Frame(void);
void	Menu_GameSettings_GrabInfo(void);
void    Menu_GameSettings_Default(void);


// Weapons Restrictions
void    Menu_WeaponsRestrictions(const std::string& szMod);
bool    Menu_WeaponsRestrictions_Frame(void);
void	Menu_WeaponsRestrictionsShutdown(void);

// Load/save dialog
void	Menu_WeaponPresets(bool save, CWpnRest *gamescript);
bool	Menu_WeaponPresetsOkSave(const std::string& szFilename);
void	Menu_WeaponPresetsShutdown(void);

// Ban List
void	Menu_BanList(void);
bool	Menu_BanList_Frame(void);
void	Menu_BanListShutdown(void);

// Server settings
void	Menu_ServerSettings(void);
bool	Menu_ServerSettings_Frame(void);
void	Menu_ServerSettingsShutdown(void);


// Options
bool	Menu_OptionsInitialize(void);
void	Menu_OptionsShutdown(void);
void	Menu_OptionsFrame(void);
void	Menu_OptionsWaitInput(int ply, const std::string& name, CInputbox *b);

// Floating Options
bool	Menu_FloatingOptionsInitialize(void);
void	Menu_FloatingOptionsShutdown(void);
void	Menu_FloatingOptionsFrame(void);
void	Menu_FloatingOptionsWaitInput(int ply, const std::string& name, CInputbox *b);


// Main net
bool	Menu_NetInitialize(void);
void	Menu_Net_GotoHostLobby(void);
void	Menu_Net_GotoJoinLobby(void);
void	Menu_NetFrame(void);
void	Menu_NetShutdown(void);


// Net::Main menu
bool	Menu_Net_MainInitialize(void);
void	Menu_Net_MainFrame(int mouse);
void	Menu_Net_MainShutdown(void);


// Net::Host menu
bool	Menu_Net_HostInitialize(void);
void	Menu_Net_HostShutdown(void);
void	Menu_Net_HostFrame(int mouse);
void	Menu_Net_HostPlyFrame(int mouse);
void	Menu_Net_HostPlyShutdown(void);

bool	Menu_Net_HostLobbyInitialize(void);
void    Menu_Net_HostLobbyDraw(void);
void    Menu_Net_HostLobbyCreateGui(void);
void	Menu_Net_HostGotoLobby(void);
void	Menu_Net_HostLobbyFrame(int mouse);
void	Menu_Net_HostLobbyShutdown(void);
std::string	Menu_Net_HostLobbyGetText(void);
void Menu_Net_HostLobbySetText(const std::string& str);
void	Menu_HostDrawLobby(SDL_Surface * bmpDest);
void	Menu_HostShowMinimap(void);
void	Menu_Net_HostDeregister(void);

void Menu_HostActionsPopupMenuInitialize( CGuiLayout & layout, int id_PopupMenu, int id_PopupPlayerInfo, int wormid );
void Menu_HostActionsPopupMenuClick(CGuiLayout & layout, int id_PopupMenu, int id_PopupPlayerInfo, int wormid, int menuItem);
void Menu_HostActionsPopupPlayerInfoClick(CGuiLayout & layout, int id_PopupMenu, int id_PopupPlayerInfo, int wormid, int menuItem);


// Net::LAN menu
bool	Menu_Net_LANInitialize(void);
void	Menu_Net_LANShutdown(void);
void	Menu_Net_LANFrame(int mouse);
void	Menu_Net_LANSendPing(void);
void	Menu_Net_LANJoinServer(const std::string& sAddress, const std::string& sName);
void    Menu_Net_LanShowServer(const std::string& szAddress);


// Net::Joining menu
bool	Menu_Net_JoinInitialize(const std::string& sAddress);
void	Menu_Net_JoinShutdown(void);
void	Menu_Net_JoinFrame(int mouse);

// Net::Favourites menu
bool	Menu_Net_FavouritesInitialize(void);
void	Menu_Net_FavouritesShutdown(void);
void	Menu_Net_FavouritesFrame(int mouse);
void	Menu_Net_FavouritesJoinServer(const std::string& sAddress, const std::string& sName);
void	Menu_Net_FavouritesShowServer(const std::string& szAddress);
void	Menu_Net_RenameServer(std::string& szName);
void	Menu_Net_FavouritesAddServer(void);

// Net::News menu
bool	Menu_Net_NewsInitialize(void);
void	Menu_Net_NewsShutdown();
void	Menu_Net_NewsFrame(int mouse);

// deprecated
//int	Menu_Net_JoinInitializePlayers(void);
//void	Menu_Net_JoinPlayersFrame(int mouse);

bool	Menu_Net_JoinConnectionInitialize(const std::string& sAddress);
void	Menu_Net_JoinConnectionFrame(int mouse);
void	Menu_Net_JoinConnectionShutdown(void);

bool	Menu_Net_JoinLobbyInitialize(void);
void    Menu_Net_JoinDrawLobby(void);
void    Menu_Net_JoinLobbyCreateGui(void);
void	Menu_Net_JoinGotoLobby(void);
std::string	Menu_Net_JoinLobbyGetText();
void	Menu_Net_JoinLobbySetText(const std::string& str);
void	Menu_Net_JoinLobbyFrame(int mouse);
void	Menu_Net_JoinLobbyShutdown(void);


// Net::Internet menu
bool	Menu_Net_NETInitialize(void);
void	Menu_Net_NETShutdown(void);
void	Menu_Net_NETFrame(int mouse);
void	Menu_Net_NETJoinServer(const std::string& sAddress, const std::string& sName);
void	Menu_Net_NETAddServer(void);
void	Menu_Net_NETUpdateList(void);
void	Menu_Net_NETParseList(class CHttp& http);
void    Menu_Net_NETShowServer(const std::string& szAddress);

// CGuiSkin menu - when GUI skinning system will be complete (hopefully) this will become the main menu
bool	Menu_CGuiSkinInitialize(void);
void	Menu_CGuiSkinFrame(void);
void	Menu_CGuiSkinShutdown(void);

}; // namespace DeprecatedGUI

#endif  //  __MENU_H__DEPRECATED_GUI__
