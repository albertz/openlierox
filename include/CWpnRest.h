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

#include "CGameScript.h"
// TODO: remove this after we changed network
// TODO: Weapons with same names retain their state for different mods - fix that, each mod should have individual banlist
#include "CBytestream.h"

// Weapon states
enum {
    wpr_enabled = 0,
    wpr_bonus = 1,
    wpr_banned = 2
};


// Weapon Restriction structure
class wpnrest_t { public:

	std::string    szName;
    int     nState;

    wpnrest_t   *psNext;
    wpnrest_t   *psLink;        // For sorted array

};


// Weapon Restrictions class
class CWpnRest {
private:
    // Attributes

    wpnrest_t   *m_psWeaponList;
    wpnrest_t   *m_psSortedList;
    int         m_nCount;
	int			iCycleState;




public:
    // Methods

    // Constructor
    CWpnRest();

    void        loadList(const std::string& szFilename);
    void        saveList(const std::string& szFilename);
    void        Shutdown(void);

    void        updateList(CGameScript *pcGameS);
    void        reset(void);
    void        resetVisible(CGameScript *pcGameS);
    void        randomizeVisible(CGameScript *pcGameS);
	void		cycleVisible(CGameScript *pcGameS);

    void        sendList(CBytestream *psByteS, CGameScript *pcGameS);
    void        readList(CBytestream *psByteS);

    bool        isEnabled(const std::string& szName);
	bool        isBonus(const std::string& szName);
	std::string findEnabledWeapon(CGameScript *pcGameS);

    int         getWeaponState(const std::string& szName);

    void        sortList(void);

    wpnrest_t   *getList(void);
    int         getNumWeapons(void);


private:
    // Internal methods

    wpnrest_t   *findWeapon(const std::string& szName);
    void        addWeapon(const std::string& szName, int nState);
};




#endif  //  __CWPNREST_H__
