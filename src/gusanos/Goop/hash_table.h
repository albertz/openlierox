#ifndef GUSANOS_HASH_TABLE_H
#define GUSANOS_HASH_TABLE_H

#include <utility>
#include <string>
#include <cstddef>

typedef unsigned long Hash;

template<class T>
struct hash
{
	Hash operator()(T const& key) const
	{
		return Hash(key);
	}
};

template<>
struct hash<unsigned long>
{
	Hash operator()(unsigned long key) const
	{
		return key;
	}
};

template<>
struct hash<std::string>
{
	Hash operator()(std::string const& key) const
	{
		size_t left = key.size();
		char const* d = key.data();
		Hash h = 0;
		for(; left > sizeof(Hash);
			left -= sizeof(Hash),
			d += sizeof(Hash))
		{
			h ^= *(Hash *)d;
		}
		
		// Do the rest byte by byte
		Hash rest = 0;
		for(; left > 0; --left, ++d)
		{
			rest = (rest << 8) | *d;
		}
		
		h ^= rest;
		return h;
	}
};

extern unsigned long tableSizes[18];

template<class KeyT, class ValueT, class HashFunc = hash<KeyT> >
class HashTable
{
public:
	struct Node : public std::pair<const KeyT, ValueT>
	{
		Node(KeyT const& key, ValueT const& value)
		: std::pair<const KeyT, ValueT>(key, value), next(0)
		{
		}
		
		Node(KeyT const& key)
		: std::pair<const KeyT, ValueT>(key, ValueT()), next(0)
		{
		}
		
		Node* next;
	};
	
	struct Index
	{
		Index()
		: first(0)
		{
		}
		
		Node* first;
	};
	
	friend struct iterator;
	
	// TODO: iterator where you can actually iterate
	struct iterator
	{
		friend class HashTable;
		
		std::pair<const KeyT, ValueT>* operator->() const
		{
			return slot;
		}
		
		std::pair<const KeyT, ValueT>& operator*() const
		{
			return *slot;
		}
		
		bool operator==(iterator const& b) const
		{
			return slot == b.slot;
		}
		
		bool operator!=(iterator const& b) const
		{
			return slot != b.slot;
		}
		
	private:
		iterator(Node* slot_)
		: slot(slot_)
		{
		}
		
		Node* slot;
	};
	
	HashTable(size_t initialSizeOrder = 3, HashFunc hashFunc_ = HashFunc())
	: count(0), sizeOrder(initialSizeOrder)
	, hashFunc(hashFunc_)
	{
		if(sizeOrder > 17)
			sizeOrder = 17;

		indexCount = tableSizes[sizeOrder];
		index = new Index[indexCount];
	}
	
	~HashTable()
	{
		for(size_t i = 0; i < indexCount; ++i)
		{
			for(Node* slot = index[i].first; slot;)
			{
				Node* next = slot->next;
				delete slot;
				slot = next;
			}
		}
		delete [] index;
	}
	
	iterator find(KeyT const& key)
	{
		Index& index = getIndex(key);
		if(!index.first)
			return end();

		Node* i = index.first;
		do
		{
			if(i->first == key)
				return iterator(i);
			i = i->next;
		} while(i);
		
		return end();
	}

	ValueT& operator[](KeyT const& key)
	{
		Index& index = getIndex(key);
		if(!index.first)
			return insert(key)->second;

		Node* i = index.first;
		do
		{
			if(i->first == key)
				return i->second;
			i = i->next;
		} while(i);
		
		return insert(key)->second;
	}
	
	iterator insert(KeyT const& key)
	{
		if(++count > indexCount)
			enlarge();
			
		Index& theIndex = getIndex(key);
		
		// Check if key already exists
		for(Node* i = theIndex.first; i; i = i->next)
		{
			if(i->first == key)
				return iterator(i); // Return iterator to old key
		}
		
		// Key didn't exist, add it
		Node* newNode = new Node(key);
		insert(theIndex, newNode);
		return iterator(newNode);
	}
	
	iterator end()
	{
		return iterator(0);
	}
	
private:
	void insert(Index& index, Node* node)
	{
		node->next = index.first;
		index.first = node;
	}
	
	void enlarge()
	{
		if(sizeOrder >= 17)
			return; // Max size already
			
		Index*   oldTable = index;
		Hash     oldCount = indexCount;
		
		indexCount = tableSizes[++sizeOrder];
		index = new Index[indexCount];
				
		// Move old entries to the new table
		for(size_t i = 0; i < oldCount; ++i)
		{
			for(Node* slot = oldTable[i].first; slot;)
			{
				Node* next = slot->next;
				
				insert(getIndex(slot->first), slot);
				
				slot = next;
			}
		}
		
		delete [] oldTable;
	}
	
	Index& getIndex(KeyT const& key)
	{
		return index[hashFunc(key) % indexCount];
	}
	
	Index*     index;
	size_t     count;
	Hash       indexCount;
	size_t     sizeOrder;
	HashFunc   hashFunc;
};

#endif //GUSANOS_HASH_TABLE_H
