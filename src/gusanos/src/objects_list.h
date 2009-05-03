#ifndef OBJECTS_LIST
#define OBJECTS_LIST

//#include "base_object.h"
#include <list>
#include <vector>

static const int RENDER_LAYERS_AMMOUNT = 10;
static const int COLLISION_LAYERS_AMMOUNT = 10;

class BaseObject;

class ObjectsList
{
public:
	
#ifdef USE_OBJECT_GRID
	struct ColLayerIterator
	{
	
	private:
	};
#else
	class ColLayerIterator
	{
		friend class ObjectsList;
		public:
		
			ColLayerIterator();
			~ColLayerIterator();
	
			ColLayerIterator& operator ++ ();
			BaseObject* operator * ();
			operator bool ();
	
		protected:
		
			int m_layer;
			int m_position;
			std::list<BaseObject*>::iterator currentObject;
			std::vector< std::list<BaseObject*> >* m_list;
	};

	class RenderLayerIterator
	{
		friend class ObjectsList;
		public:
		
			RenderLayerIterator();
			~RenderLayerIterator();
	
			RenderLayerIterator& operator ++ ();
			BaseObject* operator * ();
			operator bool ();
	
		protected:
		
			int m_layer;
			int m_position;
			std::list<BaseObject*>::iterator currentObject;
			std::vector< std::list<BaseObject*> >* m_list;
	};

	class Iterator
	{
		friend class ObjectsList;
		public:
		
			Iterator();
			~Iterator();

			Iterator& operator ++ ();
			BaseObject* operator * ();
			operator bool ();
	
		protected:
		
			std::vector< std::list<BaseObject*> >::iterator currentList;
			std::list<BaseObject*>::iterator currentObject;
			std::vector< std::list<BaseObject*> >* m_list;
	};
#endif

	ObjectsList();
	~ObjectsList();
	
	void insert(int colLayer, int renderLayer, BaseObject* object);
	void erase(const Iterator& location);
	void clear();
	size_t size();
	ColLayerIterator colLayerBegin(int layer);
	RenderLayerIterator renderLayerBegin(int layer);
	Iterator begin();
	

	
	private:
		
	std::vector< std::list<BaseObject*> > m_objects;
};

#endif // OBJECTS_LIST_H
