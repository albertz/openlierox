/*
 *  OlxVariable.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 28.02.10.
 *  code under LGPL
 *
 */

#ifndef __GUS_OLXVARIABLE_H__
#define __GUS_OLXVARIABLE_H__

#include <list>
#include <string>
#include "gusanos/console/variables.h"
#include "game/Settings.h"
#include "StringUtils.h"
#include "FeatureList.h"
#include "CodeAttributes.h"

// TODO: move this? or is there sth like this in boost/stdlib?
template<typename T>
INLINE T Identity(T v) { return v; }

/*
 A wrapper to an OLX variable (a ref to <FeatureSettingsLayer,index>) which can be registered in the Gus console system.
 It also supports on-the-fly conversion if Gusanos/OLX use some different norm / way to represent things.
 
 It is supposed to replace most vars of Gusanos Options (gusgame.h) with the related OLX setting.
 Many vars need some conversation (like acceleration/speed stuff) which are mostly straight forward to calculate though.
 
 The point in replacing them is to have a common setting for Gus and OLX.
 The point in doing it via the OLX Settings system is to be able to define fallback defaults, to be able
 to use settings layers and to sync them on the network (Gusanos doesn't do that at all).
 
 The FeatureSettingsLayer we link to is modSettings. This is to give a nice way via the layer system
 to overwrite the setting.
 */
template<typename T, T (*getFct) (), void (*putFct) (T)>
struct __OlxVariable {
	
	// WARNING: as long as the returned Variable exists, the __OlxVariable must exist too
	Variable* gusVar(const std::string& name, T gusDefault = T()) {
		struct GusVarWrapper : /* Gusanos */ Variable {
			__OlxVariable* lxvar;
			T gusDefault;
			GusVarWrapper(__OlxVariable* v, const std::string& name, T _default)
			: Variable(name), lxvar(v), gusDefault(_default) {}

			// Gusanos console system callback
			virtual std::string invoke(const std::list<std::string> &args) {
				if(args.size() >= 1) {
					lxvar->put( from_string<T>(*args.begin()) );
					return "";
				}
				
				return to_string( lxvar->get() );
			}
			
			virtual void reset() { lxvar->put(gusDefault); }
		};
		
		return new GusVarWrapper(this, name, gusDefault);
	}
	
	T get() const { return (*getFct)(); }
	void put(T v) { (*putFct)(v); }
	
	operator T() const { return get(); }
};


template<typename T, FeatureIndex index, T (*getConvert) (T), T (*putConvert) (T)>
struct _OlxVariable_Helpers {
	static ScriptVar_t& writeVar() { return modSettings.set(index); }
	static T getFct() { return (*getConvert)( (T)gameSettings[index] ); }
	static void putFct(T v) { writeVar() = (*putConvert)(v); }
};

template<typename T, FeatureIndex index, T (*getConvert) (T), T (*putConvert) (T)>
struct _OlxVariable : __OlxVariable<T,
	&_OlxVariable_Helpers<T,index,getConvert,putConvert>::getFct,
	&_OlxVariable_Helpers<T,index,getConvert,putConvert>::putFct> {};

template<typename T, FeatureIndex index>
struct OlxVar : _OlxVariable<T, index, &Identity<T>, &Identity<T> > {};

/* those are defined in CGameObject.cpp */
float convertSpeed_LXToGus(float v);
float convertSpeed_GusToLX(float v);

template<FeatureIndex index>
struct OlxSpeedVar : _OlxVariable<float, index, &convertSpeed_LXToGus, &convertSpeed_GusToLX> {};

INLINE float convertSpeedNeg_LXToGus(float v) { return -convertSpeed_LXToGus(v); }
INLINE float convertSpeedNeg_GusToLX(float v) { return convertSpeed_GusToLX(-v); }

template<FeatureIndex index>
struct OlxNegatedSpeedVar : _OlxVariable<float, index, &convertSpeedNeg_LXToGus, &convertSpeedNeg_GusToLX> {};

/* those are defined in CGameObject.cpp */
float convertAccel_LXToGus(float v);
float convertAccel_GusToLX(float v);

template<FeatureIndex index>
struct OlxAccelVar : _OlxVariable<float, index, &convertAccel_LXToGus, &convertAccel_GusToLX> {};

INLINE float convertWormFriction_LXToGus(float v) { return 1.0f - v; }
INLINE float convertWormFriction_GusToLX(float v) { return 1.0f - v; }

template<FeatureIndex index>
struct OlxWormFrictionVar : _OlxVariable<float, index, &convertWormFriction_LXToGus, &convertWormFriction_GusToLX> {};

INLINE bool negateBool(bool v) { return !v; }

template<FeatureIndex index>
struct OlxBoolNegatedVar : _OlxVariable<bool, index, &negateBool, &negateBool> {};

INLINE float convertRopeStrength_LXToGus(float v) { return convertAccel_LXToGus(v * 100.f); }
INLINE float convertRopeStrength_GusToLX(float v) { return convertAccel_GusToLX(v) / 100.f; }

typedef _OlxVariable<float, FT_RopeStrength, &convertRopeStrength_LXToGus, &convertRopeStrength_GusToLX>
OlxRopeStrengthVar;

#endif
