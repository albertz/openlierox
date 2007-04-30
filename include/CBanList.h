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

    tString szNick;
    tString szAddress;

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
	tString	m_szPath;
	bool		m_bLoading;



public:
    // Methods

    // Constructor
    CBanList();

    void        loadList(const tString& szFilename);
    void        saveList(const tString& szFilename);
    void        Shutdown(void);

    bool        isBanned(const tString& szAddress);

	void		addBanned(const tString& szAddress, const tString& szNick);
	void		removeBanned(const tString& stAddress);
    banlist_t   *findBanned(const tString& szAddress);

    void        sortList(void);
	void		Clear(void);

    banlist_t   *getList(void);
    int         getNumItems(void);

	tString getPath(void);
	banlist_t	*getItemById(int ID);
	int			getIdByAddr(const tString& szAddress);

};




#endif  //  __CBANLIST_H__
