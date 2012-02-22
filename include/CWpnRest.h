/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Weapon Restrictions class
// Created 30/3/03
// Jason Boettcher


#ifndef __CWPNREST_H__
#define __CWPNREST_H__

#include <vector>
#include <list>
#include "Iter.h"
#include "StringUtils.h" // stringcaseless

class CGameScript;
class CBytestream;


// Weapon states
enum WpnRestrictionState {
    wpr_enabled = 0,
    wpr_bonus = 1,
    wpr_banned = 2
};


// Weapon Restriction structure
class wpnrest_t { 
public:

	wpnrest_t( const std::string & _name = "", WpnRestrictionState _state = wpr_enabled ):
		szName(_name), state(_state) { }

	std::string    szName;
	union {
		int nState;
		WpnRestrictionState state;
	};

	bool operator < ( const wpnrest_t & rest ) const;

};


// Weapon Restrictions class
class CWpnRest {
private:
    // Attributes

	typedef std::map<std::string, WpnRestrictionState, stringcaseless> WeaponList;
	WeaponList m_psWeaponList;
	int			iCycleState;

public:
    // Methods

    // Constructor
    CWpnRest();

    void        loadList(const std::string& szFilename, const std::string& moddir);
    void        saveList(const std::string& szFilename);
    void        Shutdown();

    void        updateList(const std::vector<std::string> & weaponList);
	void        resetToEnabled();
    void        resetVisible(const std::vector<std::string> & weaponList);
    void        randomizeVisible(const std::vector<std::string> & weaponList);
	void		cycleVisible(const std::vector<std::string> & weaponList);

    void        sendList(CBytestream *psByteS, const std::vector<std::string> & weaponList);
    void        readList(CBytestream *psByteS);

    bool        isEnabled(const std::string& szName);
	bool        isBonus(const std::string& szName);
	std::string findEnabledWeapon(const std::vector<std::string> & weaponList);

    int         getWeaponState(const std::string& szName);

	Iterator<wpnrest_t>::Ref getList();
	WpnRestrictionState   *findWeapon(const std::string& szName);
	void		setWeaponState(const std::string& szName, WpnRestrictionState nState);
    int         getNumWeapons() const;
    static bool weaponExists(const std::string & weapon, const std::vector<std::string> & weaponList);

private:
    // Internal methods

    void        addWeapon(const std::string& szName, int nState);
};




#endif  //  __CWPNREST_H__
