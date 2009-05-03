#ifndef GUSANOS_MESSAGE_QUEUE_H
#define GUSANOS_MESSAGE_QUEUE_H

#include <list>
#include "util/macros.h"

struct MQMessage
{
	virtual int getID() { return -1; }
	virtual ~MQMessage()
	{}
};

struct MessageQueue
{
	void queue(MQMessage* msg)
	{
		messages.push_back(msg);
	}
	
	void deleteMessage(void* p)
	{
		foreach_delete(i, messages)
		{
			if(static_cast<void*>(*i) == p)
			{
				delete *i;
				messages.erase(i);
			}
		}
	}
	
	void clear()
	{
		foreach(i, messages)
			delete *i;
		messages.clear();
	}
	
	bool empty()
	{
		return messages.empty();
	}
	
	std::list<MQMessage*> messages;
};

#define mq_define_message(m_, id_, params_) struct m_##_MQMessage : public MQMessage { static int const id = id_; \
virtual int getID() { return id; } \
m_##_MQMessage params_

#define mq_end_define_message() };

#define mq_queue(q_, m_, params_...) (q_).queue(new m_##_MQMessage(params_))

#define mq_process_messages(q_) \
{ MessageQueue& q = (q_); \
foreach_delete(i, q.messages) { MQMessage* m = *i; \
switch(m->getID()) {
	
#define mq_case(m_) case m_##_MQMessage::id: { m_##_MQMessage& data = *static_cast<m_##_MQMessage*>(m); 
#define mq_end_case() } break;

#define mq_delay() goto mq_delay_message
	
#define mq_end_process_messages() } delete m; q.messages.erase(i); goto mq_delay_message; mq_delay_message: (void)0; } }

/*
mq_define_message(ChangeLevel, 0, (std::string level_))
	: level(level_)
	{
		
	}
	
	std::string level;
mq_end_define_message()

mq_process_messages(messages)
	mq_case(ChangeLevel)
		game.changeLevel(data.level);
	mq_end_case()
mq_end_process_messages()

mq_queue(messages, ChangeLevel, "foo");
*/



#endif //GUSANOS_MESSAGE_QUEUE_H
