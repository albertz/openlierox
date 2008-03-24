#ifndef LIERO_OBJECTLIST_HPP
#define LIERO_OBJECTLIST_HPP

#include <cstddef>
#include <cassert>

struct ObjectListBase
{
	ObjectListBase* nextFree;
	
	bool used;
	/*
	ObjectListBase* prev;
	ObjectListBase* next;*/
};

template<typename T, int Limit>
struct ObjectList
{
	struct iterator
	{
		iterator(T* cur_)
		: cur(cur_)
		{
			while(!cur->used)
				++cur;
		}
		
		iterator& operator++()
		{
			do
			{
				++cur;
			}
			while(!cur->used);
			
			return *this;
		}
		
		T& operator*()
		{
			assert(cur->used);
			return *cur;
		}
		
		T* operator->()
		{
			assert(cur->used);
			return cur;
		}
		
		bool operator!=(iterator b)
		{
			return cur != b.cur;
		}
		
		T* cur;
	};
	
	ObjectList()
	{
		clear();
	}
	
	T* getFreeObject()
	{
		T* ptr = static_cast<T*>(nextFree);
		nextFree = ptr->nextFree;
		ptr->used = true;
		
		/*
		sentinel.prev->next = ptr;
		ptr->prev = sentinel.prev;
		ptr->next = &sentinel;
		sentinel.prev = ptr;
		*/
		
		++count;
		
		return ptr;
	}
	
	T* newObjectReuse()
	{
		if(!nextFree)
			return &arr[Limit - 1];
		
		return getFreeObject();
	}
	
	T* newObject()
	{
		if(!nextFree)
			return 0;
			
		return getFreeObject();
	}
	
	iterator begin()
	{
		return iterator(&arr[0]);
	}
	
	iterator end()
	{
		return iterator(&arr[Limit]);
	}
		
	void free(T* ptr)
	{
	/*
		ptr->prev->next = ptr->next;
		ptr->next->prev = ptr->prev;
		*/
		assert(ptr->used);
		
		ptr->nextFree = nextFree;
		nextFree = ptr;
		ptr->used = false;
		
		assert(count > 0);
		
		--count;
	}
	
	void free(iterator i)
	{
		free(&*i);
	}
	
	void clear()
	{
		count = 0;
		/*
		sentinel.prev = &sentinel;
		sentinel.next = &sentinel;*/
		
		arr[Limit].used = true; // Sentinel
		
		for(std::size_t i = 0; i < Limit - 1; ++i)
		{
			arr[i].nextFree = &arr[i + 1];
			arr[i].used = false;
		}
		
		arr[Limit - 1].nextFree = 0;
		arr[Limit - 1].used = false;
		nextFree = &arr[0];
	}
	
	std::size_t size() const
	{
		return count;
	}
	
	T arr[Limit + 1]; // One sentinel

	ObjectListBase* nextFree;
	//ObjectListBase sentinel;
	std::size_t count;
};

#endif // LIERO_OBJECTLIST_HPP
