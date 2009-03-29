/*
 *  ProjectileDesc.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 29.03.09.
 *  code under LGPL
 *
 */

#ifndef __PROJECTILEDESC_H__
#define __PROJECTILEDESC_H__

#include <string>
#include <vector>

struct SDL_Surface;
struct SoundSample;

// Projectile structure
struct proj_t {
	proj_t() : id(0), PrjTrl_Proj(NULL), bmpImage(NULL), smpSample(NULL) {}
	
	std::string	filename;		// Compiler use (was 64b before)
	int		id;					// File ref use
	
	int		Type;
	int		Trail;
	std::vector<Color>	Colour;
	std::string	ImgFilename; // (was 64b before)
	int		Rotating;
	int		RotIncrement;
	int		RotSpeed;
	int		UseAngle;
	int		UseSpecAngle;
	int		AngleImages;
	int		UseCustomGravity;
	int		Gravity;
	int		Animating;
	float	AnimRate;
	int		AnimType;
	float   Dampening;
	
	// Timer (When the timer is finished)
	int		Timer_Type;
	float	Timer_Time;
	float	Timer_TimeVar;
	int		Timer_Damage;
	int		Timer_Projectiles;
	int		Timer_Shake;
	
	// Hit (When hitting the terrain)
	int		Hit_Type;
	int		Hit_Damage;
	int		Hit_Projectiles;
	int		Hit_UseSound;
	int		Hit_Shake;
	std::string	Hit_SndFilename; // (was 64b before)
	float	Hit_BounceCoeff;
	int		Hit_BounceExplode;
	
    // OnExplode (When something near has exploded)
    int		Exp_Type;
	int		Exp_Damage;
	int		Exp_Projectiles;
	int		Exp_UseSound;
    std::string	Exp_SndFilename; // (was 64b before)
	int		Exp_Shake;
	
	
    // Touch (When another projectile has touched this projectile)
    int		Tch_Type;
	int		Tch_Damage;
	int		Tch_Projectiles;
	int		Tch_UseSound;
    std::string	Tch_SndFilename; // (was 64b before)
	int		Tch_Shake;
	
	
	// Player hit
	int		PlyHit_Type;
	int		PlyHit_Damage;
	int		PlyHit_Projectiles;
	float	PlyHit_BounceCoeff;
	
	int		ProjUseangle;
	int		ProjAngle;
	int		ProjSpeed;
	float	ProjSpeedVar;
	float	ProjSpread;
	int		ProjAmount;
	proj_t	*Projectile;
	
	
	// Projectile trail
	int		PrjTrl_UsePrjVelocity;
	float	PrjTrl_Delay;
	int		PrjTrl_Amount;
	int		PrjTrl_Speed;
	float	PrjTrl_SpeedVar;
	float	PrjTrl_Spread;
	proj_t	*PrjTrl_Proj;
	
	
	SDL_Surface * bmpImage;	// Read-only var, managed by game script, no need in smartpointer
	SoundSample * smpSample;
};


#endif
