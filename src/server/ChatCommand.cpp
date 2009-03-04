/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Chat command handling
// Created 24/12/07
// Karel Petranek

#include "StringUtils.h"
#include "Utils.h"
#include "ChatCommand.h"
#include "Protocol.h"
#include "CServer.h"
#include "CWorm.h"
#include "CServerConnection.h"
#include "CServerNetEngine.h"
#include "DedicatedControl.h"
#include "CClient.h"
#include "CClientNetEngine.h"
#include "DeprecatedGUI/Menu.h"
#include "FindFile.h"
#include "Debug.h"


//////////////////
// Known commands
ChatCommand tKnownCommands[] = {
	/* command */	/* alias */	 /* MIN*//*MAX*/	/* repeat*/ /* processing function */
	{"authorise",	"authorize",	2, 2,			(size_t)-1,	&ProcessAuthorise},
	{"kick",		"kick",			2, (size_t)-1,	(size_t)-1,	&ProcessKick},
	{"ban",			"ban",			2, (size_t)-1,	(size_t)-1,	&ProcessBan},
	{"mute",		"mute",			2, 2,			(size_t)-1,	&ProcessMute},
	{"unmute",		"unmute",		2, 2,			(size_t)-1,	&ProcessUnmute},
	{"private",		"pm",			3, (size_t)-1,	2,			&ProcessPrivate},
	{"teamchat",	"teampm",		1, (size_t)-1,	0,			&ProcessTeamChat},
	{"me",			"me",			1, (size_t)-1,	0,			&ProcessMe},
	{"setmyname",	"setmyname",	1, (size_t)-1,	(size_t)-1,	&ProcessSetMyName},
	{"setname",		"setname",		3, (size_t)-1,	(size_t)-1,	&ProcessSetName},
	{"setmyskin",	"setmyskin",	1, (size_t)-1,	(size_t)-1,	&ProcessSetMySkin},
	{"setskin",		"setskin",		3, (size_t)-1,	(size_t)-1,	&ProcessSetSkin},
	{"setmycolour",	"setmycolor",	3, 3,			(size_t)-1,	&ProcessSetMyColour},
	{"setcolour",	"setcolor",		5, 5,			(size_t)-1,	&ProcessSetColour},
	{"suicide",		"suicide",		0, 1,			(size_t)-1,	&ProcessSuicide},
	{"spectate",	"spectate",		0, 0,			(size_t)-1,	&ProcessSpectate},
	{"login",		"logon",		1, 1,			(size_t)-1, &ProcessLogin},
	{"start",		"start",		0, 0,			(size_t)-1, &ProcessStart},
	{"lobby",		"lobby",		0, 0,			(size_t)-1, &ProcessLobby},
	{"mod",			"mod",			1, (size_t)-1,	(size_t)-1, &ProcessMod},
	{"level",		"level",		1, (size_t)-1,	(size_t)-1, &ProcessLevel},
	{"loadingtime",	"lt",			1, 1,			(size_t)-1, &ProcessLt},
	{"dedicated",	"ded",			1, (size_t)-1,	(size_t)-1, &ProcessDedicated},
	{"",			"",				0, 0,			(size_t)-1, NULL}
};

/////////////////////
// Get the command based on name
ChatCommand *GetCommand(const std::string& name)
{
	for (uint i=0; tKnownCommands[i].tProcFunc != NULL; ++i)
		if (stringcasecmp(name, tKnownCommands[i].sName) == 0 ||
			stringcasecmp(name, tKnownCommands[i].sAlias) == 0)
			return &tKnownCommands[i];

	// Not found
	return NULL;
}

/////////////////////
// Get the command based on function
ChatCommand *GetCommand(ChatCommand::tProcFunc_t func)
{
	for (uint i=0; tKnownCommands[i].tProcFunc != NULL; ++i)
		if (tKnownCommands[i].tProcFunc == func)
			return &tKnownCommands[i];

	// Not found
	return NULL;
}

