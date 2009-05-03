#include "objects_list.h"
#include "base_object.h"

#include <list>
#include <vector>

ObjectsList::ObjectsList()
{
	m_objects.insert( m_objects.begin(), RENDER_LAYERS_AMMOUNT*COLLISION_LAYERS_AMMOUNT, std::list<BaseObject*>() );
}

ObjectsList::~ObjectsList()
{
	
}

void ObjectsList::insert(int colLayer, int renderLayer, BaseObject* object )
{
	if( colLayer < COLLISION_LAYERS_AMMOUNT && renderLayer < RENDER_LAYERS_AMMOUNT )
	{
		m_objects[colLayer*RENDER_LAYERS_AMMOUNT+renderLayer].push_back(object);
	}
}

void ObjectsList::erase(const Iterator& location)
{
	location.currentList->erase(location.currentObject);
}

ObjectsList::ColLayerIterator ObjectsList::colLayerBegin(int layer)
{
	ColLayerIterator retIterator;
	int pos;
	for ( pos = 0; pos < RENDER_LAYERS_AMMOUNT && m_objects[layer*RENDER_LAYERS_AMMOUNT+pos].empty(); ++pos );
	retIterator.m_layer = layer;
	retIterator.m_position = pos;
	retIterator.m_list = &m_objects;
	if ( pos < RENDER_LAYERS_AMMOUNT )
		retIterator.currentObject = m_objects[layer*RENDER_LAYERS_AMMOUNT+pos].begin();
	return retIterator;
}

ObjectsList::RenderLayerIterator ObjectsList::renderLayerBegin(int layer)
{
	RenderLayerIterator retIterator;
	int pos;
	for ( pos = 0; pos < COLLISION_LAYERS_AMMOUNT && m_objects[pos*RENDER_LAYERS_AMMOUNT+layer].empty(); ++pos);
	retIterator.m_layer = layer;
	retIterator.m_position = pos;
	retIterator.m_list = &m_objects;
	if ( pos < COLLISION_LAYERS_AMMOUNT )
		retIterator.currentObject = m_objects[pos*RENDER_LAYERS_AMMOUNT+layer].begin();
	return retIterator;
}

ObjectsList::Iterator ObjectsList::begin()
{
	Iterator retIterator;
	// Find the first non empty list or the last list
	std::vector< std::list<BaseObject*> >::iterator iter = m_objects.begin();
	for ( ; iter != m_objects.end() && iter->empty(); ++iter);
	retIterator.m_list = &m_objects;
	retIterator.currentList = iter;
 	if ( iter != m_objects.end() ) retIterator.currentObject = iter->begin();
	return retIterator;
}

void ObjectsList::clear()
{
	for ( size_t i = 0; i < m_objects.size(); ++i )
	{
		m_objects[i].clear();
	}
}

size_t ObjectsList::size()
{
	size_t totalSize = 0;
	for ( size_t i = 0; i < m_objects.size(); ++i )
	{
		totalSize += m_objects[i].size();
	}
	return totalSize;
}

//******************************************************
//                    ITERATORS
//******************************************************

//****************Collision Layer Iterator********************

ObjectsList::ColLayerIterator::ColLayerIterator()
{
	m_layer = 0;
	m_position = 0;
	m_list = NULL;
}

ObjectsList::ColLayerIterator::~ColLayerIterator()
{
	
}

ObjectsList::ColLayerIterator& ObjectsList::ColLayerIterator::operator ++()
{
	if ( m_position < RENDER_LAYERS_AMMOUNT )
	{
		++currentObject;
		if ( currentObject == (*m_list)[m_layer*RENDER_LAYERS_AMMOUNT+m_position].end() )
		{
			++m_position;
			for ( ; m_position < RENDER_LAYERS_AMMOUNT && (*m_list)[m_layer*RENDER_LAYERS_AMMOUNT+m_position].empty(); ++m_position);
			
			if ( m_position < RENDER_LAYERS_AMMOUNT )
			currentObject = (*m_list)[m_layer*RENDER_LAYERS_AMMOUNT+m_position].begin();
		}
	}
	return *this;
}

BaseObject* ObjectsList::ColLayerIterator::operator *()
{
	return *currentObject;
}

ObjectsList::ColLayerIterator::operator bool()
{
	if ( m_position < RENDER_LAYERS_AMMOUNT )
	{
		return true;
	}
	return false;
}

//****************Render Layer Iterator***********************

ObjectsList::RenderLayerIterator::RenderLayerIterator()
{
	m_layer = 0;
	m_position = 0;
	m_list = NULL;
}

ObjectsList::RenderLayerIterator::~RenderLayerIterator()
{
	
}

ObjectsList::RenderLayerIterator& ObjectsList::RenderLayerIterator::operator ++()
{
	if ( m_position < COLLISION_LAYERS_AMMOUNT )
	{
		++currentObject;
		if ( currentObject == (*m_list)[m_position*RENDER_LAYERS_AMMOUNT+m_layer].end() )
		{
			++m_position;
			for ( ; m_position < COLLISION_LAYERS_AMMOUNT && (*m_list)[m_position*RENDER_LAYERS_AMMOUNT+m_layer].empty(); ++m_position);
			
			if ( m_position < COLLISION_LAYERS_AMMOUNT )
				currentObject = (*m_list)[m_position*RENDER_LAYERS_AMMOUNT+m_layer].begin();
		}
	}
	
	return *this;
}

BaseObject* ObjectsList::RenderLayerIterator::operator *()
{
	return *currentObject;
}

ObjectsList::RenderLayerIterator::operator bool()
{
	if ( m_position < COLLISION_LAYERS_AMMOUNT)
	{
		return true;
	}
	return false;
}

//****************Normal Iterator*****************************

ObjectsList::Iterator::Iterator()
{
	m_list = NULL;
}

ObjectsList::Iterator::~Iterator()
{
	
}

ObjectsList::Iterator& ObjectsList::Iterator::operator ++()
{
	if ( currentList != m_list->end() )
	{
		++currentObject;
		if ( currentObject == currentList->end() )
		{
			++currentList;
			for ( ; currentList != m_list->end() && currentList->empty(); ++currentList);
			
			if ( currentList != m_list->end() ) currentObject = currentList->begin();
		}
	}
	return *this;
}

BaseObject* ObjectsList::Iterator::operator *()
{
	return *currentObject;
}

ObjectsList::Iterator::operator bool()
{
	if ( currentList == m_list->end() )
	{
		return false;
	}
	return true;
}

