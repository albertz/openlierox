/*
 *  TaskManager.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 09.02.09.
 *  code under LGPL
 *
 */

#ifndef __TASKMANAGER_H__
#define __TASKMANAGER_H__

#include <SDL_thread.h>
#include <string>
#include <set>
#include <list>
#include <typeinfo>
#include <boost/shared_ptr.hpp>
#include "ThreadPool.h"
#include "Mutex.h"
#include "RefCounter.h"

class TaskManager;
struct CmdLineIntf;

struct Task : Action {
	Task() : manager(NULL), state(TS_INVALID), mutex(new Mutex()) {}
	virtual ~Task() {}
	
	TaskManager* manager;
	std::string name;
	enum State { TS_QUEUED, TS_WAITFORIMMSTART, TS_RUNNING, TS_RUNNINGQUEUED, TS_INVALID } state;
	boost::shared_ptr<Mutex> mutex;
	virtual int handle() = 0;
};

// small Task holder helper - task mutex is locked while this exists
struct ScopedTask : RefCounter {
	Task* task;
	ScopedTask() : task(NULL) {}
	ScopedTask(Task* t, bool doLock) : task(t) { if(task && doLock) task->mutex->lock(); }
	ScopedTask(const ScopedTask& t) : RefCounter(t), task(t.task) {}
	void onLastRefRemoved() { if(task) task->mutex->unlock(); }
};

class TaskManager {
private:
	SDL_mutex* mutex;
	SDL_cond* taskFinished;
	SDL_cond* queueThreadWakeup;
	bool quitSignal;
	std::set<Task*> runningTasks;
	std::list<Action*> queuedTasks;
	ThreadPoolItem* queueThread;
public:
	TaskManager();
	~TaskManager();
	
	enum QueueType { QT_NoQueue, QT_GlobalQueue, QT_QueueToSameType };
	void start(Task* t, QueueType queue = QT_NoQueue);
	ScopedTask haveTaskOfType(const std::type_info& taskType); // call with typeid(TaskClass)
	void finishQueuedTasks();
	void dumpState(CmdLineIntf& cli) const;	
};

extern TaskManager* taskManager;

void InitTaskManager();
void UnInitTaskManager();

#endif
