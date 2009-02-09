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
#include "ThreadPool.h"

class TaskManager;

struct Task : Action {
	Task() : manager(NULL) {}
	virtual ~Task() {}
	
	TaskManager* manager;
	std::string name;
	virtual int handle() = 0;
};

class TaskManager {
private:
	SDL_mutex* mutex;
	SDL_cond* taskFinished;
	std::set<Task*> runningTasks;
public:
	TaskManager();
	~TaskManager();
	
	void start(Task* t);
};

extern TaskManager* taskManager;

void InitTaskManager();
void UnInitTaskManager();

#endif
