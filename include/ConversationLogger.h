/*
	OpenLieroX

	game conversation logging

	code under LGPL
	created on 7/3/2009
*/

#ifndef __CONVERSATIONLOGGER_H__
#define __CONVERSATIONLOGGER_H__

#include <string>
#include <cstdio>
#include "Protocol.h" // for TXT_TYPE

class ConversationLogger  {
public:
	ConversationLogger() : 
		m_fileName("Conversations.log"), 
		m_loggingActive(false),
		m_inServer(false),
		m_file(NULL)
		{}

	~ConversationLogger() { endLogging(); }

private:
	std::string m_fileName;
	bool m_loggingActive;
	bool m_inServer;
	FILE *m_file;

private:
	void checkSizeAndRename();

public:
	void startLogging();
	void endLogging();

	void enterServer(const std::string& name);
	void leaveServer();

	void logMessage(const std::string& msg, TXT_TYPE type);
};

extern ConversationLogger *convoLogger;

#endif
