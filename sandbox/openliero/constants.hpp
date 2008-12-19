#ifndef LIERO_CONSTANTS_HPP
#define LIERO_CONSTANTS_HPP

#include <string>

enum
{
	NRInitialLength,
	NRAttachLength,

	MinBounceUp,
	MinBounceDown,
	MinBounceLeft,
	MinBounceRight,
	WormGravity,
	WalkVelLeft,
	MaxVelLeft,
	WalkVelRight,
	MaxVelRight,
	JumpForce,
	MaxAimVelLeft,
	AimAccLeft,
	MaxAimVelRight,
	AimAccRight,
	NinjaropeGravity,
	NRMinLength,
	NRMaxLength,
	BonusGravity,
	
	WormFricMult,
	WormFricDiv,
	WormMinSpawnDistLast,
	WormMinSpawnDistEnemy,
	WormSpawnRectX,
	WormSpawnRectY,
	WormSpawnRectW,
	WormSpawnRectH,
	AimFricMult,
	AimFricDiv,
	NRThrowVelX,
	NRThrowVelY,
	NRForceShlX,
	NRForceDivX,
	NRForceShlY,
	NRForceDivY,
	NRForceLenShl,
	BonusBounceMul,
	BonusBounceDiv,
	BonusFlickerTime,
	
	
	AimMaxRight,
	AimMinRight,
	AimMaxLeft,
	AimMinLeft,
	
	NRPullVel,
	NRReleaseVel,
	
	NRColourBegin,
	NRColourEnd,
	BonusExplodeRisk,
	BonusHealthVar,
	BonusMinHealth,
	LaserWeapon,
	FirstBloodColour,
	NumBloodColours,

	BObjGravity,

	BonusDropChance,
	SplinterLarpaVelDiv,
	SplinterCracklerVelDiv,
	
	// FallDamage hack
	FallDamageRight,
	FallDamageLeft,
	FallDamageDown,
	FallDamageUp,
	
	// WormFloat hack
	WormFloatLevel,
	WormFloatPower,
	
	// BonusSpawn hack
	BonusSpawnRectX,
	BonusSpawnRectY,
	BonusSpawnRectW, // This is used even when the hack isn't enabled
	BonusSpawnRectH, // -==-
	
	MaxC
};

enum
{
	InitSound,
	LoadingSounds,
	LoadingAndThinking,
	OK,
	OK2,
	PressAnyKey,
	CommittedSuicideMsg,
	KilledMsg,
	YoureIt,
	
	Init_BaseIO,
	Init_IRQ,
	Init_DMA8,
	Init_DMA16,
	
	Init_DSPVersion,
	Init_Colon,
	Init_16bit,
	Init_Autoinit,
	
	Init_XMSSucc,
	
	Init_FreeXMS,
	Init_k,
	
	MaxS
};

enum
{
	HFallDamage,
	HBonusReloadOnly,
	HBonusSpawnRect,
	HBonusOnlyHealth,
	HBonusOnlyWeapon,
	HBonusDisable,
	HWormFloat,
	
	MaxH
};

extern int C[MaxC];
extern std::string S[MaxS];
extern bool H[MaxH];

void loadConstantsFromEXE();

#endif // LIERO_CONSTANTS_HPP
