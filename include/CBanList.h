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

    std::string szNick;
    std::string szAddress;

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
	std::string	m_szPath;
	bool		m_bLoading;



public:
    // Methods

    // Constructor
    CBanList();

    void        loadList(const std::string& szFilename);
    void        saveList(const std::string& szFilename);
    void        Shutdown(void);

    bool        isBanned(const std::string& szAddress);

	void		addBanned(const std::string& szAddress, const std::string& szNick);
	void		removeBanned(const std::string& stAddress);
    banlist_t   *findBanned(const std::string& szAddress);

    void        sortList(void);
	void		Clear(void);

    banlist_t   *getList(void);
    int         getNumItems(void);

	std::string getPath(void);
	banlist_t	*getItemById(int ID);
	int			getIdByAddr(const std::string& szAddress);

};




#endif  //  __CBANLIST_H__
