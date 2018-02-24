/*
 *  FeatureList.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 22.12.08.
 *  code under LGPL
 *
 */


#include "FeatureList.h"
#include "Version.h"
#include "CServer.h"



// WARNING: Keep this always synchronised with FeatureIndex!
// Legend:	Name in options,		Human-readable-name,			Long description,	
//			Unset,	Default,		Min client Version,	Group,						[Min,]	[Max,]	[server-side only] [optional for client] [switch to unset value on older clients] [is value unsigned] (Min and Max are only for Int and Float)
// Old clients are kicked if feature version is greater that client version, no matter if feature is server-sided or safe to ignore

Feature featureArray[] = {
	Feature("GameSpeed", 			"Game-speed multiplicator", 	"Game simulation speed is multiplicated by the given value.", 
			1.0f, 	1.0f,			OLXBetaVersion(7), 	GIG_Advanced, ALT_Advanced,		0.1f, 	10.0f ),
	Feature("GameSpeedOnlyForProjs", "Speed multiplier only for projs",	"Game-speed multiplicator applies only for projectiles and weapons, everything else will be normal speed",
			false, false,			OLXBetaVersion(0,58,1),	GIG_Advanced,	ALT_Advanced,				false),
	Feature("ScreenShaking",		"Screen shaking", 		"Screen shaking when something explodes", 
			true, 	false, 			OLXBetaVersion(0,58,1),	GIG_Other, 	ALT_VeryAdvanced,				false,	true,	true ),
	Feature("FullAimAngle",			"Full aim angle", 		"Enables full aim angle, i.e. also allows to aim straight down", 
			false, 	false, 			OLXRcVersion(0,58,3),	GIG_Other, 	ALT_VeryAdvanced,				false,	true,	false ),
	Feature("MiniMap",				"Mini map", 		"Show mini map", 
			true, 	true, 			OLXBetaVersion(0,58,1),	GIG_Other, 	ALT_Advanced,					false,	false ),
	Feature("SuicideDecreasesScore", "Suicide decreases score", "The score descreases after a suicide.", 
			false, 	false, 			Version(), 			GIG_Score, 	ALT_Advanced,					false,	true ),
	Feature("TeamkillDecreasesScore", "Teamkill decreases score", "The score descreases after a teamkill.", 
			false, 	false, 			Version(), 			GIG_Score, 	ALT_Advanced,					false,	true ),
	Feature("DeathDecreasesScore", "Death decreases score", "The score decreases after each death by this factor",
			0.0f, 	0.0f, 			Version(), 			GIG_Score, 	ALT_Advanced,	0.0f,	5.0f,	false,	true ),
	Feature("CountTeamkills", 		"Count teamkills", 				"When killing player from your team increase your score", 
			false, 	false, 			Version(), 			GIG_Score, 	ALT_VeryAdvanced,				false,	true ),
	Feature("AllowNegativeScore",	"Allow negative score", 		"Allow negative score, when this option is not selected death/suicide/teamkill are not counted if score is zero", 
			false, 	false, 			Version(), 			GIG_Score, 	ALT_VeryAdvanced,				false,	true ),
	Feature("TeamInjure", 			"Damage team members", 			"If disabled, your bullets and projectiles don't damage other team members. It's like friendlyfire in other games.", 
			true, 	true, 			OLXBetaVersion(0,58,1), 	GIG_Weapons, ALT_Advanced ),
	Feature("TeamHit", 				"Hit team members", 			"If disabled, your bullets and projectiles will fly through your team members.", 
			true, 	true, 			OLXBetaVersion(0,58,1), 	GIG_Weapons, ALT_Advanced ),
	Feature("SelfInjure", 			"Damage yourself", 				"If disabled, your bullets and projectiles don't damage you.", 
			true, 	true, 			OLXBetaVersion(0,58,1), 	GIG_Weapons, ALT_Advanced ),
	Feature("SelfHit", 				"Hit yourself", 				"If disabled, your bullets and projectiles will fly through yourself.", 
			true, 	true, 			OLXBetaVersion(0,58,1), 	GIG_Weapons, ALT_Advanced ),
	Feature("AllowEmptyGames", 		"Allow empty games", 			"If enabled, games with one or zero worms will not quit. This is only possible if you have infinite lives set and also only for network games.", 
			false, 	false, 			Version(), 			GIG_Other, 	ALT_Advanced,					true,	true ),
	Feature("HS_HideTime", 			"Hiding time", 					"AbsTime at the start of the game for hiders to hide", 
			20.0f, 	20.0f, 			Version(), 			GIG_HideAndSeek, ALT_Basic,	0.0f,	100.0f,	true,	true ),
	Feature("HS_AlertTime", 		"Alert time", 					"When player discovered but escapes the time for which it's still visible", 
			10.0f, 	10.0f, 			Version(), 			GIG_HideAndSeek, ALT_Basic,	0.1f,	100.0f,	true,	true ),
	Feature("HS_HiderVision",	 	"Hider vision", 				"How far hider can see, in pixels (whole screen = 320 px)", 
			175, 	175, 			Version(), 			GIG_HideAndSeek, ALT_Advanced,	0,	320, 	true,	true ),
	Feature("HS_HiderVisionThroughWalls", "Hider vision thorough walls", "How far hider can see through walls, in pixels (whole screen = 320 px)", 
			75, 	75, 			Version(), 			GIG_HideAndSeek, ALT_Advanced,	0,	320, 	true,	true ),
	Feature("HS_SeekerVision",		"Seeker vision", 				"How far seeker can see, in pixels (whole screen = 320 px)", 
			125, 	125, 			Version(), 			GIG_HideAndSeek, ALT_Advanced,	0,	320, 	true,	true ),
	Feature("HS_SeekerVisionThroughWalls", "Seeker vision thorough walls", "How far seeker can see through walls, in pixels (whole screen = 320 px)", 
			0, 		0, 				Version(), 			GIG_HideAndSeek, ALT_Advanced,	0,	320, 	true,	true ),
	Feature("HS_SeekerVisionAngle",	"Seeker vision angle",			"The angle of seeker vision (180 = half-circle, 360 = full circle)", 
			360, 	360, 			Version(),			GIG_HideAndSeek, ALT_Advanced,	0,	360,	false,	true ),
	Feature("NewNetEngine", 		"New net engine (non-functional)",	"This feature does not work, do not use it",
			false, 	false, 			OLXBetaVersion(0,58,1),	GIG_Advanced, ALT_DevKnownUnstable ),
	Feature("FillWithBotsTo",		"Fill with bots up to",	"If too less players, it will get filled with bots",
			0,	0,					OLXBetaVersion(0,58,1),		GIG_Other, ALT_Advanced,	0,	MAX_PLAYERS, true,	true),
	Feature("WormSpeedFactor",		"Worm speed factor",	"Initial factor to worm speed",
			1.0f,	1.0f,			OLXBetaVersion(0,58,1),		GIG_Other, ALT_Advanced,	-2.0f,	10.0f,	true),
	Feature("WormDamageFactor",		"Worm damage factor",	"Initial factor to worm damage",
			1.0f,	1.0f,			OLXBetaVersion(0,58,1),		GIG_Other,	ALT_VeryAdvanced,	-2.0f,	10.0f,	true),
	Feature("WormShieldFactor",		"Worm shield factor",	"Initial factor to worm shield",
			1.0f,	1.0f,			OLXBetaVersion(0,58,1),		GIG_Other,	ALT_VeryAdvanced,	-2.0f,	10.0f,	true),
	Feature("InstantAirJump",		"Instant air jump",		"Worms can jump in air instantly, this allows floating in air",
			false,	false,			OLXBetaVersion(0,58,1),		GIG_Other, ALT_Advanced,	true),	// Server-side
	Feature("RelativeAirJump",		"Relative air jump",	"Worms can jump in air in a given time interval",
			false,	false,			OLXBetaVersion(0,58,1),		GIG_Other, ALT_Advanced),				// Client-side
	Feature("RelativeAirJumpDelay",	"Delay for relative air jumps",	"How fast can you do air-jumps",
			0.7f,	0.7f,			Version(),				GIG_Other,	ALT_VeryAdvanced,	0.0f, 	5.0f),
	Feature("AllowWeaponsChange",	"Allow weapons change",	"Everybody can change its weapons at any time",
			true,	true,			OLXBetaVersion(0,58,1),		GIG_Weapons, ALT_Advanced,	true, true),
	Feature("ImmediateStart",		"Immediate start",		"Immediate start of game, don't wait for other players weapon selection",
			true,	true,			OLXBetaVersion(8),		GIG_Advanced, ALT_Advanced,	true),
	Feature("DisableWpnsWhenEmpty",	"Disable weapons when empty", "When a weapon got uncharged, it got disabled and you have to catch a bonus (be sure that you have bonuses activated). This is usefull in games like Race.",
			false,	false,			OLXBetaVersion(7) /* it needs wpninfo packet which is there since beta7 */,		GIG_Weapons, ALT_VeryAdvanced,	true),
	Feature("WeaponCombos",			"Weapon combos",	"Enable/disable weapon combos, i.e. fast changing of weapons and shooting at the same time",
			true,	true,			OLXBetaVersion(0,58,10),	GIG_Weapons, ALT_Advanced,	false, false),
	Feature("InfiniteMap",			"Infinite map",			"Map has no borders and is tiled together",
			false,	false,			OLXBetaVersion(0,58,1),		GIG_Other,	ALT_Advanced,	false),
	Feature("AddMirroredMap",		"Add mirrored map",		"Extend the map by adding a mirrored version",
			false,	false,			OLXRcVersion(0,58,4),		GIG_Other,	ALT_Advanced,	false),
	Feature("MirroredMapSide",		"Mirrored map side",	"0 = left side, 1 = right side",
			0,	0,					Version(),					GIG_Other,	ALT_Advanced,	0,	1, false),
	Feature("AddTopMirroredMap",	"Add vertically mirrored map",	"Extend the map by adding a mirrored version to the top",
			false,	false,			OLXRcVersion(0,58,4),		GIG_Other,	ALT_VeryAdvanced,	false),
	Feature("WormFriction",			"Worm Friction",		"Friction coefficient for worms (0 = disabled)",
			0.0f, 0.0f,				OLXBetaVersion(0,58,1),		GIG_Other,	ALT_VeryAdvanced,	0.0f, 2.0f,	false),
	Feature("WormGroundFriction",	"Worm Ground Friction",		"Friction coefficient when worms are on ground (0.1 = default; 1 = stucked; 0 = no friction)",
			0.1f, 0.1f,				OLXBetaVersion(0,58,9),		GIG_Other,	ALT_VeryAdvanced,	0.0f, 1.0f,	false),
	Feature("ProjFriction",			"Projectile Friction",	"Friction coefficient for projectiles (0 = disabled)",
			0.0f, 0.0f,				OLXBetaVersion(0,58,1),		GIG_Other,	ALT_VeryAdvanced,	0.0f, 2.0f,	false),
	Feature("TeamScoreLimit",		"Team Score limit",		"Team score limit",
			-1, -1,					OLXBetaVersion(0,58,1),		GIG_General, ALT_Basic,	-1, 100,	true, true, false, true),
	Feature("SizeFactor",			"Size factor",			"The size of everything in game will be changed by this factor (i.e. made bigger or smaller)",
			1.0f, 1.0f,				OLXBetaVersion(0,58,1),		GIG_Advanced, ALT_Advanced,	0.5f, 4.0f, false),
	Feature("CTF_AllowRopeForCarrier", "Allow rope for carrier", "The worm who is holding the flag can use ninja rope",
			true, true,				OLXBetaVersion(0,58,1),		GIG_CaptureTheFlag, ALT_Basic, true),
	Feature("CTF_SpeedFactorForCarrier", "Speed factor for carrier", "Changes the carrier speed by this factor. Perhaps you want to make the carrier slower so you can more easily get the flag again.",
			1.0f, 1.0f,				OLXBetaVersion(0,58,1),		GIG_CaptureTheFlag, ALT_Basic, 0.1f, 3.0f, true),
	Feature("Race_Rounds", "Rounds", "Amount of rounds",
			5,5,					Version(),				GIG_Race,	ALT_Basic,	-1,		100,	true,	true),
	Feature("Race_AllowWeapons", "Allow weapons", "If disabled, you cannot shoot",
			false,	false,			Version(),				GIG_Race,	ALT_Advanced,	false),
	Feature("Race_CheckPointRadius", "Checkpoint radius", "The radius of the checkpoints (bigger value makes race easier)",
			15.0f, 15.0f,			Version(),				GIG_Race,	ALT_VeryAdvanced, 5.0f, 100.f, true, true),
	Feature("IndestructibleBonuses", "Indestructible bonuses", "Bonuses will not be destroyed by explosions",
			false, false,			Version(),				GIG_Bonus,	ALT_VeryAdvanced, false, true),

	Feature::Unset()
};

