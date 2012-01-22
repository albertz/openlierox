#ifndef OBJECTS_LIST
#define OBJECTS_LIST

//#include "game/CGameObject.h"
#include <list>
#include <vector>

static const int RENDER_LAYERS_AMMOUNT = 10;
static const int COLLISION_LAYERS_AMMOUNT = 10;

class CGameObject;

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
			CGameObject* operator * ();
			operator bool ();
	
		protected:
		
			int m_layer;
			int m_position;
			std::list<CGameObject*>::iterator currentObject;
			std::vector< std::list<CGameObject*> >* m_list;
	};

	class RenderLayerIterator
	{
		friend class ObjectsList;
		public:
		
			RenderLayerIterator();
			~RenderLayerIterator();
	
			RenderLayerIterator& operator ++ ();
			CGameObject* operator * ();
			operator bool ();
	
		protected:
		
			int m_layer;
			int m_position;
			std::list<CGameObject*>::iterator currentObject;
			std::vector< std::list<CGameObject*> >* m_list;
	};

	class Iterator
	{
		friend class ObjectsList;
		public:
		
			Iterator();
			~Iterator();

			Iterator& operator ++ ();
			CGameObject* operator * ();
			operator bool ();
	
		protected:
		
			std::vector< std::list<CGameObject*> >::iterator currentList;
			std::list<CGameObject*>::iterator currentObject;
			std::vector< std::list<CGameObject*> >* m_list;
	};
#endif

	ObjectsList();
	~ObjectsList();
	
	void insert(int colLayer, int renderLayer, CGameObject* object);
	void erase(const Iterator& location);
	void clear();
	size_t size();
	ColLayerIterator colLayerBegin(int layer);
	RenderLayerIterator renderLayerBegin(int layer);
	Iterator begin();
	

	
	private:
		
	std::vector< std::list<CGameObject*> > m_objects;
};

#endif // OBJECTS_LIST_H
