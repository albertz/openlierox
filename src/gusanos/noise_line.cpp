#include "noise_line.h"
#include "CVec.h"
#include "util/math_func.h"

#include "gusanos/allegro.h"
#include <vector>

using namespace std;

NoiseLine::NoiseLine()
{
}

NoiseLine::~NoiseLine()
{
}

void NoiseLine::createPath( int iterations, float variation )
{
	int N = ( 1 << iterations ) + 1;
	m_nodes.resize(N);
	m_nodes.front() = 0.f;
	m_nodes.back() = 0.f;
	for ( int i = iterations; i > 0; --i )
	{
		int offset = ( 1 << (i - 1) );
		int step = ( 1 << i );
		
		for ( int x = offset; x < N; x += step )
		{
			//m_nodes[x] = ( m_nodes[x-offset] + m_nodes[x+offset] )/2 + variation*(midrnd()*i)/iterations;
			m_nodes[x] = ( m_nodes[x-offset] + m_nodes[x+offset] )/2.f + variation*((float)midrnd()*i);
		}
	}
}

void NoiseLine::render(ALLEGRO_BITMAP* where, int x, int y, int x1, int y1, int colour)
{
	Vec inc = Vec( (float)(x1 - x), (float)(y1-y) ) / (float)m_nodes.size();
	Vec perpToLine = inc.perp().normal();
	Vec pos = Vec((float)x, (float)y);
	Vec oldRenderPos = pos;
	for ( size_t x = 0; x < m_nodes.size(); ++x )
	{
		pos += inc;
		Vec renderPos = pos + perpToLine*m_nodes[x];
		line( where, (int)oldRenderPos.x, (int)oldRenderPos.y, (int)renderPos.x, (int)renderPos.y, colour);
		oldRenderPos = renderPos;
	}
}
