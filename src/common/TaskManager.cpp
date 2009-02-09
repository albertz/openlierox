/*
 *  TaskManager.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 09.02.09.
 *  code under LGPL
 *
 */

#include "ThreadPool.h"
#include "TaskManager.h"
#include "Debug.h"

TaskManager* taskManager = NULL;

void InitTaskManager() {
	if(!taskManager) {
		taskManager = new TaskManager();
	}
}

void UnInitTaskManager() {
	if(taskManager) {
		delete taskManager;
		taskManager = NULL;
	}
}

TaskManager::TaskManager() {
	mutex = SDL_CreateMutex();
	taskFinished = SDL_CreateCond();
}

TaskManager::~TaskManager() {
	SDL_mutexP(mutex);
	while(runningTasks.size() > 0) {
		notes << "waiting for " << runningTasks.size() << " task(s) to finish:" << endl;
		for(std::set<Task*>::iterator i = runningTasks.begin(); i != runningTasks.end(); ++i) {
			notes << "  " << (*i)->name << endl;
		}
		SDL_CondWait(taskFinished, mutex);
	}
	SDL_mutexV(mutex);
	
	SDL_DestroyMutex(mutex);
	SDL_DestroyCond(taskFinished);
}


void TaskManager::start(Task* t) {
	SDL_mutexP(mutex);
	runningTasks.insert(t);
	SDL_mutexV(mutex);
	
	struct TaskHandler : Action {
		Task* task;
		int handle() {
			int ret = task->handle();
			SDL_mutexP(task->manager->mutex);
			task->manager->runningTasks.erase(task);
			SDL_mutexV(task->manager->mutex);
			delete task;
			return ret;
		}
	};
	TaskHandler* handler = new TaskHandler();
	handler->task = t;
	
	threadPool->start(handler, t->name + " handler");
}


