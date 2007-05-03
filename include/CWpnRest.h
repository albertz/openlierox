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

	UCString    szName;
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

    void        loadList(const UCString& szFilename);
    void        saveList(const UCString& szFilename);
    void        Shutdown(void);

    void        updateList(CGameScript *pcGameS);
    void        reset(void);
    void        resetVisible(CGameScript *pcGameS);
    void        randomizeVisible(CGameScript *pcGameS);
	void		cycleVisible(CGameScript *pcGameS);

    void        sendList(CBytestream *psByteS);
    void        readList(CBytestream *psByteS);

    bool        isEnabled(const UCString& szName);
	UCString findEnabledWeapon(CGameScript *pcGameS);

    int         getWeaponState(const UCString& szName);

    void        sortList(void);

    wpnrest_t   *getList(void);
    int         getNumWeapons(void);


private:
    // Internal methods

    wpnrest_t   *findWeapon(const UCString& szName);
    void        addWeapon(const UCString& szName, int nState);
};




#endif  //  __CWPNREST_H__
