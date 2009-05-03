#ifndef GUSANOS_CACHE_H
#define GUSANOS_CACHE_H

#include <map>
#include <utility>

template<class KeyT, class ValueT, class ConstructorT, class DestructorT>
class Cache
{
public:
	Cache(ConstructorT const& construct_ = ConstructorT(),
	DestructorT const& destruct_ = DestructorT())
	: construct(construct_)
	, destruct(destruct_)
	{
	}
	
	struct Item
	{
		Item(ValueT const& value_, int lifetime_)
		: value(value_), lifetime(lifetime_)
		{
			
		}
		
		int lifetime;
		ValueT value;
	};
	
	typedef std::map<KeyT, Item> MapT;
	
	ValueT& operator[](KeyT const& k)
	{
		typename MapT::iterator i = m_map.find(k);
		if(i == m_map.end())
			i = m_map.insert(std::make_pair(k, Item(construct(k), 500))).first;
		else
			i->second.lifetime = 500;
		
		return i->second.value;
	}
	
	void think()
	{
		typename MapT::iterator i = m_map.begin(), next;
		for(; i != m_map.end(); i = next)
		{
			next = i;
			++next;
			
			if((i->second.lifetime -= 1) <= 0)
			{
				destruct(i->second.value);
				m_map.erase(i);
			}
		}
	}
	
private:
	MapT m_map;
	ConstructorT construct;
	DestructorT destruct;
};

#endif //GUSANOS_CACHE_H