static_assert(__FTI_BOTTOM == sizeof(featureArray)/sizeof(Feature) - 1, "featureArray__sizecheck");



Feature* featureByName(const std::string& name) {
	foreach( Feature*, f, Array(featureArray,featureArrayLen()) ) {
		if( stringcaseequal(f->get()->name, name) )
			return f->get();
	}
	return NULL;
}

FeatureSettings::FeatureSettings() {
	settings = new ScriptVar_t[featureArrayLen()];
	foreach( Feature*, f, Array(featureArray,featureArrayLen()) ) {
		(*this)[f->get()] = f->get()->defaultValue;
	}
}

FeatureSettings::~FeatureSettings() {
	if(settings) delete[] settings;
}

FeatureSettings& FeatureSettings::operator=(const FeatureSettings& r) {
	if(settings) delete[] settings;

	settings = new ScriptVar_t[featureArrayLen()];
	foreach( Feature*, f, Array(featureArray,featureArrayLen()) ) {
		(*this)[f->get()] = r[f->get()];		
	}
	
	return *this;
}

ScriptVar_t FeatureSettings::hostGet(FeatureIndex i) {
	ScriptVar_t var = (*this)[i];
	Feature* f = &featureArray[i];
	if(f->getValueFct)
		var = (cServer->*(f->getValueFct))( var );
	else if(f->unsetIfOlderClients) {
		if(cServer->clientsConnected_less(f->minVersion))
			var = f->unsetValue;
	}
			
	return var;
}

bool FeatureSettings::olderClientsSupportSetting(Feature* f) {
	if( f->optionalForClient ) return true;
	return hostGet(f) == f->unsetValue;
}

