#ifndef NOISE_LINE_H
#define NOISE_LINE_H

#include <vector>
#include "gusanos/allegro.h"

class NoiseLine
{
	public:
		
		NoiseLine();
		~NoiseLine();
	
		void createPath(int iterations, float variation);
		void render(ALLEGRO_BITMAP* where, int x, int y, int x1, int y1, int colour);
	
	private:
	
		std::vector<float> m_nodes;
};


#endif
