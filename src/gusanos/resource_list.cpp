#include "resource_list.h"

#include <list>
#include <functional>

void reduceStack(std::function<void()> loadFunc) {
	// Use clever co-routine stack dumping to avoid too deep stacks.
	// We expect that this is always called from the same thread.
	
	static bool insideLoading = false;
	struct Stack {
		std::function<void()> loadFunc_;
		/*xxx*/ int stack_;
		Stack(std::function<void()> f = NULL) : loadFunc_(f), stack_(/*xxx*/) {}
	};
	static std::list<Stack> stacks;
	
	if(insideLoading) {
		// Dump our stack + loadFunc.
		stacks.push_back(Stack(loadFunc));
		// It will get handled from the root call context.
		// Nothing to do here anymore.
		// Switch to root call context context.
		// We must *not* return here.
		// xxx: longjmp root call context.
		return;
	}
	
	insideLoading = true;
	
	if(/* xxx: setup root call context */ false) {
		// We got longjumped here from somewhere else.
		// The top of the stacks should be that other context.
		// Handle it now.
		assert(!stacks.empty());
		Stack top;
		std::swap(top, stacks.back());
		stacks.pop_back();
		
		top.loadFunc_();
		// xxx: resume top.stack_;
		return;
	}
	else
		loadFunc();
	
	insideLoading = false;
}

