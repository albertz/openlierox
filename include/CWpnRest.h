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


// Weapon states
enum {
    wpr_enabled = 0,
    wpr_bonus = 1,
    wpr_banned = 2
};


// Weapon Restriction structure
typedef struct wpnrest_s {

	std::string    szName;
    int     nState;

    struct  wpnrest_s   *psNext;
    struct  wpnrest_s   *psLink;        // For sorted array

} wpnrest_t;


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

    void        sendList(CBytestream *psByteS);
    void        readList(CBytestream *psByteS);

    bool        isEnabled(const std::string& szName);
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
