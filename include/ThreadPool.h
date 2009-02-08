/*
 *  ThreadPool.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 08.02.09.
 *  code under LGPL
 *
 */

#ifndef __OLX__THREADPOOL_H__
#define __OLX__THREADPOOL_H__

#include <set>

struct SDL_mutex;
struct SDL_cond;
struct SDL_Thread;
class ThreadPool;
typedef int (*ThreadFunc) (void*);

struct ThreadPoolItem {
	ThreadPool* pool;
	SDL_Thread* thread;
	bool working;
	bool finished;
	SDL_cond* finishedSignal;
	SDL_cond* readyForNewWork;
	int ret;
};

class ThreadPool {
private:
	SDL_mutex* mutex;
	SDL_cond* awakeThread;
	SDL_cond* threadStartedWork;
	SDL_cond* threadFinishedWork;
	ThreadFunc nextFunc; void* nextParam;
	std::set<ThreadPoolItem*> availableThreads;
	std::set<ThreadPoolItem*> usedThreads;
	void prepareNewThread();
	static int threadWrapper(void* param);
public:
	ThreadPool();
	~ThreadPool();
	
	ThreadPoolItem* start(ThreadFunc fct, void* param = NULL);
	bool wait(ThreadPoolItem* thread, int* status = NULL);
};

extern ThreadPool* threadPool;

void InitThreadPool();
void UnInitThreadPool();

#endif

