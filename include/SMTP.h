/*
 *  SMTP.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 19.10.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __OLX_SMTP_H__
#define __OLX_SMTP_H__

#include <string>
#include "Utils.h"

struct SmtpClient : DontCopyTag {
	struct Handler;
	Handler* intern;
	std::string host;
	std::string mailfrom;
	std::list<std::string> mailrcpts;
	std::string subject;
	std::list<std::string> headers;
	
	SmtpClient() : intern(NULL) {}
	~SmtpClient() { reset(); }
	void reset();
	
	// Note: all of them are blocking (I am too lazy right now...)
	bool connect();
	bool addText(const std::string& txt);
	bool close();
};

#endif
