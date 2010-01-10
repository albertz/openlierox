

#ifndef _LLIST_H_
#define _LLIST_H_

#include <utility>
#include <iostream>
using std::cerr;
using std::endl;

template<class T>
class LList;


template<class T>
class LItem
{
private:
	T			 m_Data;
	LItem<T>	*m_Prev;
	LItem<T>	*m_Next;

public:
	LItem<T>(T const& a_Init)
	{
		m_Data = a_Init;
	}

	LItem<T>* getNext(void)
	{
		return m_Next;
	}

	LItem<T>* getPrev(void)
	{
		return m_Prev;
	}

	void setNext(LItem<T>* a_Src)
	{
		m_Next = a_Src;
	}

	void setPrev(LItem<T>* a_Src)
	{
		m_Prev = a_Src;
	}

	operator T()
	{
		return m_Data;
	}

	T* operator ->()
	{
		return &m_Data;
	}

	LItem()
	{
	}
};

//Example: class Unit : LNodeImp<Unit>
template<class T>
class LNodeImp
{
public:
	T* getNext(void)
	{
		return m_Next_;
	}

	T* getPrev(void)
	{
		return m_Prev_;
	}

	void setNext(T* a_Src)
	{
		m_Next_ = a_Src;
	}

	void setPrev(T* a_Src)
	{
		m_Prev_ = a_Src;
	}
private:
	T	*m_Prev_;
	T	*m_Next_;
};

template<class T>
class LList
{
private:
	T *m_First;
	T *m_Last;
	long m_Count;
	
	template<class Op>
	static std::pair<T*, T*> sort_private(T* section, Op& op);

public:
	struct iterator
	{
		iterator()
		: ptr(0)
		{
		}
		
		iterator(T* ptr_)
		: ptr(ptr_)
		{

		}
		
		iterator& operator++()
		{
			ptr = static_cast<T *>(ptr->getNext());
			return *this;
		}
		
		iterator& operator--()
		{
			ptr = static_cast<T *>(ptr->getPrev());
			return *this;
		}
		
		bool operator==(iterator const& rhs) const
		{
			return ptr == rhs.ptr;
		}
		
		bool operator!=(iterator const& rhs) const
		{
			return ptr != rhs.ptr;
		}
		
		T* operator->() const
		{
			return ptr;
		}
		
		T& operator * () const
		{
			return *ptr;
		}
		
		operator T*()
		{
			return ptr;
		}
		
		T * ptr;
	};
	
	typedef T& reference;
	
	//typedef iterator reverse_iterator;
	
	inline T*		insertSorted(T* a_Item);
	inline T*		insert(T* a_Item, T* a_insertAfter);
	inline T*		insert(T* a_Item);			//inserts existing item in list
	inline T*		insert(T const& a_Src);			//inserts copy of existing item in list
	inline T*		insert(void);				//inserts an empty item in the list
	inline void		unlink(T* a_Item);			//Removes the item from the list but doesn't free it's memory
	inline void		erase(T* a_Item);			//Removes the item from the list and frees it's memory
	inline void		clear(void);				//clear the whole lists (frees the memory)
	inline void		unlinkAll(void);			//unlinks all members from the list (this practically sets the first and last pointers to NULL)

	template<class Op>
	inline void		sort(Op&);

	iterator begin()
	{
		return iterator(getFirst());
	}
	
	iterator end()
	{
		return iterator(0);
	}
	
	iterator last()
	{
		return iterator(m_Last);
	}
	
/*
	reverse_iterator rbegin()
	{
		return iterator(getLast());
	}
	
	reverse_iterator rend()
	{
		return iterator(0);
	}*/

	inline T*		getFirst(void);
	inline T*		getLast(void);
	inline long		getCount(void);
	
	inline static bool	isEnd(T* a_Item);
	inline static T*	getEnd(void);

	inline LList();
	inline ~LList();

};

template<class T>
inline T* LList<T>::insertSorted(T* a_Item)
{
	if(m_First == NULL)
	{
		insert(a_Item);
	}
	else
	{
		T* insertAfter = NULL;
		
		if(T::compare(m_First, a_Item) < 0)
		{
			insertAfter = m_First;
			
			while(insertAfter->getNext() != NULL && T::compare(insertAfter->getNext(), a_Item) < 0)
			{
				insertAfter = insertAfter->getNext();
			}
		}

		insert(a_Item, insertAfter);
	}
	
	return a_Item;
}