static inline std::string ReadQuotedParam(const std::string& str, int *param_size)  {
	*param_size = 0;
	if (str.size() == 0)
		return "";
	if (*str.begin() != '\"')
		return "";

	std::string::const_iterator it = str.begin();
	bool was_backslash = false;
	std::string result;

	// Leading quote
	*param_size = 1;
	it++;

	for (; it != str.end(); it++, (*param_size)++)  {
		switch (*it)  {
			case '\\':
				if (was_backslash)  {
					result += '\\';
					was_backslash = false;
				} else
					was_backslash = true;
			break;

			case '\"':
				if (was_backslash)  {
					result += '\"';
					was_backslash = false;
				} else  {
					(*param_size)++;
					return result;
				}
			break;

			default:
				result += *it;
				was_backslash = false;
		}
	}

	(*param_size)++;
	return result;
}

/////////////////////
// Main command parsing function
const std::vector<std::string>& ParseCommandMessage(const std::string& msg, bool ignore_blank_params)
{
	static std::vector<std::string> result;
	result.clear();

	// Checks
	if (msg == "")
		return result;

	if (*msg.begin() != '/')
		return result;

	std::string::const_iterator it = msg.begin();
	it++; // Skip /

	// Parse
	while (it != msg.end())  { // it++ - skip the space
		std::string p;

		if (*it == '\"')  {
			int skip;
			p = ReadQuotedParam(std::string(it, msg.end()), &skip);
			SafeAdvance(it, skip, msg.end());
		} else if (*it == ' ')  {
			it++;
			continue;
		}
		else  {
			p = ReadUntil(std::string(it, msg.end()), ' ');
			SafeAdvance(it, p.size(), msg.end());
		}

		// Add it
		if (result.size() == 0 && ignore_blank_params)
			continue;

		result.push_back(p);
	}

	return result;
}

///////////////////
// Check and convert the string ID
bool ConvertID(const std::string& str_id, /*out*/ int *id)
{
	// ID check
	bool fail = false;
	int p_id = from_string<int>(str_id, fail);
	if (fail || p_id < 0 || p_id >= MAX_WORMS)
		return false;

	*id = p_id;
	return true;
}

///////////////////
// Common param check used in many cases
std::string CheckIDParams(const std::vector<std::string>& params, ChatCommand::tProcFunc_t func, /*out*/ int *id)
{
	*id = 0;

	// Param count
	ChatCommand *me = GetCommand(func);
	if (params.size() < me->iMinParamCount || params.size() > me->iMaxParamCount)
		return "Invalid parameter count";

	// ID presence check
	if (stringcasecmp(params[0], "id") != 0)
		return "Please specify a worm ID";

	// ID check
	if (!ConvertID(params[1], id))
		return "Invalid worm ID";

	return "";
}

std::string ProcessAuthorise(const std::vector<std::string>& params, int sender_id)
{
	int id = 0;

	// Param check
	std::string ch = CheckIDParams(params, &ProcessAuthorise, &id);
	if (ch.size() != 0)
		return ch;

	// Is the player authorized?
	CServerConnection *sender = cServer->getClient(sender_id);
	if (sender)  {
		if (!sender->getRights()->Authorize)
			return "You do not have enough privileges to authorize a player.";
	}

	// Authorise the client
	CServerConnection *remote_cl = cServer->getClient(id);
	if (remote_cl)  {
		remote_cl->getRights()->Everything();
		cServer->SendGlobalText((cServer->getWorms() + id)->getName() + " has been authorised", TXT_NORMAL);
		return "";
	}

	return "No corresponding client found";
}


////////////////////
// Kick or ban the client
// The two functions were so similar that I merged them in one
enum {
	ACT_KICK,
	ACT_BAN
};

