#include "updater.h"
#include "network.h"
#include "client.h"
#include "level.h"
#include "glua.h"
#include "luaapi/context.h"
#include "util/log.h"
#include "message_queue.h"
#include "util/text.h"
#include "util/macros.h"
#include <list>
#include <string>
#include <map>
#include <utility>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
namespace fs = boost::filesystem;

Updater updater;
ZCom_ClassID Updater::classID;

namespace
{
	enum Message
	{
		MsgRequestLevel = 0,
		MsgHello,
		MsgRequestDone,
		MsgSorry,
	};
	
	mq_define_message(RequestLevel, 0, (std::string name))
		: name(name), sent(false)
		{
			
		}
		
		std::string name;
		bool sent;
	mq_end_define_message()
	
	struct ConnData
	{
		ConnData()
		: sendingFile(false)
		{
		}
		
		std::list<std::pair<unsigned long, std::string> > fileQueue;
		
		bool sendingFile;
		ZCom_ConnID connID;
		
		void sendOne();
		void queuePath(fs::path const& p);
		void queueLevel(std::string const& level, unsigned long reqID);
	};
	
	std::map<ZCom_ConnID, ConnData> connections;
	
	ConnData& getConnection(ZCom_ConnID connID)
	{
		let_(i, connections.find(connID));
		if(i != connections.end())
		{
			return i->second;
		}
		else
		{
			ConnData& c = connections[connID];
			c.connID = connID;
			return c;
		}
	}
	
	MessageQueue msg;
	
	ZCom_Node* node = 0;
	bool isAuthority = false;
	bool ready = false;
	bool noTransfers = false;
	
	void ConnData::sendOne()
	{
		if(!fileQueue.empty())
		{
			std::pair<unsigned long, std::string>& t = fileQueue.front();
			if(!sendingFile)
			{
				if(t.first == 0)
				{
					std::string const& file = t.second;
					ZCom_FileTransID fid = node->sendFile(file.c_str(), 0, connID, 0, 1.0f);
					ILOG("Sending file with ID " << fid);
					sendingFile = true;
					fileQueue.pop_front();
				}
				else
				{
					ZCom_BitStream* str = new ZCom_BitStream;
					str->addInt(MsgRequestDone, 8);
					str->addInt(t.first, 32);
					node->sendEventDirect(eZCom_ReliableOrdered, str, connID );
					fileQueue.pop_front();
				}
			}
		}
	}
	
	void ConnData::queuePath(fs::path const& p)
	{
		if(fs::is_directory(p))
		{
			fs::directory_iterator i(p), e;
			
			for(; i != e; ++i)
			{
				queuePath(*i);
			}
		}
		else
		{
			DLOG("Queing " << p.string());
			fileQueue.push_back(std::make_pair(0, p.native_file_string()));
		}
	}
	
	void ConnData::queueLevel(std::string const& level, unsigned long reqID) 
	{
		// TODO: WARNING: Prevent exception throwing if level doesn't exist
		fs::path const& p = levelLocator.getPathOf(level);
		
		queuePath(p);
		fileQueue.push_back(std::make_pair(reqID, level));
	}
}

Updater::Updater()
{
	
}

void Updater::assignNetworkRole( bool authority )
{
	assert(!node);
	node = new ZCom_Node;

	isAuthority = authority;
	if( authority)
	{
		node->setEventNotification(true, false); // Enables the eEvent_Init.
		if( !node->registerNodeUnique(classID, eZCom_RoleAuthority, network.getZControl() ) )
			ELOG("Unable to register updater authority node.");
		
		node->removeFromZoidLevel(1);
		node->applyForZoidLevel(2); // Updater operates at zoidlevel 2
	}
	else
	{
		if( !node->registerNodeUnique( classID, eZCom_RoleProxy, network.getZControl() ) )
			ELOG("Unable to register updater requested node.");
	}
}