template<class T>
inline T* LList<T>::insert(T* a_Item, T* a_InsertAfter)
{
	if(!m_First)
	{
		m_First = m_Last = a_Item;
		a_Item->setNext(0);
		a_Item->setPrev(0);
	}
	else
	{
		if(!a_InsertAfter)
		{
			a_Item->setNext(m_First);
			m_First = a_Item;
		}
		else
		{
			a_Item->setNext(a_InsertAfter->getNext());
			
			if(a_InsertAfter == m_Last)
			{
				m_Last = a_Item;
			}
			
			a_InsertAfter->setNext(a_Item);
		}
		
		a_Item->setPrev(a_InsertAfter);
		
		if(a_Item->getNext())
		{
			a_Item->getNext()->setPrev(a_Item);
		}
	}

	m_Count++;

	return a_Item;
}

template<class T>
inline T* LList<T>::insert(T* a_Item)
{
	if(!m_First)
	{
		m_First = m_Last = a_Item;
		a_Item->setNext(0);
		a_Item->setPrev(0);
	}
	else
	{
		a_Item->setNext(0);
		a_Item->setPrev(m_Last);

		m_Last->setNext(a_Item);
		m_Last = a_Item;
	}
	
	++m_Count;

	return a_Item;
}

template<class T>
inline T* LList<T>::insert(void)
{
	T *n = new T;
	return insert(n);
}


template<class T>
inline T* LList<T>::insert(T const& a_Src)
{
	T *item = insert();
	
	T* prev = static_cast<T *>(item->getPrev());
	T* next = static_cast<T *>(item->getNext());

	*item = a_Src;
	
	item->setPrev(prev);
	item->setNext(next);

	return item;
}

template<class T>
inline void LList<T>::unlink(T* a_Item)
{
	if(a_Item->getPrev())
	{
		a_Item->getPrev()->setNext(a_Item->getNext());
	}
	if(a_Item->getNext())
	{
		a_Item->getNext()->setPrev(a_Item->getPrev());
	}

	if(a_Item == m_Last)
		m_Last = a_Item->getPrev();

	if(a_Item == m_First)
		m_First = a_Item->getNext();
		
	m_Count--;
}

template<class T>
inline void	LList<T>::erase(T* a_Item)
{
	unlink(a_Item);
	delete a_Item;
}

template<class T>
inline void LList<T>::clear(void)
{
	T* pIter = getFirst();

	while(pIter)
	{
		T* pNext = (T *)pIter->getNext();
		delete pIter;
		pIter = pNext;
	}

	unlinkAll();
}

template<class T>
inline void LList<T>::unlinkAll(void)
{
	m_First = m_Last = 0;
	m_Count = 0;
}


template<class T>
template<class Op>
inline void LList<T>::sort(Op& op)
{
	if(!m_First)
		return;
		
	std::pair<T*, T*> p = sort_private(m_First, op);
	m_First = p.first;
	m_Last = p.second;
	m_Last->setNext(0);
	
	// Correct the prev pointers
	T* i = m_First;
	T* last = 0;
	
	for(;i ; i = i->getNext())
	{
		i->setPrev(last);
		last = i;
	}
}

template<class T>
template<class Op>
std::pair<T*, T*> LList<T>::sort_private(T* section, Op& op)
{
	T* pivot = section;
	
	if(pivot->getNext() == 0)
		return std::make_pair(pivot, pivot); // Section is sorted (only one element)
		
	T* lo = 0;
	T* hi = 0;
		
	T* i = pivot->getNext();
	while(i)
	{
		T* next = i->getNext();
		
		if(op(i, pivot))
		{
			i->setNext(lo);
			lo = i;
		}
		else
		{
			i->setNext(hi);
			hi = i;
		}
		
		i = next;
	}
	
	std::pair<T*, T*> ret;
	
	if(lo)
	{
		std::pair<T*, T*> p = sort_private(lo, op);
		p.second->setNext(pivot);
		ret.first = p.first;
	}
	else
		ret.first = pivot;
	
	if(hi)
	{
		std::pair<T*, T*> p = sort_private(hi, op);
		pivot->setNext(p.first);
		ret.second = p.second;
	}	
	else
		ret.second = pivot;
				
	return ret;
}

template<class T>
inline T* LList<T>::getFirst(void)
{
	return m_First;
}

template<class T>
inline T* LList<T>::getLast(void)
{
	return m_Last;
}

template<class T>
inline long LList<T>::getCount(void)
{
	return m_Count;
}

template<class T>
inline /*static*/ bool LList<T>::isEnd(T* a_Item)
{
	return !a_Item;
}

template<class T>
inline /*static*/ T* LList<T>::getEnd(void)
{
	return 0;
}

template<class T>
inline LList<T>::LList()
{
	m_First = m_Last = 0;
	m_Count = 0;
}

template<class T>
inline LList<T>::~LList()
{
	clear();
}

#endif //_LLIST_H_
