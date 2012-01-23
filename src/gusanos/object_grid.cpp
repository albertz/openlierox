#include "object_grid.h"
#include "util/macros.h"

void Grid::resize(int x1_, int y1_, int x2_, int y2_)
{
	// save list of objects because we are clearing everything
	std::vector<std::pair<CGameObject*, Layer*> > objs;
	objs.reserve(size());
	for(iterator i = beginAll(); i; ) {
		objs.push_back(std::make_pair(&*i, &i.curLayer()));
		i.unlink();
	}
	
	// Delete old guard nodes
	foreach(l, layers)
	{
		l->clear();
		for(std::vector<Layer::Square>::iterator s = l->grid.begin(); s != l->grid.end(); ++s)
		{
			if(s->guardNode)
				delete s->guardNode;
			s->guardNode = NULL;
		}
	}
	
	width = x2_ - x1_;
	height = y2_ - y1_;
	offsetX = -x1_;
	offsetY = -y1_;
	x1 = x1_;
	y1 = y1_;
	x2 = x2_;
	y2 = y2_;
	squaresH = (width + squareSide - 1) / squareSide;
	squaresV = (height + squareSide - 1) / squareSide;
	layers.clear();
	layers.resize(layerCount, Layer(squaresH * squaresV));
	
	foreach(l, layers)
	{
		// insertD inserts at the front, so we have to go through the squares in
		// reverse order.
		assert(!l->list.beginS() && !l->list.beginD());
		
		for(std::vector<Layer::Square>::reverse_iterator s = l->grid.rbegin(); s != l->grid.rend(); ++s)
		{
			s->guardNode = new CGameObject;
			l->list.insertD(s->guardNode);
		}
		
		ObjectList::light_iterator i = l->list.beginD();
		
		size_t c = 0;
		for(; i; ++i)
		{
			++c;
		}
		
		assert(c == l->grid.size());
		//assert(l->grid.size() == squaresH * squaresV + 1);
	}
	
	// put objects back in
	foreach(o, objs)
		insertImmediately(o->first, *o->second);
}
