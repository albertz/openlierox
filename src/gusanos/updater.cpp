#include "updater.h"
#include "network.h"
#include "client.h"
#include "level.h"
#include "glua.h"
#include "lua51/luaapi/context.h"
#include "util/log.h"
#include "message_queue.h"
#include "util/text.h"
#include "util/macros.h"
#include "FindFile.h"
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
Net_ClassID Updater::classID;

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
		{}

		std::list<std::pair<unsigned long, std::string> > fileQueue;

		bool sendingFile;
		Net_ConnID connID;

		void sendOne();
		void queuePath(std::string const& p);
		void queueLevel(std::string const& level, unsigned long reqID);
	};

	std::map<Net_ConnID, ConnData> connections;

	ConnData& getConnection(Net_ConnID connID)
	{
		let_(i, connections.find(connID));
		if(i != connections.end()) {
			return i->second;
		} else {
			ConnData& c = connections[connID];
			c.connID = connID;
			return c;
		}
	}

	MessageQueue msg;

	Net_Node* node = 0;
	bool isAuthority = false;
	bool ready = false;
	bool noTransfers = false;

}

Updater::Updater()
{
}

void Updater::assignNetworkRole( bool authority )
{
	assert(!node);
	node = new Net_Node;

	isAuthority = authority;
	if( authority) {
		node->setEventNotification(true, false); // Enables the eEvent_Init.
		if( !node->registerNodeUnique(classID, eNet_RoleAuthority, network.getNetControl() ) )
			ELOG("Unable to register updater authority node.");

		node->removeFromNetLevel(1);
		node->applyForNetLevel(2); // Updater operates at zoidlevel 2
	} else {
		if( !node->registerNodeUnique( classID, eNet_RoleProxy, network.getNetControl() ) )
			ELOG("Unable to register updater requested node.");
	}
}

void Updater::think()
{
	if( node ) {
		if(ready) {
			mq_process_messages(msg)
			mq_case(RequestLevel)
			/*
				if(data.sent)
					mq_delay();*/

			DLOG("Requesting level " << data.name);
			Net_BitStream* str = new Net_BitStream;
			str->addInt(MsgRequestLevel, 8);
			str->addInt(1, 32);
			str->addString( data.name.c_str() );
			node->sendEventDirect(eNet_ReliableOrdered, str, network.getServerID() );
			//data.sent = true;
			mq_end_case()
			mq_end_process_messages();
		}

		foreach(i, connections) {
			i->second.sendOne();
		}

		while ( node->checkEventWaiting() ) {
			eNet_Event    type;
			eNet_NodeRole remote_role;
			Net_ConnID    conn_id;

			Net_BitStream* data = node->getNextEvent(&type, &remote_role, &conn_id);
			switch(type) {
					case eNet_EventFile_Incoming: {

						Net_FileTransID fid = static_cast<Net_FileTransID>(data->getInt(Net_FTRANS_ID_BITS));

						if(/*!network.autoDownloads*/ false ) {
							break;
						}

					}
					break;

					case eNet_EventFile_Complete: {
						Net_FileTransID fid = static_cast<Net_FileTransID>(data->getInt(Net_FTRANS_ID_BITS));
						ILOG("Transfer of file with ID " << fid << " complete");

						ConnData& c = getConnection(conn_id);
						c.sendingFile = false;
					}
					break;

					case eNet_EventFile_Data: {
						Net_FileTransID fid = static_cast<Net_FileTransID>(data->getInt(Net_FTRANS_ID_BITS));

					}
					break;

					case eNet_EventInit: {
						if(true /*network.autoDownloads*/) {
							Net_BitStream* str = new Net_BitStream;
							str->addInt(MsgHello, 8);
							node->sendEventDirect(eNet_ReliableOrdered, str, conn_id );
						} else {
							Net_BitStream* str = new Net_BitStream;
							str->addInt(MsgSorry, 8);
							node->sendEventDirect(eNet_ReliableOrdered, str, conn_id );
						}
					}
					break;

					case eNet_EventUser: {
						Message i = static_cast<Message>(data->getInt(8));
						ILOG("Got user event: " << i);

						switch(i) {
								case MsgRequestLevel: {
									if(isAuthority /*&& network.autoDownloads*/) {
										unsigned long reqID = data->getInt(32);
										ConnData& c = getConnection(conn_id);
										c.queueLevel(data->getStringStatic(), reqID);
									}
								}
								break;

								case MsgHello: {
									if(!isAuthority) {
										DLOG("Got hello from server, connection " << conn_id);
										ready = true;
									}
								}
								break;

								case MsgSorry: {
									if(!isAuthority) {
										DLOG("Got hello from server, connection " << conn_id);
										ready = false;
										noTransfers = true;
									}
								}
								break;

								case MsgRequestDone: {
									unsigned long reqID = data->getInt(32);

									EACH_CALLBACK(i, transferFinished) {
										(lua.call(*i))();
									}

									if(reqID == 1)
										network.olxReconnect(50);
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
	delete node;
	node = 0;
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