std::string ProcessKickOrBan(const std::vector<std::string>& params, int sender_id, int action)
{
	int id;

	// Param check
	std::string ch = CheckIDParams(params, &ProcessKick, &id);
	if (ch.size() != 0)
		return ch;

	CServerConnection *target = cServer->getClient(id);
	if(target)
		if (target->isLocalClient())
			return action == ACT_KICK ? "Cannot kick host" : "Cannot ban host";

	// Get the reason if specified
	std::string reason;
	if (params.size() >= 3)  {
		std::vector<std::string>::const_iterator it = params.begin() + 2;
		for (; it != params.end(); it++)  {
			reason += *it;
			reason += ' ';
		}
		reason.erase(reason.size() - 1);  // Remove the last space
	}

	// Kick/ban
	CServerConnection *remote_cl = cServer->getClient(sender_id);
	if (remote_cl)  {
		bool rights = action == ACT_KICK ? remote_cl->getRights()->Kick : remote_cl->getRights()->Ban;
		if (rights)
			if (action == ACT_KICK)
				cServer->kickWorm(id, reason);
			else
				cServer->banWorm(id, reason);
		else
			if (action == ACT_KICK)
				return "You don't have privilges to kick the player";
			else
				return "You don't have privilges to ban the player";
	}

	return "";
}

std::string ProcessKick(const std::vector<std::string>& params, int sender_id)
{
	return ProcessKickOrBan(params, sender_id, ACT_KICK);
}

std::string ProcessBan(const std::vector<std::string>& params, int sender_id)
{
	return ProcessKickOrBan(params, sender_id, ACT_BAN);
}

/////////////////////
// Mute or unmute the worm (another merge of two similar functions)
std::string ProcessMuteUnmute(const std::vector<std::string>& params, int sender_id, bool mute)
{
	int id;

	// Param check
	std::string ch = CheckIDParams(params, &ProcessKick, &id);
	if (ch.size() != 0)
		return ch;

	// (un)mute
	CServerConnection *remote_cl = cServer->getClient(sender_id);
	if (remote_cl)  {
		if (remote_cl->getRights()->Mute)
			if (mute)
				cServer->muteWorm(id);
			else
				cServer->unmuteWorm(id);
		else
			return "You don't have privilges to mute the player";
	}

	return "";
}

std::string ProcessMute(const std::vector<std::string>& params, int sender_id)
{
	return ProcessMuteUnmute(params, sender_id, true);
}

std::string ProcessUnmute(const std::vector<std::string>& params, int sender_id)
{
	return ProcessMuteUnmute(params, sender_id, false);
}


std::string ProcessPrivate(const std::vector<std::string>& params, int sender_id)
{
	// Param check
	int p_id;
	std::string ch = CheckIDParams(params, &ProcessPrivate, &p_id);
	if (ch.size() != 0)
		return ch;

	// Get the sender & recipient
	CServerConnection *sender = cServer->getClient(sender_id);
	CServerConnection *recipient = cServer->getClient(p_id);
	if (!sender || !recipient)
		return "Message could not be sent";

	// Get the message
	std::string msg = sender->getWorm(0)->getName() + ": ";
	std::vector<std::string>::const_iterator it = params.begin();
	it += 2; // skip id <number>
	for (; it != params.end(); it++)  {
		msg += *it;
		msg += ' ';
	}

	// Send the message
	recipient->getNetEngine()->SendText(msg, TXT_PRIVATE);
	if (recipient != sender)
		sender->getNetEngine()->SendText(msg, TXT_PRIVATE); // Send the message also back to the client

	if( DedicatedControl::Get() )
		DedicatedControl::Get()->PrivateMessage_Signal(sender->getWorm(0), recipient->getWorm(0), msg.substr(sender->getWorm(0)->getName().length()+2));

	return "";
}

