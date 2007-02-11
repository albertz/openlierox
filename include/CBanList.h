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
typedef struct banlist_s {

    char    *szNick;
    char    *szAddress;

    struct  banlist_s   *psNext;
    struct  banlist_s   *psLink;        // For sorted array

} banlist_t;


// Ban List class
class CBanList {
private:
    // Attributes

    banlist_t   *m_psBanList;
    banlist_t   *m_psSortedList;
    int         m_nCount;
	char		*m_szPath;
	bool		m_bLoading;



public:
    // Methods

    // Constructor
    CBanList();

    void        loadList(char *szFilename);
    void        saveList(char *szFilename);
    void        Shutdown(void);

    bool        isBanned(char *szAddress);

	void		addBanned(char *szAddress, char *szNick);
	void		removeBanned(char *stAddress);
    banlist_t   *findBanned(char *szAddress);

    void        sortList(void);
	void		Clear(void);

    banlist_t   *getList(void);
    int         getNumItems(void);

	char		*getPath(void);
	banlist_t	*getItemById(int ID);
	int			getIdByAddr(char *szAddress);

};




#endif  //  __CBANLIST_H__
