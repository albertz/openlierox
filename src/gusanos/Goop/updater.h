#ifndef GUSANOS_UPDATE_H
#define GUSANOS_UPDATE_H

#include <zoidcom.h>
#include <string>

// Manages file transfers to clients
class Updater
{
public:
	static ZCom_ClassID classID;
	
	Updater();
	void assignNetworkRole( bool authority );
	void think();
	void removeNode();
	
	void requestLevel(std::string const& name);
	//bool requestsFulfilled();
	
};

extern Updater updater;

#endif //GUSANOS_UPDATE_H
