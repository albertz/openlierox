/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Ban List class
// Created 9/8/06
// Dark Charlie


#ifndef __CBANLIST_H__
#define __CBANLIST_H__


// Ban List structure
class banlist_t { public:

    UCString szNick;
    UCString szAddress;

    banlist_t   *psNext;
    banlist_t   *psLink;        // For sorted array

};


// Ban List class
class CBanList {
private:
    // Attributes

    banlist_t   *m_psBanList;
    banlist_t   *m_psSortedList;
    int         m_nCount;
	UCString	m_szPath;
	bool		m_bLoading;



public:
    // Methods

    // Constructor
    CBanList();

    void        loadList(const UCString& szFilename);
    void        saveList(const UCString& szFilename);
    void        Shutdown(void);

    bool        isBanned(const UCString& szAddress);

	void		addBanned(const UCString& szAddress, const UCString& szNick);
	void		removeBanned(const UCString& stAddress);
    banlist_t   *findBanned(const UCString& szAddress);

    void        sortList(void);
	void		Clear(void);

    banlist_t   *getList(void);
    int         getNumItems(void);

	UCString getPath(void);
	banlist_t	*getItemById(int ID);
	int			getIdByAddr(const UCString& szAddress);

};




#endif  //  __CBANLIST_H__
