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


#include <assert.h>

#include "LieroX.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "CWpnRest.h"
#include "CBytestream.h"
#include "MathLib.h"


///////////////////
// WpnRest Constructor
CWpnRest::CWpnRest()
{
    m_psWeaponList = NULL;
    m_psSortedList = NULL;
    m_nCount = 0;
	iCycleState = wpr_banned;
}


///////////////////
// Update the weapons list from a gamescript file
void CWpnRest::updateList(CGameScript *pcGameS)
{
    assert( pcGameS );

    // Go through the weapons in the gamescript
    // If any weapon is not in our list, add it to the list
    const weapon_t *psWpn = pcGameS->GetWeapons();
    int count = pcGameS->GetNumWeapons();
    int i;


    for(i=0; i<count; i++, psWpn++) {

        wpnrest_t *w = findWeapon( psWpn->Name );
        if( !w ) {
            // No match, add it to the list
            addWeapon( psWpn->Name, wpr_enabled );
        }
    }

    // Sort the list
    sortList();
}


///////////////////
// Reset all the weapons to default (enabled)
void CWpnRest::reset(void)
{
    wpnrest_t *psWpn = m_psWeaponList;
    for(; psWpn; psWpn=psWpn->psNext) {
        psWpn->nState = wpr_enabled;
    }
}


///////////////////
// Reset all the weapons in the current game script to default (enabled)
void CWpnRest::resetVisible(CGameScript *pcGameS)
{
    assert(pcGameS);

    wpnrest_t *psWpn = m_psWeaponList;
    for(; psWpn; psWpn=psWpn->psNext) {
        if(pcGameS->weaponExists(psWpn->szName)) 
            psWpn->nState = wpr_enabled;
    }
}


///////////////////
// Randomize all the weapons in the current game script
void CWpnRest::randomizeVisible(CGameScript *pcGameS)
{
    assert(pcGameS);

    wpnrest_t *psWpn = m_psWeaponList;
    for(; psWpn; psWpn=psWpn->psNext) {
        if(pcGameS->weaponExists(psWpn->szName))
            psWpn->nState = GetRandomInt(2);
    }
}

///////////////////
// Cycles the weapon states (enabled -> bonus -> disabled)
void CWpnRest::cycleVisible(CGameScript *pcGameS)
{
    assert(pcGameS);

    wpnrest_t *psWpn = m_psWeaponList;
    for(; psWpn; psWpn=psWpn->psNext) {
        if(pcGameS->weaponExists(psWpn->szName))  {
			if (iCycleState == 3) iCycleState = 0;
            psWpn->nState = iCycleState;
		}
    }
	iCycleState++;
}


///////////////////
// Find a weapon in the list
wpnrest_t *CWpnRest::findWeapon(const std::string& szName)
{
    if(szName == "")
    	return NULL;

    static std::string tmp = "";
	tmp = szName;
	TrimSpaces(tmp);

    wpnrest_t *psWpn = m_psWeaponList;

    for(; psWpn; psWpn=psWpn->psNext) {

        // We need to be a bit lenient here in case some simple mistakes in different game scripts occur
        // Like case & leading/trailing spaces
        if( stringcasecmp(psWpn->szName, tmp) == 0 )
            return psWpn;
    }

    // No match
    return NULL;
}


///////////////////
// Add a weapon to the list
void CWpnRest::addWeapon(const std::string& szName, int nState)
{
    if(szName == "") return;

    wpnrest_t *psWpn = new wpnrest_t;

    if( !psWpn )
        return;

    psWpn->szName = szName;
    psWpn->nState = nState;
    
    // Link it in
    psWpn->psNext = m_psWeaponList;
    m_psWeaponList = psWpn;

    // Sort the list
    sortList();
}


///////////////////
// Save the weapons restrictions list
void CWpnRest::saveList(const std::string& szFilename)
{
    // Save it as plain text
    FILE *fp = OpenGameFile(szFilename, "wt");
    if( !fp )
        return;

    wpnrest_t *psWpn = m_psWeaponList;
    for(; psWpn; psWpn=psWpn->psNext) {
        fprintf(fp, "%s,%d\n", psWpn->szName.c_str(), psWpn->nState);
    }

    fclose(fp);
}


///////////////////
// Load the weapons restrictions list
void CWpnRest::loadList(const std::string& szFilename)
{
    // Shutdown the list first
    Shutdown();

    FILE *fp = OpenGameFile(szFilename, "rt");
    if( !fp )
        return;

    static std::string line;
	
    while( !feof(fp) ) {
        line = ReadUntil(fp, '\n');
		const std::vector<std::string>& exploded = explode(line,",");
		if (exploded.size() >= 2)
			addWeapon(exploded[0],from_string<int>(exploded[1]));
    }

    fclose(fp);

    // Sort the list
    sortList();
}


///////////////////
// Checks if the weapon is enabled or not
bool CWpnRest::isEnabled(const std::string& szName)
{
    wpnrest_t *psWpn = findWeapon(szName);

    // If we can't find the weapon, then assume it is banned
    if( !psWpn )
        return false;
    
    return (psWpn->nState == wpr_enabled);
}

