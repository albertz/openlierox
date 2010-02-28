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
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "variables.h"
#include "game/Settings.h"
#include "StringUtils.h"

// TODO: move this? or is there sth like this in boost/stdlib?
template<typename T>
T identity(T v) { return v; }

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
template<typename T>
struct OlxVariable : public /* Gusanos */ Variable {
	FeatureIndex index;
	typedef boost::function<T (T)> GetConvertFct; GetConvertFct getConvert;
	typedef boost::function<T (T)> PutConvertFct; PutConvertFct putConvert;
	
	OlxVariable() : index(FeatureIndex(-1)) {} // dummy constr; don't forget to correctly assign it!
	OlxVariable(const std::string& name, FeatureIndex i, GetConvertFct gC = boost::bind(&identity), PutConvertFct pC = boost::bind(&identity))
	: Variable(name), index(i), getConvert(gC), putConvert(pC) {}
	
	// Gusanos console system callback
	virtual std::string invoke(const std::list<std::string> &args) {
		if(args.size() >= 1) {
			put( from_string<T>(*args.begin()) );
			return "";
		}
		
		return to_string(get());
	}
	
	ScriptVar_t& writeVar() { return modSettings.set(index); }
	T get() const { return getConvert( (T)gameSettings[index] ); }
	void put(T v) { writeVar() = putConvert(v); }
};

#endif
