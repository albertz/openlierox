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
#include "ReadWriteLock.h"

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
	quitSignal = false;
	mutex = SDL_CreateMutex();
	taskFinished = SDL_CreateCond();
	queueThreadWakeup = SDL_CreateCond();
		
	struct QueuedTaskHandler : Action {
		TaskManager* manager;
		QueuedTaskHandler(TaskManager* m) : manager(m) {}
		int handle() {
			ScopedLock lock(manager->mutex);
			while(true) {
				if(manager->queuedTasks.size() > 0) {
					Action* act = manager->queuedTasks.front();
					manager->queuedTasks.pop_front();
					SDL_mutexV(manager->mutex);
					act->handle();
					delete act;
					SDL_mutexP(manager->mutex);
					continue;
				}
				if(manager->quitSignal) {
					return 0;
				}
				SDL_CondWait(manager->queueThreadWakeup, manager->mutex);
			}
		}
	};
	queueThread = threadPool->start(new QueuedTaskHandler(this), "queued task handler");
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
	
	finishQueuedTasks();
	
	SDL_DestroyMutex(mutex);
	SDL_DestroyCond(taskFinished);
	SDL_DestroyCond(queueThreadWakeup);
}


void TaskManager::start(Task* t, bool queued) {
	SDL_mutexP(mutex);
	runningTasks.insert(t);
	SDL_mutexV(mutex);
	
	struct TaskHandler : Action {
		Task* task;
		int handle() {
			assert(task->manager != NULL);
			int ret = task->handle();
			SDL_mutexP(task->manager->mutex);
			task->manager->runningTasks.erase(task);
			SDL_CondSignal(task->manager->taskFinished);
			SDL_mutexV(task->manager->mutex);
			delete task;
			return ret;
		}
	};
	TaskHandler* handler = new TaskHandler();
	handler->task = t;
	assert(t->manager == NULL);
	t->manager = this;
	
	if(!queued)
		threadPool->start(handler, t->name + " handler", true);
	else {
		SDL_mutexP(mutex);
		if(quitSignal) {
			SDL_mutexV(mutex);
			warnings << "tried to start queued task " << t->name << " when queued task manager was already shutdown" << endl;
			return;
		}
		queuedTasks.push_back(handler);
		SDL_CondSignal(queueThreadWakeup);
		SDL_mutexV(mutex);
	}
}

void TaskManager::finishQueuedTasks() {
	SDL_mutexP(mutex);
	if(quitSignal) { // means that we have already finished
		SDL_mutexV(mutex);
		return;
	}
	quitSignal = true;
	SDL_CondSignal(queueThreadWakeup);
	SDL_mutexV(mutex);
	
	threadPool->wait(queueThread);
	queueThread = NULL;
}