///////////////////
// Checks if the weapon is bonus or not
bool CWpnRest::isBonus(const std::string& szName)
{
    wpnrest_t *psWpn = findWeapon(szName);

    // If we can't find the weapon, then assume it is banned
    if( !psWpn )
        return false;
    
    return (psWpn->nState == wpr_bonus);
}


///////////////////
// Finds a weapon that is enabled and returns the name
std::string CWpnRest::findEnabledWeapon(CGameScript *pcGameS) {
    assert(pcGameS);

    // Go from the start of the list looking for an enabled weapon
    // The weapon must also be in the gamescript
    wpnrest_t *psWpn = m_psWeaponList;
    for(; psWpn; psWpn=psWpn->psNext) {

        if( psWpn->nState != wpr_enabled )
            continue;

        // Is the weapon in the gamescript?
        if( !pcGameS->FindWeapon(psWpn->szName) )
            continue;

        // Must be good
        return psWpn->szName;
    }

    // No enabled weapons found
    return "";
}


///////////////////
// Get the state of a weapon
int CWpnRest::getWeaponState(const std::string& szName)
{
    wpnrest_t *psWpn = findWeapon(szName);

    // Default to disabled if we can't find the weapon
    if( !psWpn )
        return wpr_banned;

    return psWpn->nState;
}


///////////////////
// Return the sorted weapon list
wpnrest_t *CWpnRest::getList(void)
{
    return m_psSortedList;
}


///////////////////
// Return the number of weapons
int CWpnRest::getNumWeapons(void)
{
    return m_nCount;
}


///////////////////
// Create a sorted list
void CWpnRest::sortList(void)
{
    int i, j;

    // Free any previous list
    if( m_psSortedList )
        delete[] m_psSortedList;

    // Count the number of weapons
    m_nCount = 0;
    wpnrest_t *psWpn = m_psWeaponList;
    for(; psWpn; psWpn=psWpn->psNext)
        m_nCount++;

    // Allocate the sorted list
	// TODO: is the following comment still valid?
    // TODO: valgrid says, this got lost
    m_psSortedList = new wpnrest_t[m_nCount];
    if( !m_psSortedList )
        return;

    // Fill in the links
    psWpn = m_psWeaponList;
    for( i=0; i<m_nCount; i++, psWpn=psWpn->psNext)
        m_psSortedList[i].psLink = psWpn;
    
    if( m_nCount < 2 )
        return;

    // Sort the list using a simple bubble sort
    wpnrest_t temp;
   	for(i=0; i<m_nCount; i++) {
		for(j=0; j<m_nCount-1-i; j++) {

            if( m_psSortedList[j].psLink->szName.compare( m_psSortedList[j+1].psLink->szName) > 0 ) {

                // Swap the 2 items
                temp = m_psSortedList[j];
                m_psSortedList[j] = m_psSortedList[j+1];
                m_psSortedList[j+1] = temp;
            }
        }
    }
}


///////////////////
// Send the list
void CWpnRest::sendList(CBytestream *psByteS)
{
    wpnrest_t *psWpn = NULL;

    // Count the number of weapons that are _not_ enabled
    int nCount = 0;
    psWpn = m_psWeaponList;
    for(; psWpn; psWpn = psWpn->psNext ) {
        if( psWpn->nState != wpr_enabled )
            nCount++;
    }


    // Write the header
    psByteS->writeInt(nCount,2);

    // Go through the list writing out weapons that are _not_ enabled
    psWpn = m_psWeaponList;
    for(; psWpn; psWpn = psWpn->psNext ) {
        if( psWpn->nState != wpr_enabled ) {
            
            psByteS->writeString(psWpn->szName);
            psByteS->writeByte(psWpn->nState);
        }
    }
}


///////////////////
// Receive the list
void CWpnRest::readList(CBytestream *psByteS)
{
    static std::string szName;
    int nState;
    wpnrest_t *psWpn = NULL;

    // Initialize all the weapons to a default of 'enabled'
    psWpn = m_psWeaponList;
    for(; psWpn; psWpn = psWpn->psNext ) {
        psWpn->nState = wpr_enabled;
    }


    int nCount = psByteS->readInt(2);

    // Go through the list reading weapons
    for( int i=0; i<nCount; i++ ) {
        szName = psByteS->readString();
        nState = psByteS->readByte();

        // Try and find the weapon
        psWpn = findWeapon(szName);
        if( psWpn )
            psWpn->nState = nState;
        else {
            // If the weapon doesn't exist, thats ok
            // just add it to the list
            addWeapon(szName,nState);
        }
    }
}


///////////////////
// Shutdown the weapons restrictions list
void CWpnRest::Shutdown(void)
{
     wpnrest_t *psWpn = m_psWeaponList;
     wpnrest_t *psNext = NULL;

     for(; psWpn; psWpn=psNext) {
         psNext = psWpn->psNext;

         delete psWpn;
     }

     m_psWeaponList = NULL;

    // Free any sorted list
    if( m_psSortedList )
        delete[] m_psSortedList;
    m_psSortedList = NULL;
}