std::string ProcessTeamChat(const std::vector<std::string>& params, int sender_id)
{
	// Param check
	if (params.size() < GetCommand(&ProcessTeamChat)->iMinParamCount)
		return "Not enough parameters";

	// Get the sender
	CServerConnection *sender = cServer->getClient(sender_id);
	if (!sender)
		return "Message could not be sent";

	if(sender->getNumWorms() == 0)
		return "Message sent from client with no worms";
	
	if(sender->getMuted())
		return "Teamchats can not be sent when you are muted";
	
	// Get the message
	std::string msg = sender->getWorm(0)->getName() + ": ";
	std::vector<std::string>::const_iterator it = params.begin();
	for (; it != params.end(); it++)  {
		msg += *it;
		msg += ' ';
	}

	// Send the message to teammates
	CServerConnection *client = cServer->getClients();
	for (int i=0; i < MAX_CLIENTS; ++i, client++)  {
		for (int j=0; j < client->getNumWorms(); ++j)  {
			CWorm *w = client->getWorm(j);
			if (w && w->isUsed() && w->getTeam() == sender->getWorm(0)->getTeam())  {
				client->getNetEngine()->SendText(msg, TXT_TEAMPM);
				break;
			}
		}
	}

	return "";
}

std::string ProcessMe(const std::vector<std::string>& params, int sender_id)
{
	// Param check
	if (params.size() < GetCommand(&ProcessMe)->iMinParamCount)
		return "Not enough parameters";

	// Get the sender
	CServerConnection *sender = cServer->getClient(sender_id);
	if (!sender)
		return "Message could not be sent";

	// Get the message
	std::string msg = "* " + sender->getWorm(0)->getName() + " "; // Add star so clients with empty name won't fake others
	std::vector<std::string>::const_iterator it = params.begin();
	for (; it != params.end(); it++)  {
		msg += *it;
		msg += ' ';
	}

	// Send the message to everyone
	cServer->SendGlobalText(msg, TXT_CHAT);

	return "";
}

std::string ProcessSetMyName(const std::vector<std::string>& params, int sender_id)
{
	// Check params
	if (params.size() < GetCommand(&ProcessSetMyName)->iMinParamCount)
		return "Not enough parameters";

	// Get the sender
	CServerConnection *sender = cServer->getClient(sender_id);
	if (!sender)
		return "Name could not be changed";

	// Check if we can change the name
	if (!sender->getRights()->NameChange && !tLXOptions->tGameInfo.bAllowNickChange && !sender->getRights()->Override)
		return "You don't have sufficient privileges to change your nick";

	// Get the name
	std::string name;
	for (std::vector<std::string>::const_iterator it = params.begin(); it != params.end(); it++)  {
		name += *it;
		name += ' ';
	}
	name.erase(name.size() - 1);  // erase the last space

	name = RemoveSpecialChars(name); // Strip unicode characters
	if (name.size() > 32)  // Check if not too long
		name.erase(32, std::string::npos);

	// Check no other user has this name
	CWorm *w = cServer->getWorms();
	for(int i=0; i < MAX_WORMS; i++, w++) {
		if(!w->isUsed())
			continue;
		if(!stringcasecmp(name, w->getName()) && w->getID() != sender_id)
			return "Another player is already using this nick";
	}

	// Set the name
	std::string oldname = sender->getWorm(0)->getName();
	sender->getWorm(0)->setName(name);

	// Send the update
	cServer->UpdateWorms();

	// Send the notification
	cServer->SendGlobalText(oldname + " is now known as " + name, TXT_NORMAL);

	return "";
}

std::string ProcessSetName(const std::vector<std::string>& params, int sender_id)
{
	// Check params
	int p_id;
	std::string ch = CheckIDParams(params, &ProcessSetName, &p_id);
	if (ch.size() != 0)
		return ch;

	// Get the sender
	CServerConnection *sender = cServer->getClient(sender_id);
	if (!sender)
		return "Name could not be changed";

	// Check if we can change the name
	if (!sender->getRights()->NameChange && !sender->getRights()->Override)
		return "You don't have sufficient privileges to change user nick";

	// Get the name
	std::string name;
	for (std::vector<std::string>::const_iterator it = params.begin() + 2; it != params.end(); it++)  {
		name += *it;
		name += ' ';
	}
	name.erase(name.size() - 1);  // erase the last space

	name = RemoveSpecialChars(name); // Strip unicode characters
	if (name.size() > 32)  // Check if not too long
		name.erase(32, std::string::npos);

	// Get the target worm
	CWorm *tw = cServer->getWorms() + p_id;
	if (!tw->isUsed())
		return "The worm with specified ID does not exist";

	// Check no other user has this name
	CWorm *w = cServer->getWorms();
	for(int i=0; i < MAX_WORMS; i++, w++) {
		if(!w->isUsed())
			continue;
		if(!stringcasecmp(name, w->getName()) && w->getID() != p_id)
			return "Another player is already using this nick";
	}

	// Set the name
	std::string oldname = (cServer->getWorms() + p_id)->getName();
	tw->setName(name);

	// Send the update
	cServer->UpdateWorms();

	// Send the notification
	cServer->SendGlobalText(oldname + " is now known as " + name, TXT_NORMAL);

	return "";
}

