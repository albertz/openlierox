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
#include "CBytestream.h"

// Weapon states
enum {
    wpr_enabled = 0,
    wpr_bonus = 1,
    wpr_banned = 2
};


// Weapon Restriction structure
class wpnrest_t { public:

	tString    szName;
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

    void        loadList(const tString& szFilename);
    void        saveList(const tString& szFilename);
    void        Shutdown(void);

    void        updateList(CGameScript *pcGameS);
    void        reset(void);
    void        resetVisible(CGameScript *pcGameS);
    void        randomizeVisible(CGameScript *pcGameS);
	void		cycleVisible(CGameScript *pcGameS);

    void        sendList(CBytestream *psByteS);
    void        readList(CBytestream *psByteS);

    bool        isEnabled(const tString& szName);
	tString findEnabledWeapon(CGameScript *pcGameS);

    int         getWeaponState(const tString& szName);

    void        sortList(void);

    wpnrest_t   *getList(void);
    int         getNumWeapons(void);


private:
    // Internal methods

    wpnrest_t   *findWeapon(const tString& szName);
    void        addWeapon(const tString& szName, int nState);
};




#endif  //  __CWPNREST_H__
