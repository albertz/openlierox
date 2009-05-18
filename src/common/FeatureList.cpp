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
//			Unset,	Default,		Min client Version,	Group,				[Min,]	[Max,]	[server-side only] (Min and Max are only for Int and Float)

Feature featureArray[] = {
	Feature("GameSpeed", 			"Game-speed multiplicator", 	"Game simulation speed is multiplicated by the given value.", 
			1.0f, 	1.0f,			OLXBetaVersion(7), 	GIG_Advanced, 		0.1f, 	10.0f ),
	Feature("ForceScreenShaking", 	"Force screen shaking", 		"Screen shaking will be activated for everybody.", 
			true, 	true, 			Version(), 			GIG_Other, 							true ),
	Feature("SuicideDecreasesScore", "Suicide/teamkill decreases score", "The kills count will be descreased by one after a suicide or teamkill.", 
			false, 	false, 			Version(), 			GIG_Score, 							true ),
	Feature("TeamInjure", 			"Damage team members", 			"If disabled, your bullets and projectiles don't damage other team members.", 
			true, 	true, 			OLXBetaVersion(9), 	GIG_Weapons ),
	Feature("TeamHit", 				"Hit team members", 			"If disabled, your bullets and projectiles will fly through your team members.", 
			true, 	true, 			OLXBetaVersion(9), 	GIG_Weapons ),
	Feature("SelfInjure", 			"Damage yourself", 				"If disabled, your bullets and projectiles don't damage you.", 
			true, 	true, 			OLXBetaVersion(9), 	GIG_Weapons ),
	Feature("SelfHit", 				"Hit yourself", 				"If disabled, your bullets and projectiles will fly through yourself.", 
			true, 	true, 			OLXBetaVersion(9), 	GIG_Weapons ),
	Feature("AllowEmptyGames", 		"Allow empty games", 			"If enabled, games with one or zero worms will not quit.", 
			false, 	false, 			Version(), 			GIG_Other, 							true),
	Feature("CountTeamkills", 		"Count teamkills", 				"When killing player from your team increase your damage score counter", 
			false, 	false, 			Version(), 			GIG_Score, 							true),
	Feature("HS_HideTime", 			"Hiding time", 					"AbsTime at the start of the game for hiders to hide", 
			20.0f, 	20.0f, 			Version(), 			GIG_HideAndSeek,	0.0f,	100.0f,	true ),
	Feature("HS_AlertTime", 		"Alert time", 					"When player discovered but escapes the time for which it's still visible", 
			10.0f, 	10.0f, 			Version(), 			GIG_HideAndSeek, 	0.1f, 	100.0f,	true ),
	Feature("HS_HiderVision",	 	"Hider vision", 				"How far hider can see, in pixels (whole screen = 320 px)", 
			175, 	175, 			Version(), 			GIG_HideAndSeek, 	0, 		320, 	true ),
	Feature("HS_HiderVisionThroughWalls", "Hider vision thorough walls", "How far hider can see through walls, in pixels (whole screen = 320 px)", 
			75, 	75, 			Version(), 			GIG_HideAndSeek, 	0, 		320, 	true ),
	Feature("HS_SeekerVision",		"Seeker vision", 				"How far seeker can see, in pixels (whole screen = 320 px)", 
			125, 	125, 			Version(), 			GIG_HideAndSeek, 	0, 		320, 	true ),
	Feature("HS_SeekerVisionThroughWalls", "Seeker vision thorough walls", "How far seeker can see through walls, in pixels (whole screen = 320 px)", 
			0, 		0, 				Version(), 			GIG_HideAndSeek, 	0, 		320, 	true ),
	Feature("HS_SeekerVisionAngle",	"Seeker vision angle",			"The angle of seeker vision (180 = half-circle, 360 = full circle)", 
			360, 	360, 			Version(),			GIG_HideAndSeek, 	0, 		360,	true ),
	Feature("NewNetEngine", 		"New net engine (restricted)",	"New net engine without self-shooting and lag effects, CPU-eating, many features won't work with it; DONT USE IF YOU DONT KNOW IT", 
			false, 	false, 			OLXBetaVersion(9),	GIG_Advanced ),
	Feature("FillWithBotsTo",		"Fill with bots up to",	"If too less players, it will get filled with bots",
			0,	0,					OLXBetaVersion(9),		GIG_Other,			0,		MAX_PLAYERS, true),
	Feature("WormSpeedFactor",		"Worm speed factor",	"Initial factor to worm speed",
			1.0f,	1.0f,			OLXBetaVersion(9),		GIG_Other,		-2.0f,	10.0f, true),
	Feature("WormDamageFactor",		"Worm damage factor",	"Initial factor to worm damage",
			1.0f,	1.0f,			OLXBetaVersion(9),		GIG_Other,		-2.0f,	10.0f, true),
	Feature("InstantAirJump",		"Instant air jump",		"Worms can jump in air instantly, this allows floating in air",
			false,	false,			OLXBetaVersion(9),		GIG_Other,		true),	// Server-side
	Feature("RelativeAirJump",		"Relative air jump",	"Worms can jump in air, jumps have delay between them",
			false,	false,			OLXBetaVersion(9),		GIG_Other),				// Client-side
	Feature("RelativeAirJumpDelay",	"Delay between relative air jumps",	"How fast can you do air-jumps",
			0.7f,	0.7f,			Version(),				GIG_Other,		0.0f, 	5.0f),
	Feature("AllowWeaponsChange",	"Allow weapons change",	"Everybody can change its weapons at any time",
			true,	true,			OLXBetaVersion(9),		GIG_Weapons,	true),
	Feature("ImmediateStart",		"Immediate start",		"Immediate start of game, don't wait for other players weapon selection",
			false,	false,			Version(),				GIG_Advanced,	true),
	Feature("DisableWpnsWhenEmpty",	"Disable weapons when empty", "When a weapon got uncharged, it got disabled and you have to catch a bonus (be sure that you have bonuses activated)",
			false,	false,			OLXBetaVersion(7) /* it needs wpninfo packet which is there since beta7 */,		GIG_Weapons,	true),
	Feature("CTF_ScoreLimit",		"Score limit",			"Flag score limit",
			5, 5,					OLXBetaVersion(9),		GIG_CaptureTheFlag, -1, 100, true),
	Feature("CTF_AllowRopeForCarrier", "Allow rope for carrier", "The worm who is holding the flag can use ninja rope",
			true, true,				OLXBetaVersion(9),		GIG_CaptureTheFlag, true),
	Feature("CTF_SpeedFactorForCarrier", "Speed factor for carrier", "Changes the carrier speed by this factor",
			1.0f, 1.0f,				OLXBetaVersion(9),		GIG_CaptureTheFlag,	 0.1f, 3.0f, true),
	Feature("Race_Rounds", "Rounds", "Amount of rounds",
			5,5,					OLXBetaVersion(9),		GIG_Race,		-1, 100, true),
	Feature("Race_AllowWeapons", "Allow weapons", "If disabled, you cannot shoot",
			false,false,			OLXBetaVersion(9),		GIG_Race,		true),

	Feature::Unset()
};

static_assert(__FTI_BOTTOM == sizeof(featureArray)/sizeof(Feature) - 1, featureArray__sizecheck);


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
	if(f->serverSideOnly) return true;
	return hostGet(f) == f->unsetValue;
}