std::string ProcessSetMySkin(const std::vector<std::string>& params, int sender_id)
{
	// Param check
	if (params.size() < GetCommand(&ProcessSetMySkin)->iMinParamCount)
		return "Not enough parameters";

	// Sender
	CServerConnection *sender = cServer->getClient(sender_id);
	if (!sender)
		return "Cannot change the skin";

	// Changing others skin can only authorized users
	if (!sender->getRights()->NameChange && !tLXOptions->tGameInfo.bAllowNickChange && !sender->getRights()->Override)
		return "You do not have sufficient rights to change your skin";

	// Get the worm
	CWorm *worm = sender->getWorm(0);
	if (!worm->isUsed())
		return "Cannot change skin of a non-existing worm";

	// Set the skin
	worm->setSkin(params[0]);
	cServer->UpdateWorms();

	return "";
}

std::string ProcessSetSkin(const std::vector<std::string>& params, int sender_id)
{
	// Param check
	int p_id;
	std::string ch = CheckIDParams(params, &ProcessSetSkin, &p_id);
	if (ch.size() != 0)
		return ch;

	// Sender
	CServerConnection *sender = cServer->getClient(sender_id);
	if (!sender)
		return "Cannot change the skin";

	// Changing others skin can only authorized users
	if (!sender->getRights()->NameChange && !sender->getRights()->Override)
		return "You do not have sufficient rights to change user skin";

	// Get the worm
	CWorm *worm = cServer->getWorms() + p_id; // HINT: this is safe because CheckIDParams makes a range check
	if (!worm->isUsed())
		return "Cannot change skin of a non-existing worm";

	// Set the skin
	worm->setSkin(params[2]);
	cServer->UpdateWorms();

	return "";
}

std::string ProcessSetMyColour(const std::vector<std::string>& params, int sender_id)
{
	// Param check
	if (params.size() < GetCommand(&ProcessSetMyColour)->iMinParamCount ||
		params.size() > GetCommand(&ProcessSetMyColour)->iMaxParamCount)
		return "Invalid parameter count, use /setcolor R G B";

	// Sender
	CServerConnection *sender = cServer->getClient(sender_id);
	if (!sender)
		return "Cannot change the skin";

	// Changing others colour can only authorized users
	if (!sender->getRights()->NameChange && !tLXOptions->tGameInfo.bAllowNickChange && !sender->getRights()->Override)
		return "You do not have sufficient rights to change your skin";

	// The profile graphics are only loaded once
	Uint8 r, g, b;
	r = (Uint8) atoi(params[0]);
	g = (Uint8) atoi(params[1]);
	b = (Uint8) atoi(params[2]);

	CWorm *worm = sender->getWorm(0);
	if (!worm->isUsed())
		return "Cannot change colour of a non-existing worm";

	// Set the colour
	worm->getSkin().setDefaultColor(MakeColour(r, g, b));
	worm->setColour(r, g, b);
	cServer->UpdateWorms();

	return "";
}

