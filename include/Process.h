/*
 *  Process.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 10.02.09.
 *  code under LGPL
 *
 */

#ifndef __OLX__PROCESS_H__
#define __OLX__PROCESS_H__

#include <iostream>
#include <string>
#include <vector>

struct ProcessIntern;

class Process {
private:
	ProcessIntern* data;
public:
	Process(); ~Process();
	
	std::ostream& in();
	std::istream& out();
	void close();

	// this can also open script files in the UNIX-way on every system
	bool open( const std::string & cmd, std::vector< std::string > params = std::vector< std::string > (), const std::string& working_dir = "." );
};

#endif