void Updater::think()
{
	if( node )
	{
		if(ready)
		{
			mq_process_messages(msg)
				mq_case(RequestLevel)
				/*
					if(data.sent)
						mq_delay();*/
					
					DLOG("Requesting level " << data.name);
					ZCom_BitStream* str = new ZCom_BitStream;
					str->addInt(MsgRequestLevel, 8);
					str->addInt(1, 32);
					str->addString( data.name.c_str() );
					node->sendEventDirect(eZCom_ReliableOrdered, str, network.getServerID() );
					//data.sent = true;
				mq_end_case()
			mq_end_process_messages();
		}
		
		foreach(i, connections)
		{
			i->second.sendOne();
		}
	
		while ( node->checkEventWaiting() )
		{
			eZCom_Event    type;
			eZCom_NodeRole remote_role;
			ZCom_ConnID    conn_id;
			
			ZCom_BitStream* data = node->getNextEvent(&type, &remote_role, &conn_id);
			switch(type)
			{
				case eZCom_EventFile_Incoming:
				{
					
					ZCom_FileTransID fid = static_cast<ZCom_FileTransID>(data->getInt(ZCOM_FTRANS_ID_BITS));
					
					if(!network.autoDownloads)
					{
						node->acceptFile(conn_id, fid, 0, false);
						break;
					}
					
					ZCom_FileTransInfo const& info = node->getFileInfo(conn_id, fid);
					
					bool accept = true;
					
					try
					{
						fs::path p(info.path);
						
						foreach(i, p)
						{
							if(*i == "..")
								accept = false;
						}
						
						if(accept)
						{
							fs::create_directories(p.branch_path());
							
							ILOG("Accepting incoming file with ID " << fid);
						}
					}
					catch(fs::filesystem_error& e)
					{
						ELOG("Filesystem error: " << e.what());
						accept = false;
					}

					node->acceptFile(conn_id, fid, 0, accept);
				}
				break;
				
				case eZCom_EventFile_Complete:
				{
					ZCom_FileTransID fid = static_cast<ZCom_FileTransID>(data->getInt(ZCOM_FTRANS_ID_BITS));
					ILOG("Transfer of file with ID " << fid << " complete");
					
					ConnData& c = getConnection(conn_id);
					c.sendingFile = false;
				}
				break;
				
				case eZCom_EventFile_Data:
				{
					ZCom_FileTransID fid = static_cast<ZCom_FileTransID>(data->getInt(ZCOM_FTRANS_ID_BITS));
					ZCom_FileTransInfo const& info = node->getFileInfo(conn_id, fid);
					
					//DLOG("Transfer: " << double(info.bps) / 1000.0 << " kB/s, " << ((100 * info.transferred) / info.size) << "% done.");
					EACH_CALLBACK(i, transferUpdate)
					{
						(lua.call(*i), info.path, info.bps, info.transferred, info.size)();
					}
					
					//ZCom_ConnStats const& state = network.getZControl()->ZCom_getConnectionStats(conn_id);
					
				}
				break;
				
				case eZCom_EventInit:
				{
					if(network.autoDownloads)
					{
						ZCom_BitStream* str = new ZCom_BitStream;
						str->addInt(MsgHello, 8);
						node->sendEventDirect(eZCom_ReliableOrdered, str, conn_id );
					}
					else
					{
						ZCom_BitStream* str = new ZCom_BitStream;
						str->addInt(MsgSorry, 8);
						node->sendEventDirect(eZCom_ReliableOrdered, str, conn_id );
					}
				}
				break;
				
				case eZCom_EventUser:
				{
					Message i = static_cast<Message>(data->getInt(8));
					ILOG("Got user event: " << i);
					
					switch(i)
					{
						case MsgRequestLevel:
						{
							if(isAuthority && network.autoDownloads)
							{
								unsigned long reqID = data->getInt(32);
								ConnData& c = getConnection(conn_id);
								c.queueLevel(data->getStringStatic(), reqID);
							}
						}
						break;
						
						case MsgHello:
						{
							if(!isAuthority)
							{
								DLOG("Got hello from server, connection " << conn_id);
								ready = true;
							}
						}
						break;
						
						case MsgSorry:
						{
							if(!isAuthority)
							{
								DLOG("Got hello from server, connection " << conn_id);
								ready = false;
								noTransfers = true;
							}
						}
						break;
						
						case MsgRequestDone:
						{
							unsigned long reqID = data->getInt(32);
														
							EACH_CALLBACK(i, transferFinished)
							{
								(lua.call(*i))();
							}
							
							if(reqID == 1)
								network.reconnect(50);
						}
						break;
					}
				}
				break;
				
				default:
					ILOG("Got some other event");
				break;
			}
		}
	}
}

void Updater::removeNode()
{
	delete node; node = 0;
	ready = false;
	noTransfers = false;
}

void Updater::requestLevel(std::string const& name)
{
	mq_queue(msg, RequestLevel, name);
}

/*
bool Updater::requestsFulfilled()
{
	return msg.empty();
}*/