std::string ProcessSetColour(const std::vector<std::string>& params, int sender_id)
{
	// Param check
	int p_id;
	std::string ch = CheckIDParams(params, &ProcessSetColour, &p_id);
	if (ch.size() != 0)
		return ch;

	// Sender
	CServerConnection *sender = cServer->getClient(sender_id);
	if (!sender)
		return "Cannot change the skin";

	// Changing others colour can only authorized users
	if (!sender->getRights()->NameChange && !sender->getRights()->Override)
		return "You do not have sufficient rights to change user skin";

	// The profile graphics are only loaded once
	Uint8 r, g, b;
	r = (Uint8) atoi(params[2]);
	g = (Uint8) atoi(params[3]);
	b = (Uint8) atoi(params[4]);

	CWorm *worm = cServer->getWorms() + p_id;  // HINT: this is safe because CheckIDParams makes a range check
	if (!worm->isUsed())
		return "Cannot change colour of a non-existing worm";

	// Set the colour
	worm->setColour(r, g, b);
	cServer->UpdateWorms();

	return "";
}

std::string ProcessSuicide(const std::vector<std::string>& params, int sender_id)
{
	// Make sure we are playing
	if (cServer->getState() != SVS_PLAYING)
		return "Cannot suicide when not playing";

	// Param check
	if (params.size() < GetCommand(&ProcessSuicide)->iMinParamCount ||
		params.size() > GetCommand(&ProcessSuicide)->iMaxParamCount)
		return "Invalid parameter count";

	// Check the sender
	if (sender_id < 0 || sender_id >= MAX_WORMS)
		return "Invalid worm for suicide";

	// Get the "victim"
	CWorm *w = cServer->getWorms() + sender_id;
	if (!w->isUsed())
		return "The worm does not exist";

	// Get the number of suicides
	int lives = 1;
	if (params.size() >= 1)
		lives = atoi(params[0]);

	lives = CLAMP(lives, 0, w->getLives() + 1);

	cServer->killWorm(w->getID(), w->getID(), lives);
	
	return "";
}

std::string ProcessSpectate(const std::vector<std::string>& params, int sender_id)
{
	// Make sure we are playing
	if (cServer->getState() != SVS_PLAYING)
		return "Cannot spectate when not playing";

	// Param check
	if (params.size() < GetCommand(&ProcessSuicide)->iMinParamCount ||
		params.size() > GetCommand(&ProcessSuicide)->iMaxParamCount)
		return "Invalid parameter count";

	// Check the sender
	if (sender_id < 0 || sender_id >= MAX_WORMS)
		return "Invalid worm";

	// Get the "victim"
	CWorm *w = cServer->getWorms() + sender_id;
	if (!w->isUsed())
		return "The worm does not exist";

	// Can't spectate if we're out, apparently it causes the player to be able to play while out
	if(w->getLives() == WRM_OUT)
		return "You can't spectate while out";

	w->setLives(0);
	cClient->getNetEngine()->SendDeath(sender_id, sender_id);
	return "";
};

std::string ProcessLogin(const std::vector<std::string>& params, int sender_id)
{
	// Param check
	if (params.size() < GetCommand(&ProcessLogin)->iMinParamCount ||
		params.size() > GetCommand(&ProcessLogin)->iMaxParamCount)
		return "Invalid parameter count";

	// Check the sender
	if (sender_id < 0 || sender_id >= MAX_WORMS)
		return "Invalid worm";

	// Safety check for blank password (disallow these from security reasons)
	if (!tLXOptions->sServerPassword.size())
		return "The server has no password set, cannot log you in";

	// Check the password
	if (params[0] != tLXOptions->sServerPassword)
		return "Invalid password";

	// All OK, authorize the worm
	cServer->authorizeWorm(sender_id);
	if( DedicatedControl::Get() )	// Re-send it to ded script
		DedicatedControl::Get()->PrivateMessage_Signal( & cServer->getWorms()[sender_id], & cServer->getWorms()[sender_id], params[0] );

	return "";
}

