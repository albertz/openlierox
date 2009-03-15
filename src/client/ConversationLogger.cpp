/*
	OpenLieroX

	game conversation logging

	code under LGPL
	created on 15/3/2009
*/

#include "ConversationLogger.h"
#include "FindFile.h"
#include "Debug.h"
#include "Timer.h"

#define MAX_FILESIZE (1024*1024)  // 1 MB

////////////////////
// Starts the conversation logging
void ConversationLogger::startLogging()
{
	checkSizeAndRename();

	// Open the log file
	m_file = OpenGameFile(m_fileName, "a");
	if (!m_file)  {
		errors << "Could not open the " << m_fileName << " file for appending" << endl;
		return;
	}

	fprintf(m_file, "<game starttime=\"%s\">\r\n", GetDateTime().c_str());
}

////////////////////
// Ends conversation logging
void ConversationLogger::endLogging()
{
	if (!m_file)
		return;

	if (m_inServer)
		leaveServer();

	// Log the end
	fprintf(m_file, "</game>\r\n");

	// Close the file
	fclose(m_file);
	m_file = NULL;

	// Check that the file is not too big
	checkSizeAndRename();
}

///////////////////
// Log entering the server
void ConversationLogger::enterServer(const std::string &name)
{
	if (!m_file)
		return;

	// Check if not already in a server
	if (m_inServer)
		leaveServer();

	m_inServer = true;

	if (name.size() == 0)
		fprintf(m_file, "  <server hostname=\"\" jointime=\"%s\">\r\n", GetDateTime().c_str());
	else
		fprintf(m_file, "  <server hostname=\"%s\" jointime=\"%s\">\r\n", name.c_str(), GetDateTime().c_str());
}

//////////////////
// Log leaving a server
void ConversationLogger::leaveServer()
{
	if (!m_file || !m_inServer)
		return;

	fprintf(m_file, "  </server>\r\n");
	m_inServer = false;
}

//////////////////
// Log a message
void ConversationLogger::logMessage(const std::string &msg, TXT_TYPE type)
{
	if (!m_file)
		return;

	if (!m_inServer)  {
		warnings << "We're not in a server, message ignored: " << msg << endl;
		return;
	}

	std::string msg_type = "UNKNOWN";
	switch (type)  {
		// Chat
		case TXT_CHAT:		msg_type = "CHAT";	break;
		// Normal
		case TXT_NORMAL:	msg_type = "NORMAL";	break;
		// Notice
		case TXT_NOTICE:	msg_type = "NOTICE";	break;
		// Important
		case TXT_IMPORTANT:	msg_type = "IMPORTANT";	break;
		// Network
		case TXT_NETWORK:	msg_type = "NETWORK";	break;
		// Private
		case TXT_PRIVATE:	msg_type = "PRIVATE";	break;
		// Team Private Chat
		case TXT_TEAMPM:	msg_type = "TEAMPM";	break;
	}

	std::string msg_escaped;
	replace(msg, "\"", "\\\"", msg_escaped);  // Escape any quotes

	fprintf(m_file, "    <message type=\"%s\" text=\"%s\" />\r\n", msg_type.c_str(), msg_escaped.c_str());
}

////////////////////
// Checks for maximum allowed file size, renames old conversations.log file and creates a new empty one
void ConversationLogger::checkSizeAndRename()
{
	if (FileSize(m_fileName) > MAX_FILESIZE)  {
		if (m_file)  {
			hints << "Cannot rename the " << m_file << " file because it is being used by the logger" << endl;
			return;
		}

		std::string fullfn = GetFullFileName(m_fileName);
		if (!fullfn.size())  {
			errors << "Could not find the " << m_fileName << " file" << endl;
			return;
		}

		// Extract filename and extension
		std::string newname = GetBaseFilename(m_fileName);
		// TODO: move that out here
		std::string ext;
		size_t dotpos = newname.find('.');
		if (dotpos != std::string::npos)  {
			ext = newname.substr(dotpos);
			newname.erase(dotpos);
		}

		// Append the current time
		newname += "_" + GetDateTime();

		std::string renamed = ExtractDirectory(fullfn) + "/" + newname + ext;

		notes << "The conversations log file is too big, renaming to " << newname << ext << endl;
		
		// Rename
		if (rename(Utf8ToSystemNative(fullfn).c_str(), Utf8ToSystemNative(renamed).c_str()) != 0)  {
			errors << "Could not rename " << fullfn << " to " << renamed << endl;
		}

		// HINT: the current conversations log file will be regenerated automatically
	}
}

