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

#include <algorithm>
#include <assert.h>

#include "LieroX.h"
#include "CGameScript.h"
#include "FindFile.h"
#include "StringUtils.h"
#include "CWpnRest.h"
#include "CBytestream.h"
#include "MathLib.h"
#include "WeaponDesc.h"


///////////////////
// WpnRest Constructor
CWpnRest::CWpnRest()
{
	iCycleState = wpr_banned;
}

bool wpnrest_t::operator < ( const wpnrest_t & rest ) const
{
	return strcasecmp( szName.c_str(), rest.szName.c_str() ) < 0 ;
};


///////////////////
// Update the weapons list from a gamescript file
void CWpnRest::updateList(const std::vector<std::string> & weaponList)
{
    // Go through the weapons in the gamescript
    // If any weapon is not in our list, add it to the list
    int count = weaponList.size();
    int i;


    for(i=0; i<count; i++) {

        wpnrest_t *w = findWeapon( weaponList[i] );
        if( !w ) {
            // No match, add it to the list
            addWeapon( weaponList[i], wpr_enabled );
        }
    }

    // Sort the list
    sortList();
}


///////////////////
// Reset all the weapons to default (enabled)
void CWpnRest::reset()
{
	
	for( std::list<wpnrest_t> :: iterator it = m_psWeaponList.begin(); 
			it != m_psWeaponList.end(); it++ )
				it->nState = wpr_enabled;
}

class findWeaponByName
{
	std::string name;
	public:
	findWeaponByName( const std::string & _name ): name(_name) {};
	bool operator() ( const wpnrest_t & w )
	{
		return strcasecmp( name.c_str(), w.szName.c_str() ) == 0;
	}
	bool operator() ( const std::string & szName )
	{
		return strcasecmp( name.c_str(), szName.c_str() ) == 0;
	}
};

///////////////////
// Reset all the weapons in the current game script to default (enabled)
void CWpnRest::resetVisible(const std::vector<std::string> & weaponList)
{
	for( std::list<wpnrest_t> :: iterator it = m_psWeaponList.begin(); 
			it != m_psWeaponList.end(); it++ )
				if( std::find_if( weaponList.begin(), weaponList.end(), findWeaponByName(it->szName) ) != weaponList.end() )
					it->nState = wpr_enabled;
}


///////////////////
// Randomize all the weapons in the current game script
void CWpnRest::randomizeVisible(const std::vector<std::string> & weaponList)
{
	for( std::list<wpnrest_t> :: iterator it = m_psWeaponList.begin(); 
			it != m_psWeaponList.end(); it++ )
				if( std::find_if( weaponList.begin(), weaponList.end(), findWeaponByName(it->szName) ) != weaponList.end() )
					it->nState = GetRandomInt(2);
}

///////////////////
// Cycles the weapon states (enabled -> bonus -> disabled)
void CWpnRest::cycleVisible(const std::vector<std::string> & weaponList)
{
	if (iCycleState == 3) iCycleState = 0;

	for( std::list<wpnrest_t> :: iterator it = m_psWeaponList.begin(); 
			it != m_psWeaponList.end(); it++ )
				if( std::find_if( weaponList.begin(), weaponList.end(), findWeaponByName(it->szName) ) != weaponList.end() )
					it->nState = iCycleState;

	iCycleState++;
}


///////////////////
// Find a weapon in the list
wpnrest_t *CWpnRest::findWeapon(const std::string& szName)
{
	if(szName == "")
		return NULL;

	std::string tmp = szName;
	TrimSpaces(tmp);

	std::list< wpnrest_t > :: iterator it = std::find_if( m_psWeaponList.begin(), m_psWeaponList.end(), findWeaponByName(szName) );

	if( it != m_psWeaponList.end() )
		return & ( * it );

	// No match
	return NULL;
}


///////////////////
// Add a weapon to the list
void CWpnRest::addWeapon(const std::string& szName, int nState)
{
    if(szName == "") return;

    m_psWeaponList.push_back( wpnrest_t( szName, nState ) );

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

	for( std::list<wpnrest_t> :: iterator it = m_psWeaponList.begin(); 
			it != m_psWeaponList.end(); it++ )
				fprintf(fp, "%s,%d\n", it->szName.c_str(), it->nState);

    fclose(fp);
}


///////////////////
// Load the weapons restrictions list
void CWpnRest::loadList(const std::string& szFilename, const std::string& moddir)
{
    // Shutdown the list first
    Shutdown();

	std::string fn = szFilename;
	if(!strCaseStartsWith(fn, "cfg/")) {
		if(fn.size() > 4 && !stringcaseequal(fn.substr(fn.size()-4), ".wps"))
			fn += ".wps";
		if(moddir != "" && IsFileAvailable("cfg/presets/" + moddir + "/" + fn))
			fn = "cfg/presets/" + moddir + "/" + fn;
		else
			fn = "cfg/presets/" + fn;
	}
	
    FILE *fp = OpenGameFile(fn, "rt");
    if( !fp )
        return;

	std::string line;
	
    while( !feof(fp) && !ferror(fp) ) {
        line = ReadUntil(fp, '\n');
		std::vector<std::string> exploded = explode(line,",");
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
std::string CWpnRest::findEnabledWeapon(const std::vector<std::string> & weaponList) {

    // Go from the start of the list looking for an enabled weapon
    // The weapon must also be in the gamescript
	for( std::list<wpnrest_t> :: iterator it = m_psWeaponList.begin(); 
			it != m_psWeaponList.end(); it++ )
	{

        if( it->nState != wpr_enabled )
            continue;

        // Is the weapon in the gamescript?
        if( std::find_if( weaponList.begin(), weaponList.end(), findWeaponByName(it->szName) ) == weaponList.end() )
            continue;

        // Must be good
        return it->szName;
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
std::list<wpnrest_t> & CWpnRest::getList()
{
    return m_psWeaponList;
}


///////////////////
// Return the number of weapons
int CWpnRest::getNumWeapons() const
{
    return m_psWeaponList.size();
}


///////////////////
// Create a sorted list
void CWpnRest::sortList()
{
	m_psWeaponList.sort();
}


///////////////////
// Send the list
void CWpnRest::sendList(CBytestream *psByteS, const std::vector<std::string> & weaponList)
{
	std::list<wpnrest_t *> rest_to_send;

    // Add only weapons that are _not_ enabled
	for( std::list<wpnrest_t> :: iterator it = m_psWeaponList.begin(); 
			it != m_psWeaponList.end(); it++ )
	{
		if( it->nState != wpr_enabled && 
			std::find_if( weaponList.begin(), weaponList.end(), findWeaponByName(it->szName) ) != weaponList.end() )
			rest_to_send.push_back( & ( *it ) );
    }

    // Write the header
    psByteS->writeInt((int)rest_to_send.size(), 2);

    // Write the restrictions
	for (std::list<wpnrest_t *>::iterator it = rest_to_send.begin();
	it != rest_to_send.end(); it++)  {
		psByteS->writeString((*it)->szName);
		psByteS->writeByte((*it)->nState);
	}
}


///////////////////
// Receive the list
void CWpnRest::readList(CBytestream *psByteS)
{
    std::string szName;
    int nState;
    wpnrest_t *psWpn = NULL;

    // Initialize all the weapons to a default of 'enabled'
    reset();

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
void CWpnRest::Shutdown()
{
    m_psWeaponList.clear();
}

bool CWpnRest::weaponExists(const std::string & weapon, const std::vector<std::string> & weaponList)
{
	return std::find_if( weaponList.begin(), weaponList.end(), findWeaponByName(weapon) ) != weaponList.end();
};