std::string ProcessStart(const std::vector<std::string>& params, int sender_id)
{
	// Check the sender
	if (sender_id < 0 || sender_id >= MAX_WORMS)
		return "Invalid worm";

	// Check that not playing already
	if (cServer->getState() != SVS_LOBBY)
		return "The game is already running";

	// Check privileges
	CWorm *w = cServer->getWorms() + sender_id;
	CServerConnection *cl = w->getClient();
	if (!cl || !cl->getRights()->StartGame)
		return "You do not have sufficient privileges to start the game";

	// Check the number of players
	if (cServer->getNumPlayers() <= 1 && !tLXOptions->tGameInfo.features[FT_AllowEmptyGames])  {
		warnings << "Cannot start the game, too few players" << endl;
		return "Too few players to start the game";
	}

	// Start
	if (!bDedicated)
		DeprecatedGUI::Menu_Net_HostStartGame();
	else  {
		// Start the game
		cClient->setSpectate(false); // don't spectate; if we have added some players like bots, use them
		cServer->StartGame();	// start in dedicated mode

		// Leave the frontend
		*DeprecatedGUI::bGame = true;
		DeprecatedGUI::tMenu->bMenuRunning = false;
		tLX->iGameType = GME_HOST;
	}

	return "";
}

std::string ProcessLobby(const std::vector<std::string>& params, int sender_id)
{
	// Check the sender
	if (sender_id < 0 || sender_id >= MAX_WORMS)
		return "Invalid worm";

	// Check that not playing already
	if (cServer->getState() == SVS_LOBBY)
		return "Already in lobby";

	// Check privileges
	CWorm *w = cServer->getWorms() + sender_id;
	CServerConnection *cl = w->getClient();
	if (!cl || !cl->getRights()->StartGame)
		return "You do not have sufficient privileges to start the game";

	// Go to lobby
	cServer->gotoLobby();

	return "";
}

std::string ProcessMod(const std::vector<std::string>& params, int sender_id)
{
	// Check the sender
	if (sender_id < 0 || sender_id >= MAX_WORMS)
		return "Invalid worm";

	// Param check
	if (params.size() < GetCommand(&ProcessMod)->iMinParamCount)
		return "Too few parameters";

	// Check that we're in lobby
	if (cServer->getState() != SVS_LOBBY)
		return "Cannot change the mod while playing";

	// Check privileges
	CWorm *w = cServer->getWorms() + sender_id;
	CServerConnection *cl = w->getClient();
	if (!cl || !cl->getRights()->StartGame)
		return "You do not have sufficient privileges to change the mod";

	// Join the parameters into one string
	std::string mod = *params.begin();
	for (std::vector<std::string>::const_iterator it = params.begin() + 1; it != params.end(); it++)
		mod += " " + *it;

	// Check if the mod is available
	std::string name;
	if (!CGameScript::CheckFile(mod, name))
		return "Mod \"" + mod + "\" not available";

	// Set the mod
	tLXOptions->tGameInfo.sModDir = mod;
	tLXOptions->tGameInfo.sModName = name;
	if (!bDedicated)
		DeprecatedGUI::Menu_Net_HostLobbySetMod(mod);
	cServer->UpdateGameLobby();

	// Notify everybody
	cServer->SendGlobalText(w->getName() + " changed mod to " + name, TXT_NOTICE);

	return "";
}

std::string ProcessLevel(const std::vector<std::string>& params, int sender_id)
{
	// Check the sender
	if (sender_id < 0 || sender_id >= MAX_WORMS)
		return "Invalid worm";

	// Param check
	if (params.size() < GetCommand(&ProcessLevel)->iMinParamCount)
		return "Too few parameters";

	// Check that we're in lobby
	if (cServer->getState() != SVS_LOBBY)
		return "Cannot change the level while playing";

	// Check privileges
	CWorm *w = cServer->getWorms() + sender_id;
	CServerConnection *cl = w->getClient();
	if (!cl || !cl->getRights()->StartGame)
		return "You do not have sufficient privileges to change the level";

	// Join the parameters into one string
	std::string level = *params.begin();
	for (std::vector<std::string>::const_iterator it = params.begin() + 1; it != params.end(); it++)
		level += " " + *it;

	// Add the extension if necessary
	if (stringcaserfind(level, ".lxl") == std::string::npos)
		level += ".lxl";

	// Check if the level is available
	if (!IsFileAvailable("levels/"  + level))
		return "Level \"" + level + "\" not available";
	std::string name = DeprecatedGUI::Menu_GetLevelName(level);
	if (!name.size())
		return "The level file is corrupted";

	// Set the level
	tLXOptions->tGameInfo.sMapFile = level;
	tLXOptions->tGameInfo.sMapName = name;
	if (!bDedicated)
		DeprecatedGUI::Menu_Net_HostLobbySetLevel(level);
	cServer->UpdateGameLobby();

	// Notify everybody
	cServer->SendGlobalText(w->getName() + " changed level to " + name, TXT_NOTICE);

	return "";
}

