#ifndef VERMES_UPDATE_H
#define VERMES_UPDATE_H

#include "netstream.h"
#include <string>

// Manages file transfers to clients
class Updater
{
public:
	static Net_ClassID classID;
	
	Updater();
	void assignNetworkRole( bool authority );
	void think();
	void removeNode();
	
	void requestLevel(std::string const& name);
	//bool requestsFulfilled();
	
};

extern Updater updater;

#endif //VERMES_UPDATE_H
