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
#include "ThreadPool.h"
#include "Mutex.h"

class TaskManager;
struct CmdLineIntf;

struct Task : Action {
	Task() : manager(NULL), state(TS_INVALID) {}
	virtual ~Task() {}
	
	TaskManager* manager;
	std::string name;
	enum State { TS_QUEUED, TS_WAITFORIMMSTART, TS_RUNNING, TS_RUNNINGQUEUED, TS_INVALID } state;
	Mutex mutex;
	virtual int handle() = 0;
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
	
	void start(Task* t, bool queued = false);
	void finishQueuedTasks();
	void dumpState(CmdLineIntf& cli) const;	
};

extern TaskManager* taskManager;

void InitTaskManager();
void UnInitTaskManager();

#endif
