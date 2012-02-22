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
	Shutdown(); // resets everything
}

bool wpnrest_t::operator < ( const wpnrest_t & rest ) const
{
	return strcasecmp( szName.c_str(), rest.szName.c_str() ) < 0 ;
}


struct findWeaponByName
{
	std::string name;
	findWeaponByName( const std::string & _name ): name(_name) {}
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
// Update the weapons list from a gamescript file
void CWpnRest::updateList(const std::vector<std::string> & weaponList)
{
	// Go through the weapons in the gamescript
	// If any weapon is not in our list, add it to the list

	for(size_t i=0; i < weaponList.size(); i++) {

		WpnRestrictionState *w = findWeapon( weaponList[i] );
		if( !w ) {
			// No match, add it to the list
			addWeapon( weaponList[i], wpr_enabled );
		}
	}
}


///////////////////
// Reset all the weapons to enabled
void CWpnRest::resetToEnabled()
{
	foreach( it, m_psWeaponList )
		it->second = wpr_enabled;
}

///////////////////
// Reset all the weapons in the current game script to default (enabled)
void CWpnRest::resetVisible(const std::vector<std::string> & weaponList)
{
	foreach( it, m_psWeaponList )
		if( std::find_if( weaponList.begin(), weaponList.end(), findWeaponByName(it->first) ) != weaponList.end() )
			it->second = wpr_enabled;
}



///////////////////
// Randomize all the weapons in the current game script
void CWpnRest::randomizeVisible(const std::vector<std::string> & weaponList)
{
	foreach( it, m_psWeaponList )
		if( std::find_if( weaponList.begin(), weaponList.end(), findWeaponByName(it->first) ) != weaponList.end() )
			it->second = (WpnRestrictionState)GetRandomInt(2);
}

///////////////////
// Cycles the weapon states (enabled -> bonus -> disabled)
void CWpnRest::cycleVisible(const std::vector<std::string> & weaponList)
{
	if (iCycleState == 3) iCycleState = 0;

	foreach( it, m_psWeaponList )
		if( std::find_if( weaponList.begin(), weaponList.end(), findWeaponByName(it->first) ) != weaponList.end() )
			it->second = (WpnRestrictionState)iCycleState;

	iCycleState++;
}


///////////////////
// Find a weapon in the list
WpnRestrictionState *CWpnRest::findWeapon(const std::string& szName)
{
	if(szName == "")
		return NULL;

	std::string tmp = szName;
	TrimSpaces(tmp);

	WeaponList::iterator it = m_psWeaponList.find(tmp);
	if( it != m_psWeaponList.end() )
		return &it->second;

	// No match
	return NULL;
}


void CWpnRest::setWeaponState(const std::string &szName, WpnRestrictionState nState) {
	WpnRestrictionState* s = findWeapon(szName);
	if(s == NULL) {
		errors << "CWpnRest::setWeaponState: weapon " << szName << " not found" << endl;
		return;
	}
	*s = nState;
}


///////////////////
// Add a weapon to the list
void CWpnRest::addWeapon(const std::string& szName, int nState)
{
    if(szName == "") return;
	nState %= 3;

	m_psWeaponList[szName] = (WpnRestrictionState)nState;
}


///////////////////
// Save the weapons restrictions list
void CWpnRest::saveList(const std::string& szFilename)
{
    // Save it as plain text
    FILE *fp = OpenGameFile(szFilename, "wt");
	if( !fp ) {
		errors << "CWpnRest::saveList: couldn't open " << szFilename << endl;
        return;
	}

	foreach(it, m_psWeaponList)
		fprintf(fp, "%s,%d\n", it->first.c_str(), (int)it->second);

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
	if( !fp ) {
		warnings << "CWpnRest::loadList: couldn't load " << fn << " (moddir: " << moddir << ")" << endl;
        return;
	}
	
    while( !feof(fp) && !ferror(fp) ) {
		std::string line = ReadUntil(fp, '\n');
		std::vector<std::string> exploded = explode(line,",");
		if (exploded.size() >= 2)
			addWeapon(exploded[0],from_string<int>(exploded[1]));
    }

    fclose(fp);
}


///////////////////
// Checks if the weapon is enabled or not
bool CWpnRest::isEnabled(const std::string& szName)
{
	WpnRestrictionState *psWpn = findWeapon(szName);

    // If we can't find the weapon, then assume it is banned
    if( !psWpn )
        return false;

	return (*psWpn == wpr_enabled);
}

///////////////////
// Checks if the weapon is bonus or not
bool CWpnRest::isBonus(const std::string& szName)
{
	WpnRestrictionState *psWpn = findWeapon(szName);

    // If we can't find the weapon, then assume it is banned
    if( !psWpn )
        return false;

	return (*psWpn == wpr_bonus);
}


///////////////////
// Finds a weapon that is enabled and returns the name
std::string CWpnRest::findEnabledWeapon(const std::vector<std::string> & weaponList) {

    // Go from the start of the list looking for an enabled weapon
    // The weapon must also be in the gamescript
	foreach(it, m_psWeaponList)
	{
		if( it->second != wpr_enabled )
            continue;

        // Is the weapon in the gamescript?
		if( std::find_if( weaponList.begin(), weaponList.end(), findWeaponByName(it->first) ) == weaponList.end() )
            continue;

        // Must be good
		return it->first;
    }

    // No enabled weapons found
    return "";
}


///////////////////
// Get the state of a weapon
int CWpnRest::getWeaponState(const std::string& szName)
{
	WpnRestrictionState *psWpn = findWeapon(szName);

    // Default to disabled if we can't find the weapon
    if( !psWpn )
        return wpr_banned;

	return *psWpn;
}


///////////////////
// Return the sorted weapon list
Iterator<wpnrest_t>::Ref CWpnRest::getList()
{
	std::vector<wpnrest_t> wpnRests(m_psWeaponList.size());
	size_t i = 0;
	foreach(it, m_psWeaponList) {
		wpnRests[i].szName = it->first;
		wpnRests[i].state = it->second;
		++i;
	}
	return FullCopyIterator(GetIterator(wpnRests));
}


///////////////////
// Return the number of weapons
int CWpnRest::getNumWeapons() const
{
    return m_psWeaponList.size();
}



///////////////////
// Send the list
void CWpnRest::sendList(CBytestream *psByteS, const std::vector<std::string> & weaponList)
{
	std::list<WeaponList::iterator> rest_to_send;

    // Add only weapons that are _not_ enabled
	foreach(it, m_psWeaponList)
	{
		if( it->second != wpr_enabled &&
			std::find_if( weaponList.begin(), weaponList.end(), findWeaponByName(it->first) ) != weaponList.end() )
			rest_to_send.push_back( it );
    }

    // Write the header
    psByteS->writeInt((int)rest_to_send.size(), 2);

    // Write the restrictions
	foreach(it, rest_to_send)
	{
		psByteS->writeString((*it)->first);
		psByteS->writeByte((int)(*it)->second);
	}
}


///////////////////
// Receive the list
void CWpnRest::readList(CBytestream *psByteS)
{
	// Initialize all the weapons to enabled
	resetToEnabled();

    int nCount = psByteS->readInt(2);

    // Go through the list reading weapons
    for( int i=0; i<nCount; i++ ) {
		std::string szName = psByteS->readString();
		int nState = psByteS->readByte();
		nState %= 3;

        // Try and find the weapon
		WpnRestrictionState* psWpn = findWeapon(szName);
        if( psWpn )
			*psWpn = (WpnRestrictionState)nState;
        else {
            // If the weapon doesn't exist, thats ok
            // just add it to the list
			addWeapon(szName, nState);
        }
    }
}


///////////////////
// Shutdown the weapons restrictions list
void CWpnRest::Shutdown()
{
    m_psWeaponList.clear();
	iCycleState = wpr_banned;
}

bool CWpnRest::weaponExists(const std::string & weapon, const std::vector<std::string> & weaponList)
{
	return std::find_if( weaponList.begin(), weaponList.end(), findWeaponByName(weapon) ) != weaponList.end();
}