std::string ProcessLt(const std::vector<std::string>& params, int sender_id)
{
	// Check the sender
	if (sender_id < 0 || sender_id >= MAX_WORMS)
		return "Invalid worm";

	// Param check
	if (params.size() < GetCommand(&ProcessLt)->iMinParamCount ||
		params.size() > GetCommand(&ProcessLt)->iMaxParamCount)
		return "Invalid parameter count";

	// Check privileges
	CWorm *w = cServer->getWorms() + sender_id;
	CServerConnection *cl = w->getClient();
	if (!cl || !cl->getRights()->StartGame)
		return "You do not have sufficient privileges to change the loading time";

	// Convert to number
	bool fail = false;
	int lt = from_string<int>(params[0], fail);
	if (fail || lt < 0 || lt > 500)
		return "Please specify a loading time in the range from 0 to 500";
	
	// Set the loading time
	tLXOptions->tGameInfo.iLoadingTime = lt;
	if (cServer->getState() == SVS_LOBBY)
		cServer->UpdateGameLobby();
	else  {
		// Update the loading time for worms
		CWorm *w = cServer->getWorms();
		for (int i = 0; i < MAX_WORMS; i++)
			if (w->isUsed())
				w->setLoadingTime((float)lt/100.0f);

		// TODO: updating the LT in game won't update the value on clients
		// which means that in the Game Settings dialog they will see an incorrect value
		// (but it works otherwise because everything is simulated on the server)
		// Should we disallow changing the LT in game because of this cosmetic bug
		// or just allow hosts to change it whenever they want?
	}

	// Notify everybody
	cServer->SendGlobalText(w->getName() + " changed loading time to " + itoa(lt) + " %", TXT_NOTICE);

	return "";
}

std::string ProcessDedicated(const std::vector<std::string>& params, int sender_id) {
	// Check the sender
	if (sender_id < 0 || sender_id >= MAX_WORMS)
		return "Invalid worm";

	// Param check
	if (params.size() < GetCommand(&ProcessDedicated)->iMinParamCount ||
		params.size() > GetCommand(&ProcessDedicated)->iMaxParamCount)
		return "Invalid parameter count";

	// Check privileges
	CWorm *w = &cServer->getWorms()[sender_id];
	CServerConnection *cl = w->getClient();
	if (!cl || !cl->getRights()->Dedicated)
		return "You do not have sufficient privileges to change the level";
	
	if(DedicatedControl::Get()) {
		struct ChatDedHandler : DedInterface {
			int sender_id;
			ChatDedHandler(int i) : sender_id(i) {}
			
			void msg(const std::string& str) {
				CWorm *w = &cServer->getWorms()[sender_id];
				if(!w->isUsed()) return;
				if(!w->getClient()) return;
				w->getClient()->getNetEngine()->SendText(str, TXT_NOTICE);
			}
			
			virtual void pushReturnArg(const std::string& str) {
				msg("Dedicated: " + str);
			}
			virtual void finalizeReturn() {
				msg("Dedicated.");
			}
			virtual void finishedCommand(const std::string& cmd) {
				delete this;
			}			
		};
		
		std::string cmd = "";
		for(std::vector<std::string>::const_iterator i = params.begin(); i != params.end(); ++i) {
			if(i != params.begin()) cmd += " ";
			cmd += *i;
		}
		DedicatedControl::Get()->Execute( DedInterface::Command(new ChatDedHandler(sender_id), cmd) );
		
		return "";
	}
	
	return "dedicated control is not available";
}

